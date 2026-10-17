<p align="center"><img src="assets/logo/clipper-logo.png" alt="Clipper" width="460"></p>

<p align="center"><b>A deep synthesizer firmware for the M-VAVE FM-1.</b><br>
Free and open source (GPL-3.0). It grew out of <a href="https://github.com/isod89/sloop-fm1">SLOOP</a>, and like SLOOP it is based on <a href="https://github.com/hugelton/Felucca">Felucca</a> by Leo Kuroshita / Hügelton Instruments.</p>

<p align="center"><a href="DEMARRAGE-RAPIDE-FR.md">Démarrage rapide (français)</a> · <a href="BUILDING.md">Building</a> · <a href="web/EDITOR_PROTOCOL.md">Editor protocol</a> · <a href="docs/sloop/">SLOOP's manuals</a></p>

---

> **Status: 0.8, test build.** Install it at your own risk, and keep a backup made with the editor. You can go back to SLOOP or to the official V15 from the installer page.

SLOOP is a groovebox. **Clipper is a synthesizer** with 23 engines and 392 sounds, one instrument (two in MULTI) like the original FM-1 firmware, Felucca and Baudgirl's FM-1+VA. It has no tracks, no drum machine, no song mode and no built-in samples. It keeps SLOOP's 12 sound engines, effects, arpeggiator, step sequencer, USB audio and MIDI, and adds **HYBRID**: one voice built from three oscillators, two filters and a modulation matrix, with the whole patch editable in the browser.

## New in 0.8: JUNO and its chorus, VECTOR, a modulation sequencer, SPICE / DICE

