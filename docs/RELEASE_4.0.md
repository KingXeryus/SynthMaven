# SynthMaven 4.0 — the oriental groove box

An FM-1 firmware for oriental live musicians, turning the standard M-VAVE FM-1 into an oriental groove box.

## New in 4.0

- **New design and themes:** every screen now has the same cards and colours. Choose SynthMaven (red, the default), Orange, Baby Blue or Pink with COLOR in Settings. The earlier themes and the pixel font are removed, and Rubik gains a smaller size for the Home cards.
- **Factory grooves:** on drum Home, turn ALGORITHM to choose Ballad, Half Pipe, Straight 8, On the Floor, Maksoom, Wahda, Baladi or Fox. Turn back to MY PATTERN to get your own steps again; editing a groove makes it your pattern. ALGORITHM no longer selects a lane here, so use SELECT.
- **Factory programs:** RETRO SYNTH now leads with XD STRINGS and ORCHESTRA with FM6 STRINGS 2. DX RHODES and ORCHESTRA load with layering on.
- **Loudness compensation:** factory sounds and programs are now matched to the DX7 program's loudness, automatically and with no control of their own. Your LEVEL settings work as before. 487 of 614 sounds land within 1 dB and 563 within 3 dB; a few short or very quiet sounds stay lower because their boost is limited. Sounds saved as USER sounds aren't compensated.
- **Installer:** the optional m-bootloader step is gone, and the page now shows the firmware's key numbers. A device that already has m-bootloader keeps it.
- **New arranger:** in the editors it has its own page after Sequencer and Drums. Rows are the four layers plus Drums, and columns are the sections. On the FM-1, SEQ → SONG has the same five rows. A section can change the drum groove, use your own pattern, or stop the drums.
- **Save pattern:** save the pattern you're editing to any of a track's 8 patterns, from the editor's sequencer or SEQ → PATTERNS on the FM-1.
- **Tempo while drumming:** while the drums are running, SELECT sets the tempo on both Home screens. Stop the drums to choose a part or lane with SELECT again.
- **New Home screen:** four cards, one per layer, with the sound, engine, pattern and live steps, and four rotary knobs below showing KNOB 1–4 for the selected layer.  The header shows the program number and name. Each layer's level and pan are on the mixer page.

## What's in the box

Microtuning for maqam, four-layer key mapping, the groove box and sequencer, scenes and setlists, the web and iPad editors, and per-layer MIDI routing for several keyboards. See the [README](../README.md). This release also includes the changes from 3.6.

## Installation

1. Export a complete backup first: installation resets settings and musical storage to factory defaults.
2. Set USB ROLE to COMPUTER and install with the web installer and a data cable. Restore your backup afterwards.

## Validation and limitations

Host tests cover the groove browser and the new screens. **This release has not been tested on FM-1 hardware.** The groove patterns haven't been auditioned. USB ROLE still returns to COMPUTER at each power-on. The iPad app is still 3.5.6 (18).
