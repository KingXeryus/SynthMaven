# SynthMaven 2.2 - factory program design and research

The bank follows the requested order: P1 Piano, P2 Epiano, P3 Lead Synth, P4 Oriental Lead, P5 Disco Synth, P6 Brass, P7 Strings, P8 Bass. Each program has four enabled parts, with its main instrument in part 1. These are original combinations and edits of the existing SynthMaven sound library; no manufacturer patches, recordings or copyrighted preset downloads were imported for this bank.

## Research method

Research used the Firecrawl deep-research skill/plugin to search and retrieve eight substantive sources: a manufacturer's detailed layering tutorial, two synthesist essays on envelopes and expressive sound design, two engineering articles on masking/mono, and three instrument manuals or manufacturer descriptions. Findings below informed the recipes; they do not establish that a particular preset will suit every listener. Quality is ultimately a listening judgment. Rendered audio previews make the choices reviewable.

### 1. Assign a purpose to every layer

Roland's JD-800 tutorial separates contrasting, complementary and cohesive layering. It demonstrates combining different attacks and sustained timbres, matching envelopes when sounds should blend, and adding a fundamental or a transient only when that element is missing. We use a foreground instrument plus supporting articulation, body or air. An extra layer must earn its place rather than duplicate the entire main instrument at equal level. [Roland: The Art of Layering](https://articles.roland.com/sound-design-the-art-of-layering-with-the-roland-jd-800/)

### 2. Shape the phrase, not just the oscillator

Gordon Reid explains that an audible envelope is the combined result of several controls and that gate/retrigger behavior affects the musical phrase. Our bass and lead programs use monophonic articulation, while keys, brass and strings retain a polyphonic main voice. Short accents and longer supports are shaped separately. [Sound On Sound: Envelopes, Gates & Triggers](https://www.soundonsound.com/techniques/envelopes-gates-triggers)

### 3. Match sources to the intended instrument

Reid's concluding Synth Secrets essay emphasizes recognizing suitable source waveforms, understanding the instrument's limits and using performance controls for expression. We keep each requested instrument recognizable before adding support, use modest modulation, and avoid random fixed-interval harmonies. P1 is explicitly a synthesized stage piano, not an acoustic-grand multisample. [Sound On Sound: The Secret Of The Big Red Button](https://www.soundonsound.com/techniques/secret-big-red-button)

### 4. Keep a foreground/background hierarchy

iZotope distinguishes useful blending from masking that obscures musical information. Its discussion stresses deciding which element belongs in front before changing the balance. We measure isolated main/support parts as well as the combined program, keep the main instrument foremost, and use quieter voicings and modest filtering to separate support parts. [iZotope: Frequency Masking](https://www.izotope.com/community/blog/what-is-frequency-masking)

### 5. Stereo width must survive mono

iZotope explains why stereo summing and phase relationships matter, particularly at low frequencies. We keep the bass centred, use small pan offsets elsewhere, avoid polarity-based widening, and measure mono retention from the rendered stereo signal. Width should add interest rather than supply the only audible body. [iZotope: Mono vs. Stereo](https://www.izotope.com/community/blog/mono-vs-stereo)

### 6. Save the complete performance combination

Ableton's Rack manual documents separate chain levels, pan and input zones, and describes saving chains as a coherent preset. Our program recipes store four sounds, their mixer/routing, tuning and four scene variants together. MIDI macros and mixer controls remain available through the existing interface; we do not add a new navigation system. [Ableton: Instrument, Drum and Effect Racks](https://www.ableton.com/en/manual/instrument-drum-and-effect-racks/)

### 7. Separate layering from keyboard splitting

Sequential describes stack and split as different performance choices: a richer shared-key sound versus different instruments across keyboard regions. All eight requested programs start as four-part layers. The existing split controls remain available for the player's own arrangements. We do not silently turn the supplied piano into a bass/keys split. [Sequential: Prophet Rev2](https://sequential.com/hybrid-analog/prophet-rev2/)

### 8. A multi-part setup is a performance object

Spectrasonics distinguishes live switching/layering from stack arrangements. The useful principle for this hardware is that the saved combination must be understandable and playable, rather than just a collection of unrelated patches. Each factory slot therefore has a single instrumental focus and a documented role for each supporting part. [Spectrasonics: Live Mode and Stack Mode](https://support.spectrasonics.net/manual/Omnisphere3/30/en/topic/using-live-mode-and-stack-mode)

## Adaptation to FM-1 constraints

These principles are interpreted for this firmware's eight shared synthesis voices. There are four enabled parts, not four independent eight-voice synths. P1 and P5 retain a polyphonic main instrument with three quieter monophonic followers. P2, P6 and P7 use two polyphonic parts and two mono supports. P3, P4 and P8 are fully monophonic combinations. A mono support follows its own last-held-note behavior; it does not reproduce every note of a chord. This is a deliberate voicing choice to leave more room for the main instrument. Release tails still consume voices, and larger chords can steal voices.

All transpositions are octaves, with no automatic fifths or thirds that would fight a chosen maqam. The bank starts in equal temperament, chord generation and arpeggiation OFF, master tuning applied to all parts, and layering ON. Users choose a maqam/root explicitly. Factory loading preserves existing custom-scale definitions. The 557 sound presets and 64 Vital-derived entries remain available without moving their IDs.

## Data and validation

- `assets/programs/recipes.json` is the editable source of all eight designs.
- `tools/gen_factory_programs.py` runs the production sound loader and emits complete packed programs plus the editor catalogue. The firmware and editor preview use the same generated data.
- Eight ROM defaults sit underneath the eight saved slots. Existing saved programs and working-program autosaves take precedence; loading/saving does not automatically overwrite them. Factory defaults do not require flash writes.
- MAIN, INTIMATE, AIR and DRY are four stored scene variants. Scenes change the existing supported controls; they are not extra synthesis engines or extra voices.
- Audio checks render the actual firmware DSP and routing. They measure programme loudness, oversampled peaks, mono retention and isolated-part audibility. They also check the eight-voice limit and stress input at high velocity. Measurements are engineering checks, not a substitute for listening or physical-device validation.
- All 556 audible factory sounds are recalibrated to −14 LUFS on documented references (±0.5 LU, ≤−1 dBTP). Each isolated program part and layered scene is matched to the same target. USER WT remains silent until populated. Fixed solo gains preserve the intended quieter support balance when layered.
- Short support articulations are long enough to function as solo sounds as well as accents; the shortest click-like envelopes were lengthened before matching. INTIMATE reduces ambience and bright support; AIR emphasizes the backing texture; DRY removes shared effect sends. Their level offsets keep the scene mixes comparable.
- Each factory sound is rendered in an isolated process to reset effect tails, limiter history and random state. A sample-rate-correct BS.1770-4 meter at 44.1 kHz uses absolute/relative gating and 4× peak measurement; an independent pyloudnorm DeMan meter cross-check covers all 23 engines and the transient references.
- Explicit short/transient sound references use a 16 Hz roll. Sustained, bass, comping and melody references remain distinct. Actual loudness changes with the performance; gain does not follow a live loudness measurement.

Physical FM-1/controller validation remains pending. The downloadable previews are host renders of the production synthesis code, not recordings from an FM-1 audio output.

Reproducible reports: [factory sounds](../assets/factory-level-audit.json), [programs/scenes/solos](../assets/programs/audit.json). The release includes the eight-program WAV preview archive and fresh CI audio audits.
