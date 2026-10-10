<p align="center"><img src="assets/logo/clipper-logo.png" alt="Clipper" width="460"></p>

<p align="center"><b>A deep synthesizer firmware for the M-VAVE FM-1.</b><br>
Free and open source (GPL-3.0). It grew out of <a href="https://github.com/isod89/sloop-fm1">SLOOP</a>, and like SLOOP it is based on <a href="https://github.com/hugelton/Felucca">Felucca</a> by Leo Kuroshita / Hügelton Instruments.</p>

<p align="center"><a href="DEMARRAGE-RAPIDE-FR.md">Démarrage rapide (français)</a> · <a href="BUILDING.md">Building</a> · <a href="web/EDITOR_PROTOCOL.md">Editor protocol</a> · <a href="docs/sloop/">SLOOP's manuals</a></p>

---

> **Status: 0.2, test build.** Install it at your own risk, and keep a backup made with the editor. You can go back to SLOOP or to the official V15 from the installer page.

SLOOP is a groovebox. **Clipper is a synthesizer**, one instrument like the original FM-1 firmware, Felucca and Baudgirl's FM-1+VA. It has no tracks, no drum machine, no song mode and no built-in samples. It keeps SLOOP's 12 sound engines, effects, arpeggiator, step sequencer, USB audio and MIDI, and adds **HYBRID**: one voice built from three oscillators, two filters and a modulation matrix, with the whole patch editable in the browser.

## New in 0.2: the sound

- **MOOG ladder**: a 24 dB four-pole ladder with zero-delay feedback (after Vadim Zavalishin) and a tanh at its input: it drives warm, its resonance saturates softly, and it self-oscillates on pitch. A filter type in HYBRID; on ANALOG, FX → FILTER → **MODL** chooses SVF (12 dB) or MOOG (24 dB).
- **Soft resonance**: HYBRID's state-variable filters saturate softly instead of clipping.
- **Anti-aliasing**: triangles are band-limited (polyBLAMP), the hard sync gets a BLEP at each reset, and FM, sine feedback and the wavefolder are held below 20 kHz on high notes. Measured by `tests/alias_test.c`: folded partials stay under -40 dB (triangles -62 dB, the folder -66 dB), against -7 dB before for a full fold.
- **Plate reverb** (after Jon Dattorro): FX → REVERB has TYPE (ROOM / PLATE), SIZE, DAMP and PRE (pre-delay, ms). The chorus moved to its own CHORUS page.
- **No built-in samples**: 300 KB of flash go to synthesis. SAMPLE and GRAIN play your own samples (the editor's Samples tab, four slots). 160 factory sounds.

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
