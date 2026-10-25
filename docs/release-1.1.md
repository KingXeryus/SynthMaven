> Inherited implementation notes; this is not a SynthMaven release announcement.

# SynthMaven 1.1 (experimental)

**A free, open-source synthesizer firmware for the M-VAVE FM-1.** It grew out of [SLOOP](https://github.com/isod89/sloop-fm1) and, like SLOOP, it is based on [Felucca](https://github.com/hugelton/Felucca) by Leo Kuroshita / Hügelton Instruments.

> ⚠️ **This is an experimental release.** It passes its whole test suite on a computer, but it has run very little on a real FM-1. Expect rough edges (CPU load, clicks, storage), and there is still a huge amount of work to do. **Make a backup with the editor before installing.** You can always go back to the official V15 firmware from the installer page.

## What it is

- **23 engines, 407 sounds**: HYBRID (three oscillators, two filters, a 16-slot matrix), FMX (six operators after the opsix), FM6 (DX7, loads `.syx` banks), DIGITAL, WAVE (wavetables, and your own), WSEQ (wave sequencing), VECTOR, JUNO (with its chorus), ANALOG (eight filter models), ACID, STRINGS, MACRO, PHASE, VPS, TERRAIN, PHYS (physical models), TRIO, LOFI, WHEEL, VOICE, NOISE, GRAIN and SAMPLE (on SynthMaven's own synthesized sources).
- **On every engine**: a 12-slot modulation matrix, LFO 2 and 3, ENV 2 (one-shot or looping), the MOD SEQ, four macros (BRITE, TIMBR, MOTN, SPACE, also on CC 16-19), SPREAD (each voice its own place in the stereo field), VINTAGE, an insert effect.
- **Effects**: distortion, chorus, a tempo-synced delay (mono, ping-pong, tape), room, plate, hall and shimmer reverbs, a master EQ, compressor and limiter.
- **Playing**: 8 voices on both cores (12 / 16 as a build option), poly unison, layer / split / multi, a 15-mode arpeggiator with a Turing machine, a step sequencer, MPE, MIDI over USB and TRS (omni or one channel, program changes, expression, panic, the bend range, local control, CC out, a breath controller), microtuning with Scala scales; the FM-1 is a USB audio interface too.
- **A web editor** (Chrome or Edge, nothing to install) for every parameter, the HYBRID and FMX patches, DX7 banks, wavetables, the tuning, and full backups.

## New in 1.1

- **The crackle on the left channel with the reverb on is fixed.** PLATE, HALL and SHIMMER read one of their left output taps outside its delay line: a click every 60 ms, on the left only. ROOM overflowed into a crackle with DAMP turned down on a loud sound: fixed too. Every effect is now checked by a test that sends a sine through it at its extremes.
- **Made for MIDI controllers and DAWs**: program changes and bank select load the sounds; CC 11 expression; CC 120 / 123 (panic), CC 121 (reset), CC 122 (local control); the pitch bend range (GLO > MIDI > BEND, and RPN 0); LOCAL OFF for a DAW that sends the track back; the knobs can send their CCs (MIDI 2 > SEND CC) so a DAW records the moves; a **breath controller** (EWI, CC 2) plays the volume and the aftertouch (MIDI 2 > EWI); the MIDI identity request is answered. The whole list: [docs/midi.md](https://github.com/KingXeryus/SynthMaven/blob/main/docs/midi.md).
- **A MIDI channel for the synth**: GLO > MIDI > IN CH (OMNI or 1 to 16) and OUT.
- **The sequencer of SLOOP 2.7.2 on the FM-1**: the NOTES piano roll and its note sheet, the keys or a MIDI keyboard writing the step, EDIT / ARP / GLO held as layers (erase, shift, length, transpose, note repeat, fills, tap tempo), legato recording, a steady MIDI clock.
- **A pro sequencer in the web editor**: undo / redo, ranges, transpose, reverse, chords, Euclidean and random generators, ready-made patterns, velocity / nudge / ratchet / level lanes, a pattern library.
- **The PRESETS page filters by engine** (KNOB 2), **the factory sounds load dry** (no delay, no reverb: the SPACE macro or FX add them), **A4 in Hz** (400 to 480), and the knobs no longer miss steps when turned fast.

The full list is in the [README](https://github.com/KingXeryus/SynthMaven/blob/main/README.md).

## Install

- **From the browser**: open the [installer page](https://kingxeryus.github.io/SynthMaven/webapp/installer/) (Chrome or Edge, the FM-1 on USB; the [editor](https://kingxeryus.github.io/SynthMaven/webapp/editor/) is next to it), make a backup with the editor first, then click Install.
- **From the source, on Windows**: see [QUICKSTART.md](https://github.com/KingXeryus/SynthMaven/blob/main/QUICKSTART.md) (French: [DEMARRAGE-RAPIDE-FR.md](https://github.com/KingXeryus/SynthMaven/blob/main/DEMARRAGE-RAPIDE-FR.md)) — `INSTALL-SYNTHMAVEN.bat` builds the firmware (WSL and the JieLi toolchain, as SLOOP) and opens the installer.

## What is left to do

Testing on real units (CPU, flash, RAM, clicks, storage across power cuts); room for many more SysEx banks; a sound-design pass so the presets use what 1.0 and 1.1 added; editor pages for the matrix and the banks; a real manual. See [What is left to do](https://github.com/KingXeryus/SynthMaven/blob/main/README.md#what-is-left-to-do).

**Please report what you find** in the [Issues](https://github.com/KingXeryus/SynthMaven/issues): bugs, crashes (photograph the FELUCCA CRASH screen), stuck notes, crackles, CPU readings (GLO → SYSTEM).

SynthMaven is not affiliated with M-VAVE. GPL-3.0.
