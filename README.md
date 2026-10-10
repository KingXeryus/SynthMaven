<p align="center"><img src="assets/logo/clipper-logo.png" alt="Clipper" width="460"></p>

<p align="center"><b>A deep synthesizer firmware for the M-VAVE FM-1.</b><br>
Free and open source (GPL-3.0). It grew out of <a href="https://github.com/isod89/sloop-fm1">SLOOP</a>, and like SLOOP it is based on <a href="https://github.com/hugelton/Felucca">Felucca</a> by Leo Kuroshita / Hügelton Instruments.</p>

<p align="center"><a href="DEMARRAGE-RAPIDE-FR.md">Démarrage rapide (français)</a> · <a href="BUILDING.md">Building</a> · <a href="web/EDITOR_PROTOCOL.md">Editor protocol</a> · <a href="docs/sloop/">SLOOP's manuals</a></p>

---

> **Status: 0.3, test build.** Install it at your own risk, and keep a backup made with the editor. You can go back to SLOOP or to the official V15 from the installer page.

SLOOP is a groovebox. **Clipper is a synthesizer** with 17 engines and 308 sounds, one instrument like the original FM-1 firmware, Felucca and Baudgirl's FM-1+VA. It has no tracks, no drum machine, no song mode and no built-in samples. It keeps SLOOP's 12 sound engines, effects, arpeggiator, step sequencer, USB audio and MIDI, and adds **HYBRID**: one voice built from three oscillators, two filters and a modulation matrix, with the whole patch editable in the browser.

## New in 0.3: playing it like a big synth

