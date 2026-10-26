"""Publish readable, version-matched documentation alongside the installer."""
import html
import posixpath
import re
from pathlib import Path
import markdown

ROOT = Path(__file__).resolve().parents[1]
PAGES = {
    'docs/USER_MANUAL.md': 'user-manual.html',
    'QUICKSTART.md': 'quick-start.html',
    'docs/release-1.3.md': 'release-notes.html',
    'docs/release-1.2.md': 'release-1.2.html',
    'docs/MAQAM_SCALES.md': 'maqam-scales.html',
    'docs/release-1.1.md': 'release-1.1.html',
    'docs/release-1.0.md': 'preset-catalog.html',
    'docs/BLUETOOTH_STATUS.md': 'bluetooth-status.html',
    'docs/USB_MIDI_STATUS.md': 'usb-midi-status.html',
    'docs/SYNTHMAVEN_MIDI.md': 'midi.html',
    'BUILDING.md': 'building.html',
}

def publish_docs(out):
    dest = Path(out) / 'docs'
    dest.mkdir(parents=True, exist_ok=True)
    for source, name in PAGES.items():
        text = (ROOT / source).read_text()
        def link(match):
            target = match[1]
            if target.startswith(('#', 'https:', 'http:', 'mailto:')):
                return match[0]
            path, sep, fragment = target.partition('#')
            resolved = posixpath.normpath(posixpath.join(posixpath.dirname(source), path))
            url = PAGES.get(resolved)
            if not url:
                url = 'https://github.com/KingXeryus/SynthMaven/blob/main/' + resolved
            return '](' + url + (sep + fragment if sep else '') + ')'
        text = re.sub(r'\]\(([^)]+)\)', link, text)
        body = markdown.markdown(text, extensions=['tables', 'fenced_code', 'toc'])
        title = html.escape(text.splitlines()[0].lstrip('# '))
        page = f'''<!doctype html><html lang="en"><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>{title}</title>
<style>
:root{{color-scheme:dark}}body{{font:17px/1.65 system-ui,sans-serif;background:#101013;color:#eee;max-width:960px;margin:0 auto;padding:28px 22px 80px}}
a{{color:#70b3ff}}nav{{display:flex;flex-wrap:wrap;gap:10px 24px;padding-bottom:20px;border-bottom:1px solid #444}}
h1,h2,h3{{line-height:1.25;margin-top:1.6em}}h1{{font-size:2em}}table{{border-collapse:collapse;display:block;overflow:auto}}td,th{{border:1px solid #555;padding:8px 12px;text-align:left}}code{{background:#25252c;padding:2px 5px}}pre{{overflow:auto;background:#25252c;padding:15px}}pre code{{padding:0}}img{{max-width:100%}}
</style>
<nav aria-label="Documentation"><a href="../webapp/installer/">Installer</a><a href="../webapp/editor/">Editor</a><a href="user-manual.html">User manual</a><a href="quick-start.html">Quick-start guide</a><a href="release-notes.html">Release notes</a></nav>
<main>{body}</main></html>'''
        (dest / name).write_text(page)
