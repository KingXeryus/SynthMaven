# SynthMaven for M-VAVE FM-1

SynthMaven 1.5 provides 23 synthesis engines and 537 factory presets, including 88 Oriental/Middle Eastern sounds and nine XD compatibility sounds. HOME holds a four-sound program: ALGORITHM cycles engines, PRESET cycles presets within the engine, and SELECT selects sound 1–4. Programs can be saved/loaded with layering ON or OFF.

Tap SEL to cycle scales. Hold it three seconds for a latched custom-scale editor with four saved chromatic tunings, selectable root and 0/−25/−50/−75-cent key offsets. The 46 built-in Arabic maqam reference tunings remain available.

Bluetooth is removed. GLO → USB ROLE → PEDAL enables direct class-compliant USB MIDI host operation using a powered adapter or hub. COMPUTER mode serves the editor, audio and updates. Direct Chocolate Plus operation and audio timing still require hardware validation. Hold OCT− at power-on for USB recovery.

[Installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/) · [Editor](https://kingxeryus.github.io/SynthMaven/webapp/editor/) · [User manual](docs/USER_MANUAL.md) · [Quick start](QUICKSTART.md) · [Release notes](docs/release-1.5.md) · [Maqam catalogue](docs/MAQAM_SCALES.md)

Back up before installing. See [USB MIDI status](docs/USB_MIDI_STATUS.md) for compatibility limits and [preset catalogue](docs/release-1.0.md) for the sound library.

## Building and verification

See [BUILDING.md](BUILDING.md). Linux x86-64 runs the JieLi toolchain directly; macOS requires Docker. Run `sh tools/test_synthmaven.sh`, then `sh build.sh --release 1.5`. The build enables USB host support and excludes Bluetooth. Source and licence notices are published alongside the installer.

## Credits and licence

GPL-3.0-only; see [LICENSING.md](LICENSING.md). USB host additions retain their MPL-2.0 notices in firmware/vendor/fm1-usb-host. Original firmware work and engine attributions are retained in source.

Version 1.5 adds calibrated factory levels (536 audible sounds; USER WT stays empty) and a shared instrument-style editor/installer design with the logo palette and Dark, Light or System appearance. See the release notes for the reference measurement method and limits.