- **Master EQ and compressor**: FX → **EQ+COMP**: LOW (a shelf at 150 Hz), MID (a peak at 1 kHz), HIGH (a shelf at 5 kHz), each ±12 dB (MID ±10) in 1 dB steps, and **COMP**, a stereo bus compressor (the "glue": one knob lowers the threshold from -6 to -30 dB and raises the ratio from 2:1 to 4:1, with automatic make-up gain). Flat and at 0 they are bypassed, bit for bit; every change glides, no click.
- **Layer and split**: EDIT → **LAYER**: MODE (OFF / LAYER / SPLIT), SOUND (any of the 308 sounds or your presets as the second sound), SPLIT (the split note: the layer below, the main sound from it up), LEVEL; **LAYER 2**: OCT (the layer -2..+2 octaves), PAN, FILT and **SWAP** (the two sounds trade places: every page then edits the other one; SWAP again to go back). Keys, MIDI, the sequencer and the arpeggiator all play both; a note always ends where it started.
- **Poly unison on every engine**: EDIT → VOICE: **UNI2** and **UNI4** stack 2 or 4 detuned voices on each note (DTUNE on VOICE 2), in POLY. (UNI is still the 8-voice mono stack.)
- **Morph and randomizer**: EDIT → **MORPH**: SET A, SET B, then MORPH moves the whole sound between them (every continuous value; the switches change half way; MIDI CC 20). EDIT → **RANDOM**: AMT, WHAT (EDIT: the engine's 8 values, +ENV: and the envelopes, ALL: and the LFO and the sends), RAND, and BACK to undo.
- **Modulation**: LFO → **MOD**: VEL>F (velocity to the cutoff, ±), MW> and AT> (the mod wheel and aftertouch, channel or polyphonic) to VIB (a 5.5 Hz vibrato), CUT, SHP (the engine's shape), WAH (the sound's LFO on the cutoff) or TREM. Every engine (HYBRID keeps its own matrix).
- **308 sounds**: 99 new ones across every engine (Juno, OB, Taurus, Prophet, DX, CZ, SID, B3, bowed and plucked models, 303s, string machines, chords, swarms, speech...), each level-matched through the synth's own output chain.

## New in 0.2: the sound

- **MOOG ladder**: a 24 dB four-pole ladder with zero-delay feedback (after Vadim Zavalishin) and a tanh at its input: it drives warm, its resonance saturates softly, and it self-oscillates on pitch. A filter type in HYBRID; on ANALOG, FX → FILTER → **MODL** chooses SVF (12 dB) or MOOG (24 dB).
- **Soft resonance**: HYBRID's state-variable filters saturate softly instead of clipping.
- **Anti-aliasing**: triangles are band-limited (polyBLAMP), the hard sync gets a BLEP at each reset, and FM, sine feedback and the wavefolder are held below 20 kHz on high notes. Measured by `tests/alias_test.c`: folded partials stay under -40 dB (triangles -62 dB, the folder -66 dB), against -7 dB before for a full fold.
- **Plate reverb** (after Jon Dattorro): FX → REVERB has TYPE (ROOM / PLATE), SIZE, DAMP and PRE (pre-delay, ms). The chorus moved to its own CHORUS page.
- **WAVE, wavetables**: 16 tables of 16 frames (classic shapes, PWM, sync, FM, fold, formants, PPG-style sweeps, organ, chip, comb...), POS morphs through them (the envelope and the LFO move it through ENV DEST / LFO DEST SHP), DTN adds a second oscillator, SUB a sine an octave down, then ANALOG's filter (SVF or MOOG) and drive. The tables are stored as spectra and band-limited per octave in RAM, read with a cubic: aliasing stays below -60 dBFS at 2.4 kHz (`tests/wave_test.c`). Your own tables (a Serum-style WAV) go to a user slot from the editor's Samples tab, WAVETABLE mode; TABL = USR1..USR4 plays them.
- **ACID**: a 303-style bass line engine, one voice. Saw or square (45 %) into the ladder, a one-pole high-pass and DIST. The filter envelope (ENV, DEC), the **accent** (a note at velocity 120 and up, the sequencer's ACCENT steps: louder, the envelope shorter and deeper) and its **sweep** (SWP: a run of accents charges a capacitor that pushes the cutoff further up, the squelch), and **slides**: a SLIDE step, or legato with GLIDE in TIME mode, glides without restarting the envelopes. EDIT: WAVE CUT RES ENV / DEC ACC DIST SWP.
- **STRINGS**: a 1970s string machine (Solina, ARP, Elka). Divide-down registers 16', 8' and 4' locked on one phase, saw towards square (SHP), three ensemble lines moved by a slow sweep (~0.6 Hz) and a fast shimmer (~6 Hz) 120 degrees apart (ENS, RATE), a key-tracked 12 dB low-pass. EDIT: 16' 8' 4' ENS / CUT RATE SHP RES.
- **MACRO**: four macro-oscillator models in the spirit of Mutable Instruments Plaits (the ideas, not the code), on three knobs HARM, TIMB, MORPH: **CHORD** (OCT 5TH SUS4 m m7 m9 m11 6/9 M9 M7 M, inversions, saw to square), **SWARM** (eight oscillators spread up to a semitone, each drifting), **ADDTV** (harmonics 1..16 under a moving bump, odd to even, nothing above 0.45 FS), **SPEECH** (a glottal source through three formants gliding along a word: AI AU OI UA WOW YEAH AEIOU MAMA, held or spoken). Then a low-pass and a **low-pass gate** (DEC, LPG). EDIT: MODL HARM TIMB MRPH / CUT RES DEC LPG.
- **No built-in samples**: 300 KB of flash go to synthesis. SAMPLE and GRAIN play your own samples (the editor's Samples tab, four slots). 209 factory sounds in 0.2 (308 in 0.3): five new ones on the ladder (MOOG BASS, MOOG LEAD, LADDER PAD, MINI BASS, MINI LEAD), sixteen on WAVE, eight on ACID, eight on STRINGS, twelve on MACRO. 17 engines.

## On the FM-1

| | |
| --- | --- |
| **HOME** | The synth screen: the sound's name, its family, its number in the list, the engine and a live scope. The four knobs play the sound's main controls (for HYBRID: CUT, RES, M1, M2). Press HOME again for the full-screen visualiser. |
| **PRESETS** | Every sound, one by one, factory and user presets. |
| **FX pages** | FX (sends), FILTER (the sound's DJ filter and MODL), DLY, REVERB, CHORUS. |
| **ALGORITHM** | Jumps to the next family: PAD, BASS, LEAD, PLCK, KEYS, STAB, FX and the rest. |
| **FX, EDIT, ARP, GLO, SAVE** | A tap opens the pages. While one of them is held, the keys still play, so you can hold EDIT and play. |
| **SEQ, SEL** | Held, they are the two layers a synth needs: SEQ shows the 16 steps on the keys, SEL the key and the chords. A tap opens their pages. |
| **REC** | Records what you play into the pattern. Hold REC to clear the pattern. |
| **MIDI** | Omni: every channel plays the synth. |

A SLOOP project loads with only its first track playing. The four-track groovebox is still in the code: building with `CLIPPER_SYNTH=0` gives it back, and the tests use it.

## HYBRID, the Clipper voice

| | |
| --- | --- |
| **3 oscillators** | Each one is SAW, PULSE (width), TRI, SINE (with feedback), MORPH (sine → tri → saw → square), SUPER (seven detuned saws), FM (phase-modulated by the oscillator before it), SYNC (hard sync to osc 1), FOLD (wavefolder) or NOISE (with colour). Each has coarse ±48 semitones, fine, level and shape, and can restart at each note or keep a fixed pitch. |
| **Mix** | Ring modulation of osc 1 × osc 2, analog drift (per voice and per oscillator), drive. |
| **2 filters** | LP12, LP24, BP, HP, NOTCH, PEAK. They run in series, in parallel (with a balance) or split (osc 1 into F1, osc 2 and 3 into F2). Each has an envelope amount, keyboard tracking and velocity. |
| **Modulation** | Per voice: ENV2 and ENV3 (ADSR) and LFO2 and LFO3 (6 waves, free or key-synced, tempo-synced from 1/32 to 8 bars, fade-in), on top of the sound's amplitude ADSR and LFO1. |
| **Matrix** | 16 slots of *source × via × amount → destination*. Sources: ENV2, ENV3, AENV, LFO1-3, velocity, key, mod wheel, aftertouch (channel or per note), bend, expression, a random value per note, the macros M1-M4. Destinations: pitch (all or per oscillator), shapes, levels, ring, cutoff (both or each), resonances, drive, amplitude, the LFO rates, filter balance. |
| **On the FM-1** | EDIT shows CUT, RES and FENV (offsets on both filters), PTCH (the factory patch), and M1-M4: four macros whose effect each patch decides. |
| **In the editor** | Every value above, live while you play. Patches export and import as `.clipper.json`. |
| **24 factory patches** | CLIPPER, POLY PAD, STRINGS, ACID, SYNC LEAD, FM BELL, FM KEYS, WOBBLE, REESE, HOOVER, BRASS, VOX, PLUCK, RISER, METAL, FOLD BASS, CHIP LEAD, GLASS PAD, DRONE, 808 SUB, TRANCE, SAW PLUCK, SPACE BEL, INIT. They are level-matched with the other 153 sounds and sorted into the PRESETS list by kind. |

## Playing it from a controller (new in Clipper, every engine)

Pitch bend (±2 semitones, or the HYBRID patch's own range up to ±24), mod wheel (CC 1), expression (CC 11), sustain pedal (CC 64), channel and polyphonic aftertouch, and the macros M1-M4 on CC 16-19. SLOOP's CC map (7, 10, 71-75, 91, 93, 94) still works.

## Install and test

On Windows, see **[DEMARRAGE-RAPIDE-FR.md](DEMARRAGE-RAPIDE-FR.md)**: `INSTALL-CLIPPER.bat` builds the firmware and opens the installer at `http://localhost:8767` (SLOOP uses 8766, so the two never get mixed up). Elsewhere, see [BUILDING.md](BUILDING.md).

Projects, user presets, samples and settings stay in the same flash places as SLOOP's. Going from SLOOP to Clipper keeps them; a sound on HYBRID plays another engine if you go back to SLOOP.

## Polyphony

The voices are a build option: 8 (the default, as SLOOP), 12 or 16 (`INSTALL-CLIPPER.bat 12`). The host measurements put a typical HYBRID voice at about 2.5 % of the FM-1's CPU and the heaviest factory patch (CLIPPER, two SUPERs) at about 6.6 %. GLO → SYSTEM → CPU shows the real load on the device: those readings decide the default.

## Tests

`HOST_ONLY=1 tests/run_tests.sh` runs everything that does not need the JieLi toolchain: the synth mode (HOME, presets, keys, pages, one instrument, MIDI omni), the HYBRID tests (the pitch of every oscillator, the filters, the matrix, the controllers, 300 random patches, the cost per voice), the editor's commands, golden renders of every sound, ASan and UBSan, a soak test, and the web pages.

## Credits

- **[Felucca](https://github.com/hugelton/Felucca)** by Leo Kuroshita (@kurogedelic) / Hügelton Instruments: the platform, the engines, the editor and the installer.
- **[SLOOP](https://github.com/isod89/sloop-fm1)** by 3dSam: everything Clipper starts from. Its manuals are in [docs/sloop](docs/sloop/).
- The supersaw follows Adam Szabo's analysis of the JP-8000. The FM6 engine is msfa (Dexed), and PHYS comes from DaisySP and Mutable Instruments Rings: see [LICENSING.md](LICENSING.md).

M-VAVE and FM-1 are trademarks of their respective owners. Clipper is not affiliated with them.
