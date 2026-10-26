#!/usr/bin/env python3
"""Isolate the original single-instance C DSP using Clang's resolved declarations.
No textual name substitution: only DeclRefExprs bound to mutable static storage
are rewritten. Instance buffers are allocated before audio. No process mutex,
shared oscillator state, per-block state copies or dynamic loader tricks.
"""
import ctypes as C,json,pathlib,subprocess,sys,os
os.environ.setdefault('DEVELOPER_DIR','/Library/Developer/CommandLineTools')
root=pathlib.Path(__file__).resolve().parents[2];out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
clang=os.environ.get('CC','/Library/Developer/CommandLineTools/usr/bin/clang');sdk=subprocess.check_output(['xcrun','--sdk','macosx','--show-sdk-path'],text=True).strip()
args=['-isysroot',sdk,'-std=gnu11','-Wno-everything','-DSYNTHMAVEN_SYNTH=1','-DSM_BLE=0','-DSM_ENTER(x)=sm_current=(x)', '-I'+str(root/'build/gen'),'-I'+str(root/'firmware/src'),'-I'+str(root/'firmware/hal')]
pre=subprocess.check_output([clang,*args,'-E','-P',str(root/'desktop/Source/DSPUnity.c')]).decode('latin1')
pre='static _Thread_local struct sm_context *sm_current;\n'+pre
pp=out/'DSP.pre.c';pp.write_bytes(pre.encode('latin1'))
with (out/'DSP.ast.json').open('w') as f:subprocess.run([clang,'-isysroot',sdk,'-std=gnu11','-Wno-everything','-Xclang','-ast-dump=json','-fsyntax-only',str(pp)],stdout=f,check=True)
ast=json.loads((out/'DSP.ast.json').read_text())
# libclang supplies ABI sizes/alignment and top-level const qualification.
lib=C.CDLL('/Library/Developer/CommandLineTools/usr/lib/libclang.dylib')
class Cursor(C.Structure):_fields_=[('kind',C.c_int),('xdata',C.c_int),('data',C.c_void_p*3)]
class Type(C.Structure):_fields_=[('kind',C.c_int),('data',C.c_void_p*2)]
class Loc(C.Structure):_fields_=[('ptr',C.c_void_p*2),('data',C.c_uint)]
def fn(n,rest,*args):f=getattr(lib,n);f.restype=rest;f.argtypes=list(args);return f
index=fn('clang_createIndex',C.c_void_p,C.c_int,C.c_int)(0,0)
parse=fn('clang_parseTranslationUnit',C.c_void_p,C.c_void_p,C.c_char_p,C.POINTER(C.c_char_p),C.c_int,C.c_void_p,C.c_uint,C.c_uint)
ca=(C.c_char_p*4)(b'-isysroot',sdk.encode(),b'-std=gnu11',b'-Wno-everything');tu=parse(index,str(pp).encode(),ca,4,None,0,0);assert tu
gettype=fn('clang_getCursorType',Type,Cursor);size=fn('clang_Type_getSizeOf',C.c_longlong,Type);align=fn('clang_Type_getAlignOf',C.c_longlong,Type)
qual=fn('clang_isConstQualifiedType',C.c_uint,Type);elem=fn('clang_getArrayElementType',Type,Type);canon=fn('clang_getCanonicalType',Type,Type)
getloc=fn('clang_getCursorLocation',Loc,Cursor);getoff=fn('clang_getExpansionLocation',None,Loc,C.c_void_p,C.c_void_p,C.c_void_p,C.POINTER(C.c_uint))
sizes={}
CB=C.CFUNCTYPE(C.c_uint,Cursor,Cursor,C.c_void_p)
def visit(c,p,d):
 if c.kind==9:
  off=C.c_uint();getoff(getloc(c),None,None,None,C.byref(off));t=canon(gettype(c));ct=t
  while ct.kind in (112,114,115,116):ct=canon(elem(ct))
  sizes[off.value]=(size(t),align(t),bool(qual(ct)))
 return 2
visitor=CB(visit);fn('clang_visitChildren',C.c_uint,Cursor,CB,C.c_void_p)(fn('clang_getTranslationUnitCursor',Cursor,C.c_void_p)(tu),visitor,None)
vars={};refs=[]
def walk(n,infn=False):
 if n.get('kind')=='FunctionDecl':infn=True
 if n.get('kind')=='VarDecl' and (not infn or n.get('storageClass')=='static') and n.get('storageClass')!='extern' and n.get('name')!='sm_current':
  loc=n.get('loc',{}).get('offset');abi=sizes.get(loc)
  if abi and abi[0]>0:
   qt=n.get('type',{}).get('qualType','').split('[')[0].strip()
   immutable=qt.endswith('const') if '*' in qt else qt.startswith('const ')
   vars[n['id']]={'node':n,'abi':(abi[0],abi[1],immutable),'local':infn}
 if n.get('kind')=='DeclRefExpr' and n.get('referencedDecl',{}).get('kind')=='VarDecl':refs.append(n)
 for c in n.get('inner',[]):walk(c,infn)
