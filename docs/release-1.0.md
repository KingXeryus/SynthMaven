# Clipper 1.0 (experimental)

**A free, open-source synthesizer firmware for the M-VAVE FM-1.** It grew out of [SLOOP](https://github.com/isod89/sloop-fm1) and, like SLOOP, it is based on [Felucca](https://github.com/hugelton/Felucca) by Leo Kuroshita / Hügelton Instruments.

> ⚠️ **This is an experimental release.** It passes its whole test suite on a computer, but it has run very little on a real FM-1. Expect rough edges (CPU load, clicks, storage), and there is still a huge amount of work to do. **Make a backup with the editor before installing.** You can always go back to the official V15 firmware from the installer page.

## What it is

- **23 engines, 407 sounds**: HYBRID (three oscillators, two filters, a 16-slot matrix), FMX (six operators after the opsix), FM6 (DX7, loads `.syx` banks), DIGITAL, WAVE (wavetables, and your own), WSEQ (wave sequencing), VECTOR, JUNO (with its chorus), ANALOG (eight filter models), ACID, STRINGS, MACRO, PHASE, VPS, TERRAIN, PHYS (physical models), TRIO, LOFI, WHEEL, VOICE, NOISE, GRAIN and SAMPLE (on Clipper's own synthesized sources).
- **On every engine**: a 12-slot modulation matrix, LFO 2 and 3, ENV 2 (one-shot or looping), the MOD SEQ, four macros (BRITE, TIMBR, MOTN, SPACE, also on CC 16-19), SPREAD (each voice its own place in the stereo field), VINTAGE, an insert effect.
- **Effects**: distortion, chorus, a tempo-synced delay (mono, ping-pong, tape), room, plate, hall and shimmer reverbs, a master EQ, compressor and limiter.
- **Playing**: 8 voices on both cores (12 / 16 as a build option), poly unison, layer / split / multi, a 15-mode arpeggiator with a Turing machine, a step sequencer, MPE, MIDI over USB and TRS, microtuning with Scala scales; the FM-1 is a USB audio interface too.
- **A web editor** (Chrome or Edge, nothing to install) for every parameter, a pro step sequencer (undo, transpose, reverse, Euclidean and random generators, ready-made patterns, velocity and ratchet lanes, a pattern library), the HYBRID and FMX patches, DX7 banks, wavetables, the tuning, and full backups.

## New in 1.0

SPREAD; the matrix grows to 12 slots with LFO 3 and the macros as sources; ENV 2 LOOP; four macros on every engine; HALL and SHIMMER reverbs; the delay's PING and TAPE modes; a crack of the JUNO chorus on loud chords fixed; no samples and no project page on the FM-1 any more (one instrument: the autosave keeps what you play, user presets keep your sounds). The full list is in the [README](https://github.com/isod89/clipper-fm1/blob/main/README.md).

## Install

- **From the browser**: open the [installer page](https://isod89.github.io/clipper-fm1/webapp/installer/) (Chrome or Edge, the FM-1 on USB; the [editor](https://isod89.github.io/clipper-fm1/webapp/editor/) is next to it), make a backup with the editor first, then click Install.
- **From the source, on Windows**: see [QUICKSTART.md](https://github.com/isod89/clipper-fm1/blob/main/QUICKSTART.md) (French: [DEMARRAGE-RAPIDE-FR.md](https://github.com/isod89/clipper-fm1/blob/main/DEMARRAGE-RAPIDE-FR.md)) — `INSTALL-CLIPPER.bat` builds the firmware (WSL and the JieLi toolchain, as SLOOP) and opens the installer.

## What is left to do

Testing on real units (CPU, flash, RAM, clicks, storage across power cuts); room for many more SysEx banks; a sound-design pass so the presets use what 1.0 added; editor pages for the matrix and the banks; a real manual. See [What is left to do](https://github.com/isod89/clipper-fm1/blob/main/README.md#what-is-left-to-do).

**Please report what you find** in the [Issues](https://github.com/isod89/clipper-fm1/issues): bugs, crashes (photograph the FELUCCA CRASH screen), stuck notes, crackles, CPU readings (GLO → SYSTEM).

Clipper is not affiliated with M-VAVE. GPL-3.0.
