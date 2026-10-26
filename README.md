# SynthMaven for M-VAVE FM-1

SynthMaven 1.8 provides 23 synthesis engines and 537 factory presets, including 88 Oriental/Middle Eastern sounds and nine XD compatibility sounds. HOME holds a four-sound program: ALGORITHM cycles engines, PRESET cycles presets within the engine, and SELECT selects sound 1–4. Programs can be saved/loaded with layering ON or OFF.

Tap SEL to open the scale browser, then turn SELECT to cycle scales. Hold it three seconds for a latched custom-scale editor with four saved chromatic tunings, selectable root and 0/−25/−50/−75-cent key offsets. The 46 built-in Arabic maqam reference tunings remain available.

Bluetooth is removed. GLO → USB ROLE → PEDAL enables direct class-compliant USB MIDI host operation using a powered adapter or hub. COMPUTER mode serves the editor, audio and updates. Direct Chocolate Plus operation and audio timing still require hardware validation. Hold OCT− at power-on for USB recovery.

[Installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/) · [Editor](https://kingxeryus.github.io/SynthMaven/webapp/editor/) · [User manual](docs/USER_MANUAL.md) · [Quick start](QUICKSTART.md) · [Release notes](docs/release-1.8.md) · [Maqam catalogue](docs/MAQAM_SCALES.md)

Back up before installing. See [USB MIDI status](docs/USB_MIDI_STATUS.md) for compatibility limits and [preset catalogue](docs/release-1.0.md) for the sound library.

## Building and verification

See [BUILDING.md](BUILDING.md). Linux x86-64 runs the JieLi toolchain directly; macOS requires Docker. Run `sh tools/test_synthmaven.sh`, then `sh build.sh --release 1.8`. The build enables USB host support and excludes Bluetooth. Source and licence notices are published alongside the installer.

## Credits and licence

SynthMaven is based on [Clipper by 3dSam](https://github.com/isod89/clipper-fm1), which builds on SLOOP and Felucca. JUNO, VECTOR, FMX, WSEQ, MACRO, the filter models, TURNG and SPICE / DICE are inherited from Clipper; their hardware and synthesis inspirations do not establish SynthMaven authorship.

GPL-3.0-only; see [LICENSING.md](LICENSING.md). USB host additions retain their MPL-2.0 notices in firmware/vendor/fm1-usb-host. Original firmware work and engine attributions are retained in source.

Version 1.5 introduced calibrated factory levels (536 audible sounds; USER WT stays empty) and a shared instrument-style editor/installer design with the logo palette and Dark, Light or System appearance. See the release notes for the reference measurement method and limits.

The four-sound layer mixer includes independent split zones, octave shift, pan, level, mute and solo, adapted from the standalone instrument. Programs preserve the complete mix and upgrade existing PRG1 saves.

Version 1.6 added QNT ALL and default-on per-sound master tuning switches. Version 1.7 provides a dedicated layering screen and a two-second GLO layering toggle. See the current release notes for details.

At two seconds of holding GLO, layering toggles once and the display shows **Layered ON** or **Layered OFF**. Short taps open GLO without flashing this screen. Release GLO to return to the previous screen. Other function buttons retain their three-second hold behavior.

Version 1.7 provides eight named saved programs, SELECT slot browsing, 12-character names edited with ALGORITHM plus PRESET/KNOB 2, and a streamlined device SAVE menu. Layering ON/OFF remains in GLO and on its two-second hold.


### Scale browsing and four-sound mixer (1.8)

Tap SEL to open the scale browser without changing the scale. SELECT cycles the tuning list; PRESET or KNOB 1 changes ROOT, KNOB 2 also browses scales, and KNOBS 3/4 retain QNT/CHORD. Hold SEL for three seconds for custom-scale editing.

EDIT now has four standard pages: EDIT 1, EDIT 2, VOICE and CHARACTER. FMX keeps its operator/routing pages; OP ENV is under ENV. Macros, morph, randomize, undo and FMX STORE remain accessible in the editor. Layer ON/OFF stays in GLO and on its two-second hold.

Hold EDIT for two seconds to preview the four-sound mixer. KNOBS 1–4 control sounds 1–4; SELECT switches between VOLUME and PAN. At three seconds the mixer latches and stays open after release. Tap EDIT or HOME to exit; save a program to keep the mix. The keyboard still plays and EDIT no longer erases sequence notes. Short presses never show a held-layer screen. HOME opens its menu after three seconds.
