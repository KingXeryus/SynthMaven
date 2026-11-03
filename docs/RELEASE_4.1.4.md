# SynthMaven 4.1.4 — updates keep your programs and sounds

4.1.4 is 4.1.3 with one change: installing it no longer erases what you saved.

## What changes

- **Your programs, sounds and settings stay:** installing over SynthMaven 3.5 or later keeps saved programs, USER sounds (including Prophet, FM6 and CZ-1 slots), projects, scales, MIDI mappings and settings. Before, every install reset the FM-1 to factory defaults.
- **New installs still start clean:** over stock firmware, SynthMaven 3.0 or anything else that didn't save SynthMaven 3.5+ settings, the installer still clears musical storage, as before.

This applies from this install on: the update loader that decides is the one in the package being installed, so installing 4.1.4 over 4.1.3 already keeps your data.

## Install

1. Export a complete backup in the editor anyway. Your data should stay, but a backup lets you recover it if the install is interrupted.
2. Use the live installer in Chrome or Edge with USB ROLE set to COMPUTER, and keep the FM-1 plugged in until it restarts.

The editors are unchanged: the web editor and the iPad app 4.1 (25) work with 4.1.4.

## Validation

Host tests run the update loader over a simulated FM-1: with SynthMaven settings in either settings copy, nothing above the app area is erased; with no settings, broken settings or another firmware's settings, all musical storage is cleared and verified, and power cuts during the clear resume on retry. A firmware boot test checks that the settings record SynthMaven writes carries the tag where the loader reads it, and the settings layout is unchanged from 3.5 through 4.1.4.
