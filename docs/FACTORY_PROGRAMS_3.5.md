# SynthMaven factory programs

The factory bank is built by `tools/make_factory_programs.c` with the firmware's own preset and capture code, then packed by `tools/gen_factory_program_pack.py`. Layering starts off, so each program opens on its Layer 1 lead. Hold GLO to bring in the supporting layers.

| Slot | Name | Layer 1 lead | Layer 2 | Layer 3 | Layer 4 |
|---|---|---|---|---|---|
| P1 | DX7 E.PIANO | FM6 E.PIANO 1 | Strings 80S STR | Analog WARM PAD | FM6 DX TINE, up an octave, upper range |
| P2 | DX RHODES | Digital DX RHODES | Juno POLY KEYS | FM6 TINE EP | Hybrid, Vital Rhodex |
| P3 | JUNO 106 | Juno JUNO 106 PAD | Juno JUNO STRINGS | Strings SOLINA | Hybrid XD JUNOPAD, down an octave, lower range |
| P4 | E. ORGAN | FM6 E.ORGAN 1 | FM6 DRAWBARS | Juno JUNO ORGAN | Hybrid V48 ORGAN |
| P5 | DISCO LEAD | Hybrid V38 BRASS | Strings SOLINA | Juno JUNO BRASS | Analog FUNK BASS, lower range |
| P6 | PROPHET LEAD | Prophet Unisawyer | Analog PROPH LEAD | Hybrid, Vital Moog | Prophet Soft Brass, down an octave |
| P7 | RETRO SYNTH | Hybrid XD SYNWAVE | Analog SUPERSAW | Hybrid, Vital 80s Keys | Analog MINI BASS, lower range |
| P8 | ORCHESTRA | FM6 ORCHESTRA | Strings ENSEMBLE | Hybrid V38 BRASS, upper range | Strings LOW STR, lower range |

Layer 1 balance and the supporting levels were set by measuring rendered chords, aiming for supports about 10 to 15 dB under the lead. Every program sends through a slight, slow chorus into a bright reverb. That is an approximation of shimmer: the firmware has no pitch-shifted reverb.

The Vital sources in `assets/factory/vital` are approximate translations made by the editor's importer. Objective checks establish that the programs load, sound and do not clip in host renders. They do not establish how good they sound, which needs listening on an FM-1.

The default voice budget is 64 points. A four-layer program measured a 77% CPU peak on an FM-1 at that budget; the effects chain and voice counts in these programs may differ from that measurement.
