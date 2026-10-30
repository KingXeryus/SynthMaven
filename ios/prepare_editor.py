#!/usr/bin/env python3
"""Bundle the same Melodee-protocol editor used by the SynthMaven 3.5 website."""
from pathlib import Path
import shutil
root=Path(__file__).resolve().parents[1]
dest=root/'ios/Generated/Editor'
editor=dest/'webapp/editor';editor.mkdir(parents=True,exist_ok=True)
s=(root/'web/editor.html').read_text()
s=s.replace('width=device-width, initial-scale=1','width=device-width, initial-scale=1, viewport-fit=cover')
s=s.replace('async function connect() {','window.SynthMavenConnect = () => { if (!dev) connect(); };\nasync function connect() {',1)
assert 'window.SynthMavenConnect' in s
(editor/'index.html').write_text(s)
for name in ['fukiai.ttf','FUKIAI-LICENSE.txt','viewport-pages.js','workspace-pages.js','sound-visuals.js','native-workspace.js','editor-choice-controls.js','fm1backup.js']:shutil.copy(root/'web'/name,editor/name)
shutil.copytree(root/'web/branding',editor/'branding',dirs_exist_ok=True)
shutil.copy(root/'LICENSE',dest/'LICENSE.txt')
print('Bundled SynthMaven 3.5 USB editor; no legacy protocol bindings injected')
