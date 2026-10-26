# SynthMaven for M-VAVE FM-1

SynthMaven 1.7 provides 23 synthesis engines and 537 factory presets, including 88 Oriental/Middle Eastern sounds and nine XD compatibility sounds. HOME holds a four-sound program: ALGORITHM cycles engines, PRESET cycles presets within the engine, and SELECT selects sound 1–4. Programs can be saved/loaded with layering ON or OFF.

Tap SEL to cycle scales. Hold it three seconds for a latched custom-scale editor with four saved chromatic tunings, selectable root and 0/−25/−50/−75-cent key offsets. The 46 built-in Arabic maqam reference tunings remain available.

Bluetooth is removed. GLO → USB ROLE → PEDAL enables direct class-compliant USB MIDI host operation using a powered adapter or hub. COMPUTER mode serves the editor, audio and updates. Direct Chocolate Plus operation and audio timing still require hardware validation. Hold OCT− at power-on for USB recovery.

[Installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/) · [Editor](https://kingxeryus.github.io/SynthMaven/webapp/editor/) · [User manual](docs/USER_MANUAL.md) · [Quick start](QUICKSTART.md) · [Release notes](docs/release-1.7.md) · [Maqam catalogue](docs/MAQAM_SCALES.md)

Back up before installing. See [USB MIDI status](docs/USB_MIDI_STATUS.md) for compatibility limits and [preset catalogue](docs/release-1.0.md) for the sound library.

## Building and verification

See [BUILDING.md](BUILDING.md). Linux x86-64 runs the JieLi toolchain directly; macOS requires Docker. Run `sh tools/test_synthmaven.sh`, then `sh build.sh --release 1.7`. The build enables USB host support and excludes Bluetooth. Source and licence notices are published alongside the installer.

## Credits and licence

SynthMaven is based on [Clipper by 3dSam](https://github.com/isod89/clipper-fm1), which builds on SLOOP and Felucca. JUNO, VECTOR, FMX, WSEQ, MACRO, the filter models, TURNG and SPICE / DICE are inherited from Clipper; their hardware and synthesis inspirations do not establish SynthMaven authorship.

GPL-3.0-only; see [LICENSING.md](LICENSING.md). USB host additions retain their MPL-2.0 notices in firmware/vendor/fm1-usb-host. Original firmware work and engine attributions are retained in source.

Version 1.5 introduced calibrated factory levels (536 audible sounds; USER WT stays empty) and a shared instrument-style editor/installer design with the logo palette and Dark, Light or System appearance. See the release notes for the reference measurement method and limits.

The four-sound layer mixer includes independent split zones, octave shift, pan, level, mute and solo, adapted from the standalone instrument. Programs preserve the complete mix and upgrade existing PRG1 saves.

Version 1.6 added QNT ALL and default-on per-sound master tuning switches. Version 1.7 provides a dedicated layering screen and a two-second GLO layering toggle. See the current release notes for details.

Holding GLO replaces the current display with **Layered ON** or **Layered OFF**. At two seconds, layering toggles once and the label updates. Release GLO to return to the previous screen. Other function buttons retain their three-second hold behavior.
