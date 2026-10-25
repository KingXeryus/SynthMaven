#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Encode the supplied SynthMaven logo as the hardware's RGB565/RLE boot splash."""
from pathlib import Path
import sys,shutil
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
W,H=240,188
COLS=[(0,0,0),(255,255,255),(205,32,32)]+[(v,v,v) for v in (32,64,96,128,160,192,224)]+[(v,24,24) for v in (48,80,112,144,176,224)]
def rgb565(c):
 r,g,b=c;return ((r>>3)<<11)|((g>>2)<<5)|(b>>3)
def splash_header(path):
 im=Image.open(ROOT/'assets/logo/synthmaven-splash.png').convert('RGB').resize((W,H),Image.Resampling.LANCZOS)
 cache={};px=[]
 for c in im.getdata():
  if c not in cache:cache[c]=min(range(16),key=lambda k:sum((c[j]-COLS[k][j])**2 for j in range(3)))
  px.append(cache[c])
 data=[];i=0
 while i<len(px):
  j=i+1
  while j<len(px) and px[j]==px[i] and j-i<16:j+=1
  data.append(((j-i-1)<<4)|px[i]);i=j
 out=['/* Generated SynthMaven artwork. */','#pragma once','#include <stdint.h>',f'#define SLOOP_SPLASH_W {W}',f'#define SLOOP_SPLASH_H {H}','static const uint16_t SLOOP_SPLASH_PAL[16] = {'+','.join(str(rgb565(c)) for c in COLS)+'};',f'static const uint8_t SLOOP_SPLASH_RLE[{len(data)}] = {{']
 out += [','.join(map(str,data[k:k+24]))+',' for k in range(0,len(data),24)]
 out+=['};'];Path(path).write_text('\n'.join(out)+'\n')
if __name__=='__main__':
 if len(sys.argv)==3 and sys.argv[1]=='--assets':
  dest=Path(sys.argv[2]);dest.mkdir(parents=True,exist_ok=True)
  for f in (ROOT/'assets/logo').iterdir():
   if f.resolve()!= (dest/f.name).resolve():shutil.copy(f,dest/f.name)
 else:splash_header(sys.argv[1])
