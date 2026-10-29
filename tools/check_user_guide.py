#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Release gate for version, screen/control coverage, PDF integrity and installer navigation."""
from pathlib import Path
import json,hashlib
import pymupdf
ROOT=Path(__file__).resolve().parents[1];meta=json.loads((ROOT/'release.json').read_text());v=meta['version'];p=ROOT/'docs'/f'SynthMaven-{v}-User-Guide.pdf';d=pymupdf.open(p)
coverage=json.loads((ROOT/'assets/guide/pdf-coverage.json').read_text());inventory=json.loads((ROOT/'assets/guide/screen-inventory.json').read_text())
assert coverage['version']==v and coverage['protocol']==meta['protocol']
assert {r['image'] for r in coverage['reference']}=={r['image'] for r in inventory}
assert len({r['page_id'] for r in inventory})==coverage['menu_pages']==65
assert len(d)>200 and len(d.get_toc())>180
for row in coverage['reference']:assert d[row['page']-1].get_images(full=True)
for page in d:
 assert len(page.get_text())>100
 for b in page.get_text('blocks'):assert b[0]>=-1 and b[1]>=-1 and b[2]<=page.rect.width+1 and b[3]<=page.rect.height+1
assert 'Firmware '+v in d[-1].get_text()
assert p.name in (ROOT/'web/index_pkg.html').read_text()
print(f'Guide: {len(d)} pages, {len(d.get_toc())} bookmarks, all menus/layouts pictured, versioned installer PDF link PASS')
