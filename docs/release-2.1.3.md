# SynthMaven 2.1.3 — automatic MIDI controllers, tuning octave boundary and updated logo

- GLO > TUNING Knob 2 now selects FROM C0–C8, moving by octaves only.
- The selected scale applies inclusively from that C upward. Lower notes use equal-tempered intervals, including during glides. The A4 reference stays global.
- Knob 3 keeps the tuning root. A4 is retained on TUNING 2. Other navigation and shortcuts are unchanged.
- The editor mirrors the Apply from octave control. Settings, four-part programs, named slots and scenes preserve the threshold in their existing record sizes. Legacy records default to C0.
- Protocol 23 adds an optional threshold byte after A4; old-editor writes preserve the threshold, and the control stays hidden for old devices.
- The supplied InApp Logo.svg replaces the installer/editor logo and supplies the rendered device splash.

- Automatic USB controller identification for Alesis, Novation, Arturia, Akai and Mackie families, with safe standard-MIDI fallback.
- Native four-part MCU/HUI mixer and transport support, separate music/DAW cable routing, and APC Mini / modern Launchpad grid mappings. See [controller coverage and limitations](MIDI_CONTROLLERS.md).

## Installation and recovery

Back up your programs/settings. Use the [installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/) in Chrome or Edge, keeping USB power connected during writing. Downloads include firmware, checksums, source, [manual](USER_MANUAL.md) and [installation/recovery guide](../QUICKSTART.md). Keep original firmware available for recovery.

## Known limitations

Physical FM-1 validation remains pending. The threshold limits master tuning offsets; note quantization, chord generation, oscillator detune, pitch bend and the global A4 reference remain independent. Older firmware does not understand the new boundary. Eight voices remain shared across four parts. USB pedal use requires powered host hardware and physical validation. USER WT remains silent until populated; Vital imports remain HYBRID approximations.
