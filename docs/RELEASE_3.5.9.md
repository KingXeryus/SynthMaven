# SynthMaven 3.5.9 — device name, settings menu and Program 1

## Device name

The FM-1 now identifies itself as **SynthMaven FM1**, as both its MIDI port and its USB audio interface, and as SynthMaven FM1 Update while installing. The web editor, iPad editor and installer use the same name. If a Mac keeps showing the old name, unplug the device, delete its grey entry in Audio MIDI Setup → MIDI Studio, and plug it back in. Refresh any installer tab opened before this release.

M-VAVE's own updater may no longer find a device running this firmware. Return to stock firmware from the SynthMaven installer page.

## Settings menu (hold HOME)

- SELECT cycles the rows, KNOB 1 changes the selected row, and KNOB 4 leaves. OCT+ and OCT- work as before. KNOB 2 no longer moves the row.
- MIDI MODE (USB, TRS or BOTH) and USB ROLE replace HOLD and LIGHTS in the list. HOLD and LIGHTS remain in GLO's preference pages.
- The USB/TRS/BOTH setting is now called MIDI MODE everywhere; the knob column on GLO → SYSTEM shortens it to MODE. The old MIDI MODE row (Absolute/Rel) is now KNOB MODE.

## Fixes

A fresh installation now starts on Program 1. 3.5.7 started on Program 3 despite its documentation.

## Installation

1. Export a complete backup first: installation resets settings and musical storage to factory defaults.
2. Set USB ROLE to COMPUTER and install with the web installer and a data cable. Restore your backup afterwards.

## Validation and limitations

Host tests cover the USB names, the installer's port matching, the menu and the Program 1 default. **This release has not been tested on FM-1 hardware, and the new name has not been seen on a computer.** USB ROLE still returns to COMPUTER at each power-on.
