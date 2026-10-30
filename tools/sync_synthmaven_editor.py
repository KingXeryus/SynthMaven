#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Keep native mock banks byte-identical to a freshly compiled descdump."""
import argparse
import json
import re
from pathlib import Path

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--check", action="store_true")
args = parser.parse_args()
data = json.loads((root / "build/host/desc.json").read_text())
path = root / "web/editor.html"
old = text = path.read_text()
for name, bank in (("FACTORY_PK", data["FM6"]["factory"]), ("HYBRID_PK", data["HYBRID_PK"])):
    pattern = rf"const {name} = \[.*?\];"
    text, count = re.subn(pattern, f"const {name} = {json.dumps(bank, separators=(',', ':'))};", text, flags=re.S)
    if count != 1:
        raise SystemExit(f"Expected one {name} declaration, found {count}")
if args.check and text != old:
    raise SystemExit("Native editor banks differ: rebuild descdump and run tools/sync_synthmaven_editor.py")
if not args.check:
    path.write_text(text)
print("SynthMaven native editor banks match compiled firmware")
