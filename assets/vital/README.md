# Vital-derived factory collection

64 native HYBRID translations were selected from 68 presets supplied by the user. `selected.json` records the source relative path, SHA256, author metadata, compatibility score, waveform projection evidence and conversion warnings. `translated.json` adds packed native parameters and stable preset IDs. Original Vital files and embedded audio are not redistributed.

The selection ranks supported controls, waveform spectral fit and unsupported dependencies. Native waveform fitting is phase-independent and does not reproduce Vital wavetable motion, effects, resynthesis or its full modulation architecture. These sounds are approximations; listening approval and physical-device validation are pending.

Excluded: **Authentic Feel** and **K0T0** ranked below the retained candidates; **Harp** depends on a sample-only voice; **Modern** depends on modulation-of-modulation to create oscillator levels that the translator cannot reproduce.

The first four sounds are STROBE, DARK, REESE and PLUCK. Fresh startup uses them in parts 1–4, part 1 selected, layering off. Existing autosaves take precedence. PRESET browses V01–V64 first within HYBRID and then continues through the existing HYBRID presets. ALGORITHM and SELECT retain their existing roles. Original preset IDs and saved programs remain compatible.

All 556 audible sounds in the expanded 557-entry catalogue pass the reference stereo audit within ±0.5 LU of −29 LUFS; USER WT stays silent. Playing different notes or layering changes loudness.

Reproduce selection with `tools/select_vital_factory.py`, generate native/editor data with `tools/gen_vital_factory.mjs`, then rerun the complete loudness audit. See [import details](../../docs/VITAL_IMPORT.md).
