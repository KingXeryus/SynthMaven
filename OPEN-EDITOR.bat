@echo off
title SynthMaven editor
cd /d "%~dp0"
if not exist build\synthmaven-site\webapp\editor\index.html (
  echo The editor is not built yet: run INSTALL-SYNTHMAVEN.bat first.
  pause
  exit /b 1
)
start "" http://localhost:8767/webapp/editor/
echo SynthMaven editor: http://localhost:8767/webapp/editor/  (keep this window open while you use it)
python -m http.server 8767 --bind 127.0.0.1 --directory build/synthmaven-site
pause
