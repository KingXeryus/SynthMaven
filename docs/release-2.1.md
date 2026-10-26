# SynthMaven 2.1

A device GUI refresh and expanded chord performance tools, retaining the HOME navigation, menu/page structure, shortcuts, four-part programs and tuning behavior. Version 2.1 uses protocol 22 and the same shared eight-voice budget.

## Device visuals

Rubik Medium typography, rounded parameter and part cards, clearer value readouts, a chord-name/keyboard display and a chromatic tuning visualization take visual cues from [Melodee](https://github.com/keremimo/melodee). SynthMaven keeps its own branding and the requested black/white/red identity, with blue and grey secondary colors. No orange/yellow palette is used. The COLOR menu retains five choices and its stored selection IDs; their new display palettes are RED, WHITE, BLUE, COOL and GREY.

The twelve tuning cells remain the twelve chromatic note names. Detuning is visualized without replacing SynthMaven's maqam/chromatic mapping or changing the Scala workflow. The selected note's cent offset is shown. SEL still opens the existing scale browser; SELECT still selects the scale and PRESET its root.

## Chords through the existing SEL controls

The existing six modes retain their saved IDs and behavior: OFF, TRIAD, 7TH, 9TH, SUS4 and POWER. The list adds MAJ, MIN, DOM7, MAJ7, MIN7, SUS2, DIM, AUG, AUTO and AUTO2. No new page or shortcut is required. Choose CHORD on SEL, and VOIC/STRUM on SEL 2. The previous VLEAD OFF/ON settings are represented by CLOSE/SMOOTH, with the same stored IDs; OPEN, INV1, INV2 and +OCT are appended.

- **AUTO** selects an in-scale chord, using previous harmony and nearby inversions. Independent non-program tracks can provide melodic context; four-part programs share their input and do not feed generated harmony back into itself. A two-beat pause, STOP, panic or a scale/root/transpose/voicing change starts a new phrase.
- **AUTO2** harmonizes below the mapped melody and keeps that melody as the highest note. It is deterministic rather than dependent on previously played chords. At the bottom of the MIDI range, unavailable lower accompaniment is omitted.
- **Fixed shapes** create the named semitone chord; existing scale/keyboard mapping happens first. Existing CHORD+ keyboard modifiers remain available. AUTO modes select their own harmony.
- **Voicings** provide close, smooth voice leading, open, first/second inversion or a lower octave root. AUTO2 keeps its melody fixed at the top.
- **Strum** retains signed millisecond values −60…+60. Additional values 61–69 are D8/U8/A8, D16/U16/A16 and D32/U32/A32: down/up/alternating eighth-, sixteenth- and thirty-second-note spacing at the current tempo. AUTO2 sounds the melody immediately, then strums the accompaniment.

Chords work with the device keys and incoming USB/TRS MIDI. Generated MIDI notes are captured at note-on; changing settings while holding a note does not change which notes its release ends. Overlapping generated MIDI tones retain ownership until their last source releases. The bounded ownership table supports 128 simultaneous input chord sources. Live recordings retain the played notes. Existing manually recorded sequence notes are not reharmonized.

AUTO/AUTO2 play the input note alone when master microtuning applies to the sound. They do not replace the maqam/Scala implementation. Parts bypassing master tuning can use automatic harmony in equal temperament. Mono, legato, single-note unison modes and kits retain single-note/kit behavior for the new modes. The legacy chord modes retain their existing behavior.

## Factory sounds and compatibility

Includes the 64 Vital-derived native HYBRID translations and consolidated 44-sound regional collection: **557 entries, comprising 556 level-matched audible sounds and the intentionally silent USER WT slot**. Fresh startup uses STROBE, DARK, REESE and PLUCK with layering off; saved working programs take precedence. ALGORITHM selects engines, PRESET cycles sounds within the engine, and SELECT chooses the program part. HYBRID visits the 64 Vital entries before its older sounds. Preset IDs and stored program/backup formats are retained.

## Installation and recovery

Back up programs and settings through the editor first. Use the current [web installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/) in Chrome or Edge with a USB data cable, and keep power connected until completion. For recovery and returning to stock firmware, follow the [quick-start guide](https://kingxeryus.github.io/SynthMaven/docs/quick-start.html) and [manual](https://kingxeryus.github.io/SynthMaven/docs/user-manual.html). Do not use a package for a different hardware identity.

## Validation and limits

Automated checks cover chord ranges, all 16 Western scales and 12 roots, voicings, MIDI ownership, preserved navigation/shortcuts, font decompression and clipped rendering, program storage, USB routing and editor compatibility. The factory reference audit and actual FM1 target build must pass before deployment.

Physical FM1 validation and listening comparison of Vital translations with the originals remain pending. Native translations approximate supported parameters rather than running Vital DSP. Eight voices are shared across the entire four-part program: layering a four-note chord across four parts can demand more voices than are available and cause voice stealing. Direct USB host pedals still require suitable powered hardware and physical-device validation. Bluetooth is removed.

Melodee-derived AUTO/AUTO2 algorithms retain Kerem Kilic/Ellic Studio copyright under GPL-3.0-only. Rubik is distributed under SIL OFL 1.1; see [licensing](https://github.com/KingXeryus/SynthMaven/blob/release-2.1/LICENSING.md).
