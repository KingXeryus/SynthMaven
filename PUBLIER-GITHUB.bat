@echo off
title Clipper - publier sur GitHub
rem Publie Clipper 1.0 (experimental) sur ton GitHub : le code, la pre-release, l'installateur (GitHub Pages).
rem Lance d'abord INSTALL-CLIPPER.bat (il compile le firmware que la release et l'installateur publient).
rem Usage: PUBLIER-GITHUB.bat                 (le depot TON-NOM/clipper-fm1)
rem        PUBLIER-GITHUB.bat -Repo autre-nom
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\publish_github.ps1" %*
pause