- **JUNO**, the poly synth of the Roland Juno-6 / 60 / 106 (the idea, none of Roland's code): one DCO per voice with a **saw**, a **pulse** whose width the LFO sweeps (PWM: LFO DEST SHP, the wheel, the matrix's SHAPE) or both together (**S+P**, the 106's two buttons), a **sub** square an octave down and **noise**; the **HPF** in the 106's four positions (0 a little bass boost, 1 flat, 2 and 3 thinner), the **24 dB resonant ladder** (keyboard tracking, its envelope), and the **chorus**: **I** (0.5 Hz, wide), **II** (0.8 Hz, deeper), **I+II** (8 Hz, shallow, the 60's two buttons together): a stereo BBD chorus, two taps of one line swept in opposite directions, darkened like the bucket brigade, left = dry + one, right = dry + the other. A mode changed under a held chord fades (no click). EDIT: WAVE PW SUB NOIS / HPF CUT RES CHOR. 16 sounds (pads, brass, strings, bass, keys, synthwave lead, a hoover).
- **The JUNO chorus on any sound**: the INSERT has three more types, **JUNO1**, **JUNO2**, **JUN12** (A WIDTH, B TONE, C NOISE: the BBD's hiss, MIX the wet).
- **VECTOR**, vector synthesis after the Sequential Prophet VS and the Wavestation's joystick: four waves at the corners of a square (**A B C D**, 16 of Clipper's own: SINE TRI SAW SQR PLS25 PLS10 ORGAN VOX A VOX O BELL HOLLW BRASS RESO DIGI STRNG METAL, each in three band-limited copies), a point in it (**X**, **Y**) mixes them, and **MOVE** sends the point round a path by itself from each note's start: CIRC, 8, SWEEP (from A to the point: the VS's mix envelope), LOOP (A B D C), RND, each slow or fast (+). SHAPE moves X; the matrix moves X and Y by name. Then a 12 dB low-pass. EDIT: A B C D / X Y MOVE CUT. 16 sounds.
- **MOD SEQ**, a modulation sequencer in every sound (LFO → **MOD SEQ**, **MOD SEQ 2**): four lanes of 16 steps (-8..7) that step at **RATE** (1/32 .. 2 bars, the song's tempo), over **LEN** steps, each lane gliding (**SMTH**), from each first key (**START KEY**) or on the song's grid (**SYNC**). The matrix reads them as four more sources, **SEQ1..SEQ4**: a lane to the cutoff, another to the pitch, a rhythm to the level, any EDIT value by name. On the FM-1: LANE STEP VALUE and **SHAPE** (RAMP+ RAMP- TRI SINE SQR RND FLAT INVRT SHIFT: scroll it to audition), the lane drawn big, the step playing lit, the four lanes small below. A sound loaded brings four shapes to start from (a ramp, a triangle, a rhythm, a random line). Kept with the sound: UNDO, COMPARE, the layer's SWAP, user presets, projects.
- **SPICE / DICE** (ARP → **SPICE**, after the Arturia MicroFreak): the arpeggiator's notes and the sequencer's steps vary as much as SPICE says: some rest, some jump an octave, the velocities and the gates move; the same way each time round (16 notes, or the pattern's steps), until **DICE** is turned (another variation). What is recorded is never changed. SPICE 0: as before, note for note.
- **The web editor** (protocol 13): a **MOD SEQ** card (the four lanes as bars to drag, the shapes, LIVE), the new pages (MOD SEQ 2, SPICE, the INSERT's JUNO types), JUNO and VECTOR.
- **Both new engines run on both cores** (bit for bit, as the others). **392 sounds, 23 engines.**
- **Fixed**: a user preset sent from the web editor was written in SLOOP's layout over Clipper's record (its LFO 2, ENV 2, matrix and INSERT lost, and a few bytes past the record).

## New in 0.7: both cores, a history of the sound, and SLOOP 2.7's best

- **The FM-1's second processor core renders half the voices.** After SLOOP 2.7 (the second core's start-up is [X0X](https://github.com/charlesvestal/fm1-x0x)'s, Charles Vestal, after [Melodee](https://github.com/keremimo/melodee); the HAL came to SLOOP through Andre Rezemini's dualcore branch, 2.7.1's safe hold included). SLOOP splits by track; Clipper plays one sound, so it splits **a sound's voices**: this core works out every voice's modulation, hands the second half of the voices to core 1, renders the first half, waits, adds. Bit for bit the same sound whoever renders what (the host tests prove it for every preset, one core or two, a slow second core, a lost job, under ThreadSanitizer). Engines that may split: HYBRID, FMX, WAVE, ANALOG, FM6, VPS, TERRAIN, WSEQ, MACRO, STRINGS, TRIO, PHASE, DIGITAL, LOFI (0.8: and ACID, JUNO, VECTOR; the others stay on one core). MENU → SYS → **2ND CORE** ON / OFF; ABOUT says **2 CORES** or **1 CORE**. This is what should make 12 or 16 voices possible: measure with GLO → SYSTEM (CPU, LATE, PEAK).
- **UNDO and COMPARE on the FM-1** (EDIT → **UNDO**): every change of the sound becomes a step once it rests (a knob sweep is one step), a sound loaded too; KNOB 1 goes back (left) and forward (right) through up to 16 steps, the page lists them ("-1 CUT RES", "-2 MOOG BASS"); FMX's patch goes with them. KNOB 2 **CMP**: ORIG plays the sound as it was loaded, EDIT your edits again. HOME says **EDITED** when the sound differs from what was loaded, and loading another sound never loses your edits: UNDO brings them back.
- **RANDOM by section**: WHAT now also offers **ENV**, **MOD** (LFOs, matrix amounts), **FX** (and the INSERT's values) and **OPS**: on FMX, the operators themselves (musical ratios, the modulators' levels, envelopes, now and then a wave, the routing's amounts), as the opsix's randomizer.
- **Favourites**: PRESETS page, KNOB 4 right puts the sound at the top of the list (up to 16), left takes it out; kept with the FM-1's settings.
- **SLOOP 2.7's gifts**: **KBD HOLD** (ARP 2 → HOLD ON with the arp OFF: the notes go on sounding, the next chord replaces them; HOME shows HOLD), **15 arp modes** (REPEAT, DNUP, UP+8, CONV, DIVG, PINKY, THUMB, WALK, CHORD after UP DN UPDN RND ORD), **ARP SWG** works stopped too (#52), a sound loaded while the arp runs keeps the arp and its notes, **CC 74** sweeps the engine's own cutoff or brightness, **REC QUANT OFF** (KNOB 4 of the REC screen: your timing kept as nudges), overdub over held notes, **knob acceleration** that grows with the speed of the turn (MENU → SCREEN → KNOB ACCEL), **LEVEL and PAN glide** (no zipper), user samples that **loop while held** (LOOP, LOOP+TAIL), **USB RATE** (AUTO, 44.1, 48 kHz: MENU → AUDIO), **MIDI CLOCK OUT** on USB (MENU → MIDI), the crash screen with the stack, 10 s.
- **The web editor**: a **Sound tools** card: **Undo / Redo** (Ctrl+Z, Ctrl+Shift+Z), **Compare with loaded**, **Store A / Store B / Toggle A / B**, a **randomizer by section** with an amount, **what changed since loaded** (a table), and the **envelope drawn** (ENV, and ENV 2 dashed). The Samples page sets each file's **loop** (ONE SHOT, LOOP, LOOP+TAIL with its points on zero crossings, a WAV's own loop read from its smpl chunk). The editor finds the FM-1 under any of its port names (a port kept from another firmware, e.g. "X0X FM-1").

## New in 0.6: two parts, an insert per sound, FMX and wave sequencing

- **MULTI (bi-timbral)**: EDIT → LAYER → MODE has a fourth choice, **MULTI**: the layer's sound becomes a part of its own, on its own MIDI channel (**CH**, on the knob SPLIT has in the other modes; 2 by default), with its own pitch bend, mod wheel, pressure and sustain pedal; every other channel, the keys, the sequencer and the arpeggiator play the main sound. The voices are shared (8, or 12 / 16 with `INSTALL-CLIPPER.bat 12` / `16`). LAYER and SPLIT as before.
- **An INSERT effect per sound** (FX → INSERT, INSERT 2), SLOOP 2.6's: TYPE OFF, SOFT, HARD, FOLD, FUZZ (DRIVE TONE LEVEL), CRUSH (BITS RATE LPF), PHASR, FLANG, CHOR (RATE DEPTH FDBK), and MIX. After the sound's drive, before its filter and the sends; the main sound and the layer each their own; kept with the sound (presets, user presets, projects). OFF or MIX 0: the sound bit for bit as before. A SLOOP 2.6 sound or project brings its INSERT along.
- **FMX**, a six-operator engine after the Korg opsix: each operator its own **wave** (SINE TRI SAW SQR HALF ABS QUART PULSE SIN8 NOISE, the bright ones band-limited) and its own **mode**: **PM** (the DX7's "FM"), **FM** (true, linear, through-zero frequency modulation) or **RING**; a ratio (x0.5 .. x31, FINE) or a fixed frequency; level, ADSR, velocity, key scaling, detune. **Any routing**: any operator into any other, each connection its own amount, an operator into itself its feedback, any operator to the output. 40 templates (ALG: the DX7's 32, then CASCADE, 2 STACKS, STAR, 2 x 2>1, RING 3, LOOP, Y, 1 > 5) to start from; change any connection after. A pitch envelope (PEG). EDIT 1 / 2: PTCH ALG MOD FB / TIME DTN VEL KTRK (macros on the patch); on FMX six more EDIT pages edit the patch itself: **OP** (OP WAVE MODE RATIO), **OP 2** (FINE LEVEL VEL KSC), **OP ENV**, **ROUTE** (TO FROM AMT OUT, with the routing drawn as a grid), **FMX** (DTN FIX PEG PTIM) and **STORE** (into the bank, B1..B27, the song stopped). The bank is FM6's: a slot holds an FMX patch or a DX7 voice, and FMX plays a DX7 voice converted. In the editor: the whole patch, the routing drawn, templates, LIVE, STORE, files (`.clipper-fmx.json`). 16 sounds. A voice costs about what a HYBRID voice does.
- **WSEQ**, wave sequencing after the Korg Wavestation: each note plays a sequence of steps, each its own wave, pitch and level, at the song's tempo, the next one fading in over the end of the one before. SEQ (16 sequences: STEPS RHYTHM SWEEP FIFTHS MINOR MAJOR OCTAVE GATE BELLS 8BIT MORPH VOX NOISY DUB DORIAN PING), RATE (1/32 .. a bar), XFADE, LOOP (FWD BACK PING ONCE), START, PITCH (how wide the steps' intervals: 0 none, 127 twice), CUT, RES. 16 sounds; light (a third of HYBRID).
- **360 sounds, 21 engines.** The web editor knows the new pages (INSERT, and FMX's patch).

## New in 0.5.1: an audio timing readout (the crackle hunt)

GLO > SYSTEM, the CPU column now turns every 1.5 s: **CPU** (or the USB status), **LATE** (audio buffers rendered too late since power-on: each one a click) and **PEAK** (the slowest buffer of the last few seconds, % of the time it has; over 100 means late). If you hear crackles, note whether LATE climbs while they happen.

## New in 0.5: two more engines, VPS and TERRAIN

Both suggested in the Felucca discussions ([#145](https://github.com/hugelton/Felucca/discussions/145), [#146](https://github.com/hugelton/Felucca/discussions/146)) as light ways to sounds no oscillator makes; both cost about what ANALOG costs (a third of HYBRID).

- **VPS**, vector phaseshaping (after Kleimola, Lazzarini, Timoney and Välimäki, DAFx 2011): a sine read through a bent phase. The bend is one point, **D** (where in the period) and **V** (how many cycles by then). V under 1: phase-distorted waves, CZ style; V above 1: V cycles squeezed into the first part of the period, a formant at V × the note, as a hard sync or a vowel sweep. Whole cycles only (V between two integers crossfades them): no spike, no aliasing. **VENV** sweeps V with the envelope at each note, **OSC2** adds a second one with D mirrored and detuned (**DTN**), **SUB** a sine an octave down, then CUT and RES. SHAPE modulation (LFO, envelope, wheel, matrix) moves D. EDIT: D V VENV OSC2 / CUT RES DTN SUB. 10 sounds.
- **TERRAIN**, wave terrain synthesis, lite: an orbit circles over a surface and the surface's height under it is the sound. One turn is one period, so it is pitched; where the orbit lies (**X**, **Y**) and how wide it is (**RAD**) choose the harmonics; **ORB** squeezes it flat or turns it into a figure eight; **DRIFT** moves it slowly, each note its own way, for timbres that keep evolving. Eight surfaces of 32 × 32 (**TERR**: WAVES, RIPPLE, CRYSTAL, BUMPS, SADDLE, FOLDS, TWIST, CHAOS), periodic in both directions and band-limited; the orbit narrows on high notes. EDIT: TERR RAD X Y / ORB DRIFT CUT RES. 10 sounds.
- **328 sounds, 19 engines.** Every EDIT value of both is a matrix destination (X and Y moved by LFO 2 or the keys, say).

## New in 0.4.1: SLOOP 2.6's fixes, and living next to SLOOP 2.6

- **A cleaner output**: the master limiter looks one block (0.7 ms) ahead, so a loud attack is already turned down when it comes instead of its first samples going through the output's soft clipper (a crackle with MASTER up, in headphones most); a held loud bass keeps one gain (no whistle). After SLOOP 2.6.
- **No click when a filter starts to close**: the sound's FILTER and the master FILT start their low-pass where the signal is (it started from silence under the sound: a crack).
- **FM6**: a sound changed under held notes fades them out with their own patch; the new patch comes after (they took it at once: a click).
- **Arpeggiator** (#191, after Felucca 1.5): a latched arp follows TRN, ROOT, SCALE and QNT changed after its keys were pressed.
- **A project load or NEW** puts the pitch wheel back to the middle, the pressure to 0 and the sustain pedal up (its notes let go); MPE's per-note bends too.
- **Web editor**: a value set again (the same value on another sound, or after a knob moved it on the FM-1) is sent again (it was skipped for good); a sound file from another version loads by layout; a backup with projects newer than the firmware reads stops before writing anything.
- **Installer**: an FM-1 running SLOOP is found (going from SLOOP to CLIPPER no longer stops at *FM-1 not found*); when none is, the message lists the MIDI ports it saw.
- **Going between SLOOP 2.6 and CLIPPER on the same FM-1**: CLIPPER reads SLOOP 2.6's projects (they were shown as empty slots, and saving there wrote over them); it keeps SLOOP 2.6's MIDI settings and MIDI LEARN in the settings, and SLOOP's SYN kits; its MPE setting no longer sits where SLOOP 2.6 keeps GLO > SYSTEM > IN. User presets are one bank again for both (192-byte records: 0.4's first build made them 232, which SLOOP read as empty; they are converted at power-on): the matrix, LFO 2 and ENV 2 ride in the record's spare values, SLOOP sees the sound as it is. A project keeps LFO 2, ENV 2 and the matrix in track 3's free lock slots, which SLOOP passes over (0.4's first build put them in tracks 3 and 4's own values; still read).

## New in 0.4: a modulation matrix on every engine, and MPE

- **MATRIX** (LFO → MATRIX), on every engine: six slots, each a **SOURCE** → **DESTINATION** × **AMOUNT** (±). SLOT chooses the slot the other three knobs edit; the screen lists all six. Sources: **LFO1**, **LFO2**, **ENV1** (the amplitude envelope), **ENV2**, **VEL**, **KEY** (the note, around C4), **MW**, **AT** (channel or polyphonic), **RAND** (a new random value at each note), **SLIDE** (MPE). Destinations: **PITCH** (up to ±24 semitones, finer near 0), **CUT**, **SHAPE**, **AMP**, **PAN**, **LFO2** (its rate), and any of the engine's eight EDIT values by name (ANALOG's CUT, WAVE's POS, FM6's ratios...), per note. A slot at 0 costs nothing and changes nothing, bit for bit.
- **LFO 2** (LFO → LFO 2): RATE, WAVE (SIN TRI SAW SQR S&H), MODE: **FREE** (one for the sound) or **KEY** (each note its own, restarted at the note).
- **ENV 2** (ENV → ENV 2): a second ADSR, for the matrix.
- **MPE** (HOME held → SYSTEM → MPE): OFF, or the member channels' bend range (48 ST, the MPE standard, or 2, 12, 24, 96). For Expressive E Osmose, ROLI, LinnStrument, Sensel and the rest: channel 1 is the master channel (the bend, wheel and pedal of every note), channels 2 to 16 each carry one note with **its own pitch bend**, **its own pressure** (the AT source and MOD AT>) and **its own slide** (CC 74: the SLIDE source; with no slot using it, it opens the note's filter).
- **Kept with the sound**: projects keep LFO 2, ENV 2 and the matrix of both sounds (the layer too) in room the synth never used (0.4.1: track 3's free lock slots); a project keeps its size, older ones load with the matrix empty. User presets keep them too (0.4.1: in the record SLOOP reads). RANDOM ALL also moves LFO 2, ENV 2 and the amounts of the slots in use; MORPH moves every one of them.

## New in 0.3: playing it like a big synth

- **Master EQ and compressor**: FX → **EQ+COMP**: LOW (a shelf at 150 Hz), MID (a peak at 1 kHz), HIGH (a shelf at 5 kHz), each ±12 dB (MID ±10) in 1 dB steps, and **COMP**, a stereo bus compressor (the "glue": one knob lowers the threshold from -6 to -30 dB and raises the ratio from 2:1 to 4:1, with automatic make-up gain). Flat and at 0 they are bypassed, bit for bit; every change glides, no click.
- **Layer and split**: EDIT → **LAYER**: MODE (OFF / LAYER / SPLIT), SOUND (any of the 308 sounds or your presets as the second sound), SPLIT (the split note: the layer below, the main sound from it up), LEVEL; **LAYER 2**: OCT (the layer -2..+2 octaves), PAN, FILT and **SWAP** (the two sounds trade places: every page then edits the other one; SWAP again to go back). Keys, MIDI, the sequencer and the arpeggiator all play both; a note always ends where it started.
- **Poly unison on every engine**: EDIT → VOICE: **UNI2** and **UNI4** stack 2 or 4 detuned voices on each note (DTUNE on VOICE 2), in POLY. (UNI is still the 8-voice mono stack.)
- **Morph and randomizer**: EDIT → **MORPH**: SET A, SET B, then MORPH moves the whole sound between them (every continuous value; the switches change half way; MIDI CC 20). EDIT → **RANDOM**: AMT, WHAT (EDIT: the engine's 8 values, +ENV: and the envelopes, ALL: and the LFO and the sends), RAND, and BACK to undo.
- **Modulation**: LFO → **MOD**: VEL>F (velocity to the cutoff, ±), MW> and AT> (the mod wheel and aftertouch, channel or polyphonic) to VIB (a 5.5 Hz vibrato), CUT, SHP (the engine's shape), WAH (the sound's LFO on the cutoff) or TREM. Every engine (HYBRID keeps its own matrix).
- **Smooth sound changes**: a preset, an engine, a user preset, SWAP, RAND or BACK changed under ringing notes fades them out over 6 ms with their own values (no click); notes played meanwhile wait for the new sound.
- **WAVE loads 6x faster**: a table's band-limited copies come from an inverse FFT instead of additive sums (the same samples within 2 steps of 32767), four frames per pass of the main loop: a new table is ready in four passes.
- **No full-screen visualiser** (its code and 5.5 KB of RAM go to the synth); HOME keeps its scope.
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
| **HOME** | The synth screen: the sound's name, its family, its number in the list, the engine and a live scope. The four knobs play the sound's main controls (for HYBRID: CUT, RES, M1, M2). |
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

The voices are a build option: 8 (the default, as SLOOP), 12 or 16 (`INSTALL-CLIPPER.bat 12`). Since 0.7 the second core renders half of a sound's voices (MENU → SYS → 2ND CORE), which is what 12 or 16 voices need. The host measurements put a typical HYBRID voice at about 2.5 % of the FM-1's CPU and the heaviest factory patch (CLIPPER, two SUPERs) at about 6.6 %. GLO → SYSTEM → CPU shows the real load on the device: those readings decide the default.

## Tests

`HOST_ONLY=1 tests/run_tests.sh` runs everything that does not need the JieLi toolchain (0.8: JUNO and its chorus, VECTOR, the MOD SEQ, SPICE / DICE, their pages; 0.7: the voices on both cores in every mode of the host double of core 1, and under ThreadSanitizer; UNDO, COMPARE, FAV, KBD HOLD): the synth mode (HOME, presets, keys, pages, one instrument, MIDI omni), the HYBRID tests (the pitch of every oscillator, the filters, the matrix, the controllers, 300 random patches, the cost per voice), the editor's commands, golden renders of every sound, ASan and UBSan, a soak test, and the web pages.

## Credits

- **[Felucca](https://github.com/hugelton/Felucca)** by Leo Kuroshita (@kurogedelic) / Hügelton Instruments: the platform, the engines, the editor and the installer.
- **The second core** (0.7): its HAL (`firmware/hal/fm1_cpu1.{h,S}`) is Charles Vestal's ([X0X](https://github.com/charlesvestal/fm1-x0x)), after Kerem Kilic's [Melodee](https://github.com/keremimo/melodee); brought to SLOOP by Andre Rezemini ([sloop-lab](https://github.com/andrerezemini/sloop-lab), `cpu1.c`, `tests/cpu1_host.c`); 2.7.1's hold with the bus traps off after Felucca 1.6.1. Clipper's voice split is its own.
- **Knob acceleration, LEVEL / PAN glide** (0.7): after [JIANT FM](https://github.com/juanjiant-bit/JIANT) by juanjiant-bit, through SLOOP 2.7.
- **[SLOOP](https://github.com/isod89/sloop-fm1)** by 3dSam: everything Clipper starts from. Its manuals are in [docs/sloop](docs/sloop/).
- The supersaw follows Adam Szabo's analysis of the JP-8000. The FM6 engine is msfa (Dexed), and PHYS comes from DaisySP and Mutable Instruments Rings: see [LICENSING.md](LICENSING.md).

M-VAVE and FM-1 are trademarks of their respective owners. Clipper is not affiliated with them.
