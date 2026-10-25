@echo off
title SynthMaven - publish on GitHub
rem Publishes SynthMaven 1.1 (experimental) on your GitHub: the source, the pre-release, the installer (GitHub Pages).
rem Run INSTALL-SYNTHMAVEN.bat first (it builds the firmware the release and the installer publish).
rem Usage: PUBLISH-GITHUB.bat                 (the repository YOUR-NAME/synthmaven-fm1)
rem        PUBLISH-GITHUB.bat -Repo another-name
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\publish_github.ps1" %*
pause
