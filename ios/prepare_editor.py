#!/usr/bin/env python3
"""Bundle the production editor without a separate iOS interface fork."""
from pathlib import Path
import shutil,json
root=Path(__file__).resolve().parents[1];dest=root/'ios/Generated/Editor';editor=dest/'webapp/editor';editor.mkdir(parents=True,exist_ok=True)
s=(root/'web/editor.html').read_text()
# Native BLE drivers need more reply time than desktop USB; retain serialized requests/coalescing.
s=s.replace('timeout: opt.timeout || this.timeout,','timeout: Math.max(opt.timeout || this.timeout, 10000),')
s=s.replace('async function connect() {',(root/'ios/UI/perform-bindings.js').read_text()+'\n'+'window.SynthMavenConnect = () => { if (!dev) connect(); };\nasync function connect() {')
s=s.replace('width=device-width, initial-scale=1','width=device-width, initial-scale=1, viewport-fit=cover')
s=s.replace('href="../installer/"','href="https://kingxeryus.github.io/SynthMaven/webapp/installer/"').replace('href="../../docs/midi-controllers.html"','href="https://kingxeryus.github.io/SynthMaven/docs/midi-controllers.html"')
s=s.replace('</head>','<link rel="stylesheet" href="mobile.css"><script src="mobile.js" defer></script></head>')
(editor/'index.html').write_text(s)
for f in ['mobile.css','mobile.js']:shutil.copy(root/'ios/UI'/f,editor/f)
for f in ['synthmaven-theme.css','synthmaven-theme.js','fukiai.ttf','FUKIAI-LICENSE.txt']:shutil.copy(root/'web'/f,editor/f)
shutil.copy(root/'ios/Branding/in-app-logo.svg',editor/'synthmaven-logo.svg')
shutil.copytree(root/'web/fonts',dest/'webapp/installer/fonts',dirs_exist_ok=True,copy_function=shutil.copyfile)
shutil.copy(root/'LICENSE',dest/'LICENSE.txt')
shutil.copy(root/'docs/MIDI_CONTROLLERS.md',dest/'MIDI_CONTROLLERS.md')
print('Bundled existing editor for firmware',json.loads((root/'release.json').read_text())['version'])
