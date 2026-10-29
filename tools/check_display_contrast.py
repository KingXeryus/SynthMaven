#!/usr/bin/env python3
"""Check actual RGB565 colors used for readable device text and selected cards."""
import re
from pathlib import Path
root=Path(__file__).resolve().parents[1]
gfx=(root/'firmware/src/gfx.c').read_text();studio=(root/'firmware/src/ui_studio.c').read_text()
def colors(s):return [tuple(map(int,m)) for m in re.findall(r'RGB\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)',s)]
def lum(rgb):
 # Exactly the channel truncation used by RGB(), then the panel's channel range.
 vals=[(rgb[0]>>3)/31,(rgb[1]>>2)/63,(rgb[2]>>3)/31]
 return sum(w*(v/12.92 if v<=.04045 else ((v+.055)/1.055)**2.4) for v,w in zip(vals,[.2126,.7152,.0722]))
def ratio(a,b):
 a,b=sorted([lum(a),lum(b)]);return (b+.05)/(a+.05)
block=gfx[gfx.index('static const palette_t PALETTES'):gfx.index('#define NPALETTES')];pal=colors(block);assert len(pal)==25
text=[c for i,c in enumerate(pal) if i%5!=0]
text+=colors(studio[studio.index('static const uint16_t TE_COL'):studio.index('static const uint16_t TE_CARD')])
text+=colors(re.search(r'#define TE_G3[^\n]*',studio)[0])
backgrounds=[(0,0,0),(26,26,30),(54,54,60)]
backgrounds+=colors(studio[studio.index('static const uint16_t TE_CARD'):studio.index('static const uint16_t TE_MID')])
worst=min((ratio(t,b),t,b) for t in text for b in backgrounds)
assert worst[0]>=4.5,f'Unreadable text/background pair: {worst}'
print(f'Device text/selection colors: RGB565 minimum contrast {worst[0]:.2f}:1 across black and both card surfaces PASS')
