# SynthMaven 1.3 — Arabic maqam catalogue

Version 1.3 adds **46 maqam settings** (43 named maqamat/aliases plus three
upper-jins/descending variants), for **58 total tuning choices including USER**.
All **23 engines and 537 factory presets** remain.

- Select the maqam with **GLO → TUNING → TUNE** and choose **ROOT C–B**.
- All 12 chromatic keys remain available. Only listed keys get microtuning;
  the root rotates the pattern. Rast C lowers E/B by 50 cents; Bayati D lowers
  E by 50 cents. Chromatic accidentals are played on their normal keys.
- The hardware displays the full name and 12-key cent offsets. The editor has
  the same list, root control and note-offset readout.
- Preserve existing saved tuning IDs, A4 tuning and Scala USER ID 13. Protocol
  17 gates the expanded editor list; older firmware keeps the original list.
- Document equivalent tuning profiles, normalized half-flat tonics, approximate
  intonation and the limits of repeating 12-key tables.

Read the [full catalogue](MAQAM_SCALES.md), [manual](USER_MANUAL.md) or
[quick start](../QUICKSTART.md). Use the
[installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/) and verify
SYNTHMAVEN 1.3 / FM-1_913.

## Validation

Host tests cover all 46 settings × 12 roots × 128 MIDI notes against the
machine-readable reference catalogue, unaltered keys, octave/tonic behavior,
A4 addition, actual pitch lookup, hardware controls, editor wire messages,
old-firmware gating and Scala USER compatibility. The full native target build
must pass before this version replaces the live installer.

## Known Bluetooth crash — experimental release

An FM1 crash into USB rescue was reported during 1.1 Chocolate Plus testing;
the user restored the original firmware. The cause remains unresolved. This
release adds tuning choices and does not establish a fix for the Bluetooth
crash. Physical Bluetooth stability and pedal MIDI remain unverified. See the
[1.2 notes](release-1.2.md) for the prior discovery changes.
