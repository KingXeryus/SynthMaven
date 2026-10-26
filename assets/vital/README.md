# Vital factory candidates

No Vital presets are bundled yet. This directory reserves the workflow for presets the user will supply.

Stage files through the editor's **Vital preset import**, or use:

```
node tools/convert_vital.mjs supplied-preset.vital > translations.json
```

Unrecognised waveforms are unmapped by default. `--approximate` explicitly permits a native saw substitution. The staging output records source hashes, author metadata, the complete HYBRID patch, amp envelope and compatibility warnings. It is not automatically added to the factory bank.

Before a translation becomes a factory preset, retain its source and redistribution permission, compare it by listening, choose a unique name, validate device behavior and rerun the full stereo loudness/peak audit. User wavetable dependencies must fit the four user slots or be converted into native factory assets deliberately. See [Vital import details](../../docs/VITAL_IMPORT.md).
