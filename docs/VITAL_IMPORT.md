# Vital preset import

This is an approximate editor conversion into SynthMaven HYBRID, not a Vital engine port. No Vital factory presets are bundled. Files are read locally, never uploaded. The currently published release is unchanged until the development build is approved for publication.

## Workflow

1. Open **Sound → Vital preset import**. Select up to 64 `.vital` files (16 MB each).
2. Select a preset and review the mapped controls and compatibility warnings.
3. Missing/custom waveforms remain unmapped by default. Enable the clearly labelled saw substitution only when an approximate sound is acceptable.
4. **Save translation to editor library** preserves the complete 128-byte HYBRID patch and its amp envelope. Export the library as a portable backup.
5. Connect in COMPUTER mode and **Send translated sound to selected part** to audition. Opening a file never writes to the device. Sending changes the selected sound and starts with a conservative −12 dB trim; it is not factory level matching.
6. Save a **four-part program** to retain the full custom HYBRID sound on the FM1. The existing **32 user sound-preset slots** store parameter records, not arbitrary HYBRID patch bytes, so importing these sounds into those slots is disabled.

## Conversion scope

| Vital data | Handling |
|---|---|
| Name, author and source version | Read; names shortened to device limits for the patch |
| Three oscillators | Native sine/saw/pulse/triangle identified from compatible stored frames; pitch, fine tuning and level mapped |
| Unknown waveforms | Block sending until explicit saw substitution is enabled |
| Amp envelope / envelopes 2–3 | Quartic Vital times converted approximately to FM1 envelope times; hold/curve behavior differs |
| Two filters | Approximate LP/BP/HP, cutoff/resonance/key tracking; serial routing |
| Modulation | Up to 16 supported linear routes; unsupported sources/destinations and nonlinear routes counted in warnings |
| LFOs | First two free-running sine approximations; custom curves, tempo sync and LFO3/4 are not reproduced |
| Unison, warping, sample oscillator, effects | Reported as unsupported or omitted; not silently claimed as reproduced |
| Master volume, macros, tuning and voice modes | Not imported; controller semantics and DSP differ |

Compatible wavetables must contain a single unmodified Wave Source group, with 2048-point float keyframes. Extracted WAVs contain the **stored keyframes**, not Vital's rendered interpolation/modifier output. Open the WAV in the existing Wavetables editor, where the FM1 uses 16 frames and 128 harmonics in one of four user slots. HYBRID does not read the WAVE user slots; this extraction is a separate WAVE workflow.

A translation should be compared by listening. Spectral patches may be substantially different. Unsupported numeric fields are not treated as a compatibility guarantee; the report describes the supported subset and relevant omissions. `.vitalbank` archives are not supported in this first importer.

## Factory additions

User-supplied Vital presets will be staged separately with their origin and permission to redistribute recorded. Each chosen translation must be reviewed, tested and level-matched before becoming a factory entry. No replacement presets are invented or bundled by this change.

## Format references

- [Vital serialization](https://github.com/mtytel/vital/blob/main/src/common/load_save.cpp)
- [Vital parameter units](https://github.com/mtytel/vital/blob/main/src/common/synth_parameters.cpp)
- [Wave Source float keyframes](https://github.com/mtytel/vital/blob/main/src/common/wavetable/wave_source.cpp)
