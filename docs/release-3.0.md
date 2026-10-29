# SynthMaven 3.0

A modern device interface and an offline iPhone/iPad editor, with USB editing and no Bluetooth radio support. The existing navigation, factory programs and solo loudness are preserved. TRS polarity switching is not included.

## What changed

- Web and iOS editors are dark-only. Quick microtuning puts the borderless wheel left of the keyboard; the full Hybrid-style Tuning page centers it below. Scale search, master/bypass switches, twelve offset sliders and a 24-entry editor scale library provide direct access to tuning. The four hardware custom slots remain separate from the editor library. Hidden keyboards release their layout space. It previews -100 to +100 cents in 1-cent steps on the selected key. Single-tap selects; double-tap sets an untuned note to -50 cents or resets a tuned note to zero. Device custom tuning uses KNOB 3 in 10-cent steps. Exact offsets persist in settings and programs; protocol 26 adds fine-scale read/write while preserving older program imports.
- Lossless factory-program compression saves 7,223 bytes without changing captured sound data or removing engines.

- Perform opens immediately with twelve quick controls, Program/Scene Up/Down and current labels. Layers provides a four-part mixer; Tuning provides a keyboard-based scale view. Edit contains preset loaders and macro assignment/save controls.

- Rounded, high-contrast parameter cards and rotary controls. The tuning keyboard shows normal white/black keys, red adjusted notes, grey excluded notes and a blue played-key marker.
- Global tuning survives program changes unless a program explicitly saves its own override. The FROM octave and existing microtonal/chord rules are retained.
- LFO FILTER brings cutoff, signed depth, rate and waveform together. Holding EDIT for two seconds opens and latches the four-part mixer.
- **SynthMaven FM1** uses the supplied SynthMaven FM1 Home Screen icon and in-app logo. The native iPhone/iPad editor bundles the existing interface offline, with USB Core MIDI, Bluetooth MIDI pairing, Files import and share-sheet export. The source project is included in the source archive; Apple signing is required. Version 3.0 (6) is available to invited internal TestFlight testers; this is not a public App Store release.
- Layered mono note bursts are coalesced for the main part as well as backing parts. With three or four enabled, unmuted parts at nonzero level, each part is limited to two synthesis voices while layered. The overall eight-voice limit still applies. Layer OFF, solo, and one/two-part setups retain the normal engine voice limits. Dense chords therefore steal older notes sooner instead of allowing every layer to grow independently; stored sounds and volume settings are unchanged.

## Installation and recovery

Use the [live USB installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/) in COMPUTER mode. Back up first: every flash resets settings, programs, user presets, samples and autosave, including same-version reinstalls. Keep the cable attached until the device restarts. For interrupted updates, reconnect in recovery mode and retry using the included installation/recovery instructions and illustrated PDF guide.

## Known limitations

Direct Bluetooth is excluded from this release, including radio initialization and the BLE EDITOR menu. Use USB in COMPUTER mode for editing, backup and firmware updates. The iOS app may still offer Bluetooth pairing for other MIDI devices. All 23 engines and 557 factory entries are retained. Heavy custom layers can exceed the shared eight-voice/CPU budget. Direct USB pedals require powered host hardware and compatible class-compliant MIDI. USER WT intentionally remains silent until a user wavetable is uploaded.

## Validation

Release checks cover host regressions, the built update loader, audio renders, responsive editor layouts and the illustrated guide. The dual-core radio experiment failed and is deliberately excluded from the release. The USB-only build passed the update-loader reset tests and installed on the FM-1. All eight factory programs ran on both cores with zero missed audio deadlines in the single-note and dense-chord test: maximum average CPU 71.09%, recorded peak 89%. Host audio audits passed for 556 audible factory sounds, the silent USER WT slot, and 136 program/solo/scene renders. These results describe the tested performances, not every possible custom patch. See validation-3.0.json for the hardware measurements.
