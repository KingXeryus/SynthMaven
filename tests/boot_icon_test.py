#!/usr/bin/env python3
"""Packed startup artwork must preserve every RGB565 pixel and row boundary."""
import re
import unittest
from pathlib import Path
from PIL import Image
ROOT = Path(__file__).resolve().parents[1]
class BootIconTest(unittest.TestCase):
    def test_pixel_exact(self):
        text=(ROOT/'firmware/src/sm_boot_icon.h').read_text()
        arrays=re.findall(r'=\{([\d,]+)\}',text)
        palette,runs=[list(map(int,a.split(','))) for a in arrays]
        pixels=[];row=0
        for run in runs:
            count=(run>>2)+1
            self.assertLessEqual(row+count,96)
            pixels.extend([palette[run&3]]*count)
            row=(row+count)%96
        self.assertEqual(len(pixels),96*96)
        image=Image.open(ROOT/'ios/Branding/hybrid-app-icon.png').convert('RGB').resize((96,96),Image.Resampling.LANCZOS).quantize(colors=4).convert('RGB')
        expected=[]
        for r,g,b in image.getdata():
            c=((r>>3)<<11)|((g>>2)<<5)|(b>>3)
            expected.append(((c&255)<<8)|(c>>8))
        self.assertEqual(pixels,expected)
        self.assertLess(len(runs)+len(palette)*2,700)
if __name__=='__main__': unittest.main()
