# SynthMaven 2.1.2 — scale-aware chords

- Black keys now play chords; chord mode no longer forces white-key mapping or reserves black keys for modifiers.
- All generated chord tones follow the selected scale and root, including incoming MIDI. Named shapes adapt by scale degree on restricted scales; chromatic mode retains named shapes.
- Maqam harmony follows the catalogue’s documented degree keys and selected tuning root, preserving its chromatic detuning.
- Microtonal tuning offers OFF, OPEN 5 and NEUTRAL; incompatible templates are hidden. OPEN 5 selects existing scale notes near 700/1200 cents; NEUTRAL selects existing scale notes near 350/700 cents. These targets select notes without inventing or retuning scale tones.
- Equal-tempered scales, zero-offset maqams and parts bypassing master tuning retain the full 18-choice list. Legacy chord IDs 0–15 and stored programs remain compatible. A saved incompatible mode plays OPEN 5 without overwriting the stored mode.
- The three-second ENV shortcut, HOME controls and other menus/shortcuts remain intact. Voicing and strum remain available.

## Install and recover

Back up programs/settings first. Use the [live installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/) in Chrome or Edge and keep USB power connected during writing. The release includes firmware, SHA256 checksums, source, [manual](USER_MANUAL.md), and [installation/recovery guide](../QUICKSTART.md). Keep original firmware available for recovery.

## Limitations

Physical FM-1 testing remains pending. Eight voices are shared across four parts. Direct USB MIDI pedals require a powered adapter/hub and hardware validation; Bluetooth is unavailable. Vital imports are native HYBRID approximations; USER WT stays silent until populated. At MIDI range boundaries a chord may contain fewer notes. Neutral target intervals may differ from their targets to stay in the selected scale. See the manual for scale/root mapping and per-part tuning bypass.
