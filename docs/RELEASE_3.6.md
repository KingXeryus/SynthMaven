# SynthMaven 3.6 — MIDI routing, pooled polyphony, new factory programs and the SynthMaven FM1 name

SynthMaven 3.6 is the new main line. It uses the same code as 3.5.9, and brings together everything since 3.5.

## MIDI

- **Settings → MIDI → Routing** assigns an input (Any, USB or TRS) and a channel (Off, 1–16 or All) to layers 1–4 and the Drummer. Routing is off by default, and layers on the same input and channel split or stack by note range.
- MIDI Learn adds Transport Play / Stop, Record (selected part) and Record (drummer).
- The web editor forwards several controllers on a computer, each to its target's channel. The iPad app still forwards one at a time.
- The USB/TRS/BOTH setting is called MIDI MODE and defaults to BOTH. The old MIDI MODE row (Absolute/Rel) is now KNOB MODE.

## Sound

- BUDGET defaults to 64 points, its maximum. ANALOG, DIGITAL, VOICE, STRINGS, JUNO, HYBRID, WSEQ and VPS can use up to 16 voices a part from the shared pool. If a heavy program drops notes or crackles, lower BUDGET in GLO → TUNE/CLOCK.
- Eight new factory programs: DX7 E.PIANO, DX RHODES, JUNO 106, E. ORGAN, DISCO LEAD, PROPHET LEAD, RETRO SYNTH and ORCHESTRA. Each opens on its Layer 1 lead; hold GLO to bring in the supporting layers.
- A fresh installation starts on Program 1.

## Device name

The FM-1 identifies itself as **SynthMaven FM1**, as both its MIDI port and its USB audio interface, and as SynthMaven FM1 Update while installing. If a Mac keeps showing the old name, unplug the device, delete its grey entry in Audio MIDI Setup → MIDI Studio, and plug it back in.

M-VAVE's own updater may no longer find a device running this firmware. Return to stock firmware from the SynthMaven installer page.

## Controls

- Hold HOME for the settings menu: SELECT cycles the rows, KNOB 1 changes the selected row, and KNOB 4 leaves. MIDI MODE and USB ROLE replace HOLD and LIGHTS in the list; HOLD and LIGHTS remain in GLO's preference pages.
- On the tuning keyboard, hold a key for one second to set −50 cents or reset it to zero.

## Installation

1. Export a complete backup first: installation resets settings and musical storage to factory defaults.
2. Set USB ROLE to COMPUTER and install with the web installer and a data cable. Restore your backup afterwards.

## Validation and limitations

Host and web tests cover routing, the USB names, the menu and the Program 1 default. **This release has not been tested on FM-1 hardware,** the factory programs have not been auditioned, and the new name has not been seen on a computer. USB ROLE still returns to COMPUTER at each power-on. The iPad app is still 3.5.6 (18).
