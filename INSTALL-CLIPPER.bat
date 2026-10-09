@echo off
title Clipper installer
rem Usage: INSTALL-CLIPPER.bat        (8 voices)
rem        INSTALL-CLIPPER.bat 12     (12 voices: the CPU test, see DEMARRAGE-RAPIDE-FR.md)
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build-clipper.ps1" %1
pause