walk(ast)
def span(n):r=n['range'];return r['begin']['offset'],r['end']['offset']+r['end'].get('tokLen',1)
def dependencies(n):
 ids=[]
 def rec(x):
  if x.get('kind')=='DeclRefExpr':ids.append(x.get('referencedDecl',{}).get('id'))
  for y in x.get('inner',[]):rec(y)
 rec(n);return ids
moved={k for k,v in vars.items() if not v['abi'][2]}
while True:
 extra={k for k,v in vars.items() if any(x in moved for x in dependencies(v['node']))}
 if extra<=moved:break
 moved|=extra
entries=sorted([vars[k] for k in moved],key=lambda v:v['node']['loc']['offset']);offset=0
for i,v in enumerate(entries):
 n=v['node'];a=max(16,v['abi'][1]);offset=(offset+a-1)//a*a;v['off']=offset;v['i']=i;v['id']=n['id'];offset+=v['abi'][0]
byid={v['id']:v for v in entries};initranges=[]
for v in entries:
 n=v['node'];v['init']=None
 if n.get('init'):
  for ch in n.get('inner',[]):
   if ch.get('kind','').endswith(('Expr','Literal','Operator')) or ch.get('kind')=='InitListExpr':v['init']=ch;break
  if v['init']:initranges.append(span(v['init']))
def replacement(r):
 v=byid[r['referencedDecl']['id']];name=v['node']['name'];return '(*(__typeof__('+name+')*)(sm_current->arena+'+str(v['off'])+'))'
refedits=[]
for r in refs:
 if r['referencedDecl']['id'] in byid:
  a,b=span(r)
  if not any(lo<=a<hi for lo,hi in initranges):refedits.append((a,b,replacement(r)))
def rewritten_init(v):
 a,b=span(v['init']);s=pre[a:b];ed=[]
 for r in refs:
  if r['referencedDecl']['id'] in byid:
   lo,hi=span(r)
   if a<=lo and hi<=b:ed.append((lo-a,hi-a,replacement(r)))
 for lo,hi,t in sorted(set(ed),reverse=True):s=s[:lo]+t+s[hi:]
 return s
initializers=[];insertions={}
for v in entries:
 if not v['init']:continue
 name=v['node']['name'];init=rewritten_init(v)
 code='{ __typeof__('+name+') sm_initial_'+str(v['i'])+' = '+init+'; memcpy(sm_current->arena+'+str(v['off'])+', &sm_initial_'+str(v['i'])+', sizeof('+name+')); }'
 if v['local']:
  end=pre.find(';',span(v['node'])[1])+1;assert end>0
  insertions.setdefault(end,[]).append('if(!sm_current->ready['+str(v['i'])+']){'+code+'sm_current->ready['+str(v['i'])+']=1;}')
 else:initializers.append(code)
edits=set(refedits)
for at,codes in insertions.items():edits.add((at,at,'\n'+''.join(codes)+'\n'))
last=len(pre)+1
for a,b,t in sorted(edits,reverse=True):
 assert b<=last,(a,b,last);pre=pre[:a]+t+pre[b:];last=a
prefix='struct sm_context { unsigned char *arena; unsigned char ready['+str(len(entries))+']; };\n'
suffix='\nstatic void sm_context_initialize(sm_context *ctx){sm_current=ctx;'+''.join(initializers)+'}\n'
suffix+='sm_context *sm_create(void){sm_context *c=calloc(1,sizeof *c);if(!c)return 0;c->arena=calloc(1,'+str(offset)+');if(!c->arena){free(c);return 0;}sm_context_initialize(c);sm_init_voice();int flat=0;for(int e=0;e<12;e++)flat+=ENGINES[e]->npresets;sm_select_program(c,flat);return c;}\n'
suffix+='void sm_destroy(sm_context *c){if(c){free(c->arena);free(c);}}\nint sm_context_bytes(void){return '+str(offset)+';}\n'
(out/'SynthMavenDSP.c').write_bytes((prefix+pre+suffix).encode('latin1'))
(out/'context-layout.json').write_text(json.dumps({'bytes':offset,'fields':[{'name':v['node']['name'],'bytes':v['abi'][0],'offset':v['off'],'local':v['local']} for v in entries]},indent=2))
print('DSP context:',len(entries),'isolated fields,',offset,'bytes per instance')
