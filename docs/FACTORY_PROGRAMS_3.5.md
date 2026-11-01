# SynthMaven factory programs

P1-P3 and P5-P8 are the programs saved on the project's FM-1, read over USB on 2026-11-01 (`assets/factory/device-programs-2026-11-01.bin`, spliced in by `tools/apply_device_programs.py`), so the table's rows for them are their starting points, not their exact contents. P2, P3, P7 and P8 load layered. P4 was never saved on that FM-1 and is unchanged.

The factory bank is built by `tools/make_factory_programs.c` with the firmware's own preset and capture code, then packed by `tools/gen_factory_program_pack.py`. Layering starts off on P1, P4, P5 and P6, so each opens on its Layer 1 lead. Hold GLO to bring in the supporting layers.

| Slot | Name | Layer 1 lead | Layer 2 | Layer 3 | Layer 4 |
|---|---|---|---|---|---|
| P1 | DX7 E.PIANO | FM6 E.PIANO 1 | Strings 80S STR | Analog WARM PAD | FM6 DX TINE, up an octave, upper range |
| P2 | DX RHODES (layered) | Digital DX RHODES | Juno POLY KEYS | FM6 TINE EP | Hybrid, Vital Rhodex |
| P3 | JUNO 106 | Juno JUNO 106 PAD | Juno JUNO STRINGS | Strings SOLINA | Hybrid XD JUNOPAD, down an octave, lower range |
| P4 | E. ORGAN | FM6 E.ORGAN 1 | FM6 DRAWBARS | Juno JUNO ORGAN | Hybrid V48 ORGAN |
| P5 | DISCO LEAD | Hybrid V38 BRASS | Strings SOLINA | Juno JUNO BRASS | Analog FUNK BASS, lower range |
| P6 | PROPHET LEAD | Prophet Unisawyer | Analog PROPH LEAD | Hybrid, Vital Moog | Prophet Soft Brass, down an octave |
| P7 | RETRO SYNTH | Hybrid XD STRINGS | Analog SUPERSAW | Hybrid, Vital 80s Keys | Analog MINI BASS, lower range |
| P8 | ORCHESTRA (layered) | FM6 STRINGS 2 | Strings ENSEMBLE | Hybrid V38 BRASS, upper range | Strings LOW STR, lower range |

Layer 1 balance and the supporting levels were set by measuring rendered chords, aiming for supports about 10 to 15 dB under the lead. Every program sends through a slight, slow chorus into a bright reverb. That is an approximation of shimmer: the firmware has no pitch-shifted reverb.

The Vital sources in `assets/factory/vital` are approximate translations made by the editor's importer. Objective checks establish that the programs load, sound and do not clip in host renders. They do not establish how good they sound, which needs listening on an FM-1.

Every factory sound and program is level-matched to P1 by programmatic loudness compensation: `tests/loudness_measure.c` measures each one (a K-style weighting, the loudest 400 ms of a held note at velocity 100), `tools/gen_loudness_comp.py` writes `firmware/src/loudness_comp.h`, and `PROGRAM_LOUDNESS` in `tools/make_factory_programs.c` tunes the programs. After adding or changing factory sounds, rerun the measure and generate steps until they settle. `tests/run_tests.sh` fails if P4 (the one program not saved from the FM-1) drifts more than 1.5 dB, under 90 % of sounds are within 3 dB, or a sound reaches the rail.

While layering is on the voice budget is capped at 16 points and a chord starts one key per audio block. On an FM-1 with all four layers a chord at the former 64 points peaked at up to 257% of the audio deadline; measure with `tools/synthmaven_layer_load.py`.
