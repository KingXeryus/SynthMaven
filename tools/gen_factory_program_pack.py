#!/usr/bin/env python3
"""Losslessly pack the player's eight 3.5 programs. Only inactive native-engine
payloads are omitted; all active parameters, native patches, FX and scenes remain.
Source records are SM_SOUND_BYTES + SM_PROGRAM_META, from verified USB backup.
"""
from pathlib import Path
from collections import defaultdict,deque
import hashlib,json
root=Path(__file__).resolve().parents[1]
raw=(root/'assets/factory/programs-3.5.bin').read_bytes()
assert len(raw)==8*3556
records=[];names=[]
for slot in range(8):
 x=raw[slot*3556:(slot+1)*3556];names.append(x[:12].split(b'\0')[0].decode())
 b=bytearray(x[:12]+x[66:68]+x[3028:3048]+x[3056:])
 for k in range(4):
  t=x[68+k*250:68+(k+1)*250];b+=t;e=t[0]
  if e in (12,22):
   b+=x[1068+k*128:1068+(k+1)*128]
   if e==12:b+=x[1580+k*16:1580+(k+1)*16]
  elif e==15:b+=x[1644+k*144:1644+(k+1)*144]
  elif e==19:b+=x[2220+k*138:2220+(k+1)*138]
  elif e==10:b+=x[2772+k*64:2772+(k+1)*64]
 records.append(b)
source=b''.join(records);offset=[0]
for r in records:offset.append(offset[-1]+len(r))
out=bytearray();hist=defaultdict(deque);i=0
while i<len(source):
 f=len(out);out.append(0)
 for bit in range(8):
  if i>=len(source):break
  q=hist[bytes(source[i:i+3])]
  while q and i-q[0]>16384:q.popleft()
  best=0;dist=0
  for pos in reversed(q):
   n=3
   while n<258 and i+n<len(source) and source[pos+n]==source[i+n]:n+=1
   if n>best:best=n;dist=i-pos
   if n==258:break
  n=best if best>=3 else 1
  if n>1:out[f]|=1<<bit;out.extend((dist-1).to_bytes(2,'little'));out.append(n-3)
  else:out.append(source[i])
  for j in range(i,i+n):hist[bytes(source[j:j+3])].append(j)
  i+=n
# Independent byte-for-byte decompression check.
decoded=bytearray();i=0
while i<len(out):
 flags=out[i];i+=1
 for bit in range(8):
  if i>=len(out):break
  if flags&(1<<bit):
   distance=int.from_bytes(out[i:i+2],'little')+1;n=out[i+2]+3;i+=3
   for _ in range(n):decoded.append(decoded[-distance])
  else:decoded.append(out[i]);i+=1
assert decoded==source
header='/* SPDX-License-Identifier: GPL-3.0-only */\n/* User-curated FM-1 programs, captured 2026-10-30. Generated; do not edit. */\n'
header+=f'#define SM_FACTORY_RAW_BYTES {len(source)}u\n'
header+='static const char SM_FACTORY_PROGRAM_NAMES[8][13]='+json.dumps(names).replace('[','{').replace(']','}')+';\n'
for typ,name,data in [('uint16_t','SM_FACTORY_OFF',offset),('uint8_t','SM_FACTORY_LZ',out)]:
 header+=f'static const {typ} {name}[]={{'+','.join(map(str,data))+'};\n'
(root/'firmware/src/factory_program_pack.h').write_text(header)
(root/'assets/factory/programs-3.5.json').write_text(json.dumps(dict(names=names,source_sha256=hashlib.sha256(raw).hexdigest(),record_bytes=3556,compact_bytes=len(source),packed_bytes=len(out),source='Verified 2026-10-30 USB backup: saved P1 and P3 merged; other slots retain user-curated factory fallbacks'),indent=2)+'\n')
print(len(source),'->',len(out),'bytes; exact round-trip')
