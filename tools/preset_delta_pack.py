"""Exact predecessor-delta table packing for immutable preset rows."""
def emit(name, rows):
 rows=[bytes(x) for x in rows];width=len(rows[0]);maskbytes=(width+7)//8
 assert len(rows)<255 and all(len(x)==width for x in rows)
 data=[];offset=[];decoded=[]
 for i,row in enumerate(rows):
  offset.append(len(data))
  if not i:data.extend([255]+list(row));decoded.append(row);continue
  parent=min(range(i),key=lambda j:sum(a!=b for a,b in zip(row,rows[j])))
  mask=[0]*maskbytes;values=[]
  for k,(a,b) in enumerate(zip(row,rows[parent])):
   if a!=b:mask[k//8]|=1<<(k%8);values.append(a)
  data.extend([parent]+mask+values)
  check=bytearray(decoded[parent]);v=iter(values)
  for k in range(width):
   if mask[k//8]&(1<<(k%8)):check[k]=next(v)
  assert bytes(check)==row;decoded.append(bytes(check))
 text=f'/* Exact predecessor deltas: {len(rows)*width} -> {len(data)+2*len(rows)} bytes. */\n'
 text+=f'static const uint16_t {name}_OFF[]={{'+','.join(map(str,offset))+'};\n'
 text+=f'static const uint8_t {name}_DATA[]={{'+','.join(map(str,data))+'};\n'
 text+=f'''static void {name}_read(uint32_t index,uint8_t *out)
{{
    uint8_t chain[{len(rows)}];uint32_t n=0;
    if(index>={len(rows)}u)index=0;
    while(index){{chain[n++]=(uint8_t)index;index={name}_DATA[{name}_OFF[index]];}}
    memcpy(out,{name}_DATA+1,{width});
    while(n){{
        const uint8_t *mask={name}_DATA+{name}_OFF[chain[--n]]+1,*value=mask+{maskbytes};
        for(uint32_t i=0;i<{width};i++)if(mask[i/8]&(1u<<(i%8)))out[i]=*value++;
    }}
}}
'''
 return text
