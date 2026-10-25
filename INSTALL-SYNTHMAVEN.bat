@echo off
title SynthMaven installer
rem Usage: INSTALL-SYNTHMAVEN.bat        (8 voices)
rem        INSTALL-SYNTHMAVEN.bat 12     (12 voices: the CPU test, see DEMARRAGE-RAPIDE-FR.md)
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build-synthmaven.ps1" %1
pause
