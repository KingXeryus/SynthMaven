#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Bundle corresponding project and JUCE sources, excluding build outputs."""
from pathlib import Path
import os,subprocess,sys,zipfile
root=Path(__file__).resolve().parents[2];output=Path(sys.argv[1]).resolve();output.parent.mkdir(parents=True,exist_ok=True)
git=os.environ.get('GIT','git')
files=subprocess.check_output([git,'-C',str(root),'ls-files','--cached','--others','--exclude-standard','-z']).decode().split('\0')
files=[root/f for f in files if f and (root/f).is_file()]
files+=list((root/'desktop/vendor/JUCE-7.0.12').rglob('*'))
with zipfile.ZipFile(output,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for f in sorted(set(files)):
  if f.is_file() and '.DS_Store' not in f.parts:z.write(f,'SynthMaven/'+str(f.relative_to(root)))
print(output)
