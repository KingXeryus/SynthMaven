# SynthMaven software 0.4 — instrument guide

These controls are shared by Mac standalone, VST3, Audio Unit and the iOS
app/AUv3 targets. This guide covers the software instrument, not FM1 hardware.

## Nine-channel mixer

Open **LAYERS**. Eight channel strips feed a stereo master strip.

- Select L1–L8 to choose the layer edited by the browser and other pages.
- **− / +** at the top changes that layer's transpose by one octave, within the
  existing −24 to +24 semitone range. Any semitone offset is retained.
- **LAYER** opens a popup. Layer plays the whole keyboard. Split enables a lower
  zone (notes up to and including the split point) or upper zone (notes above
  it). Choose the split point and MIDI channel independently for each layer.
- **ON** activates the layer. **M** mutes its output. **S** solos it; multiple
  soloed layers play together. A muted soloed layer remains muted.
- The pan knob sits above the level fader. Meters show each strip's output.
  Faders run from silence to 0 dB (unity). Right-click pan/fader for MIDI Learn.
- **EDIT** opens the layer's sound editor (Sampler for sampled layers). The
  instrument/preset name appears below the controls; hover for the full name.
- Master level, stereo balance and mute affect the combined output. The master
  does not have a redundant solo control. Watch the output meter when combining
  loud layers and lower their levels/master to avoid clipping.

Layer faders are now independent; raising one no longer normalizes other layers.
When an older preset is loaded, a compensating master level preserves its initial
mix balance. Existing parameter IDs remain stable; new controls have new IDs.
Changes to octave, input channel or split geometry release existing voices to
prevent stuck notes. Mute/solo operate on output while enabled voices continue.

## Wave sampler

Choose the layer, then open **SAMPLER** and import or drop one audio file.
Supported decoders include WAV/AIFF/FLAC and platform-dependent additional formats.
Use mono/stereo audio up to 30 seconds at 8–192 kHz. Audio is resampled to 44.1
kHz and embedded as 24-bit WAV; the source file is not modified.

The sampler replaces the selected layer's synthesis engine while enabled. The
factory browser still contains the existing 24 banks / 3,383 presets; loading a
factory sound returns that layer to its synthesis engine. Sampler is an additional
user-audio source, not a bank of new factory sounds. Other layers can use any
combination of sampled audio and synth engines.

### Waveform, pitch and playback

- Drag near START/END to trim non-destructively. Shift-drag moves loop markers.
  The corresponding knobs provide precise time readouts.
- Set **ROOT KEY** to the original recorded pitch. ROOT C4 resets root to MIDI
  note 60 and fine tuning to zero. Higher/lower notes transpose playback across
  the MIDI range. This is conventional pitched resampling: pitch changes also
  change sample duration; independent time-stretch is not implemented.
- **FINE TUNE** spans −100 to +100 cents. Layer transpose and custom/master tuning
  also affect sampler playback.
- **CLASSIC** follows held notes and the release envelope. **ONE SHOT** continues
  to the end after an ordinary note-off. **LOOP** repeats the chosen loop region
  while the envelope remains active. Panic/all-notes-off remain safety controls.
- **REVERSE** plays backwards. **LOOP XFADE** blends the loop seam (up to 50 ms,
  limited by loop length). Trimmed ends have a short safety fade.

### Synthesis and performance

ATTACK, DECAY, SUSTAIN and RELEASE shape amplitude. CUTOFF and RESONANCE shape
the low-pass filter. VELOCITY blends fixed level with note-on velocity response.
The sampler is 16-voice polyphonic per layer, with sustain, expression and pitch
bend. Its sine LFO controls pitch and filter modulation; use LFO RATE, LFO PITCH
and LFO FILTER, also available on the sampled layer's Modulation page.

For a sampled layer, Effects exposes chorus, a fixed 250 ms feedback delay and
reverb sends. These are software sampler effects; the native engine's full
modulation matrix and detailed effect algorithms are not applied to sampled audio.
MIDI Learn/DAW automation can control sampler parameters, layer and master controls.
Default CCs include 7 level, 10 pan, 71 resonance, 74 cutoff, 73 attack, 75 decay,
72 release, 91 reverb, 93 chorus and 94 delay. CC11 controls expression and CC64
sustain. Pitch bend is ±2 semitones for the sampler.

### Save and recall

Save the complete layered instrument with SAVE or save the DAW project. Audio,
trim/loop positions, pitch, envelopes, effects, split/mixer settings, mappings and
tuning are recalled. The sample does not need to remain at its imported path.
Eight long stereo samples make larger preset/project files; keep backup copies.
CLEAR removes the selected layer's sample and returns it to the underlying synth.

## Bluetooth MIDI on iOS

Use **MIDI MAP → BLUETOOTH MIDI**, allow Bluetooth access, and connect the device
in Apple's pairing panel. Standalone detects newly available Core MIDI inputs.
In AUv3, route the paired device through the host DAW. See [iOS setup and build
status](IOS.md); simulator/unsigned builds do not prove physical radio performance.

### Structured parameter browser
All Parameters groups controls by Engine, Amplitude, Filter, Voices, LFOs, Modulation Envelope, Routing, Arpeggiator, Sequencing, Effects and Macros. Use the section selector at the top to focus on one section or show all sections. Controls use descriptive labels and wrap to suit the available width. Split points are selected by musical note name (including A0–A6); C4 is middle C.

### Parameter graphics
Envelope diagrams show normalized ADSR shape; drag the attack, decay/sustain or release handles to edit the same parameters as the knobs. LFO panels preview the selected waveform and rate (random shapes are illustrative). Filter panels show an illustrative response, not an exact DSP measurement; drag horizontally for cutoff and vertically for resonance when these controls are exposed by the engine. Effect-send bars adjust chorus, delay and reverb sends. Live Output displays actual mixed audio from all layers. These displays follow MIDI and host automation.

### Three-source oscillator section (software 0.4)
Each layer offers OSC 1 (the selected original synth engine or sampler), plus OSC 2 and OSC 3 as independent pitched oscillators. The original engines keep their internal architectures. Extra oscillators support sine, band-limited saw/square, and triangle, octave shift, fine tuning, level and pan. OSC 2/3 share their own ADSR and 16-voice pool per layer. They follow incoming notes, velocity, sustain, pitch bend, layer splits, transpose and scale tuning. They are mixed after the original source's internal filter/effects, before the layer mixer; the original source filter and portamento do not process the extra oscillators. Both are off by default for preset compatibility.

The Effects page adds a stereo chorus and a flanger after the combined source mix. Chorus has Mix, Rate and Depth; Flanger adds bipolar Feedback. Existing engine chorus sends remain independent. New effect Mix controls default to zero. The new controls are stored in patches, exposed to DAW automation and assignable from MIDI Map.

### Expanded graphical controls (0.4.1)
Attack, decay, sustain and release have shared ADSR plots for the original amplitude envelope, modulation envelope, extra oscillators and sampler. Move the handles or the corresponding knobs. Filter plots respond to cutoff and resonance; sampler filters have their own plot. Native engines use their existing envelope-to-filter depth, rather than adding an unsupported independent filter envelope.

The Effects page groups graphical modules with their controls: delay echoes (horizontal drag edits time, vertical edits send), reverb tail (horizontal edits size, vertical edits send), EQ (drag the low/mid/high band vertically), distortion transfer, and stereo chorus/flanger modulation traces (horizontal edits rate, vertical edits depth). Sampler delay uses fixed 250 ms timing/feedback and sampler reverb uses fixed size/damping, so their graphs edit sends only. LFO plots allow horizontal rate editing. Routing diagrams show configured source/destination connections for the first six slots.

All response and decay diagrams are illustrative normalized previews, not measurements or exact DSP impulse responses. The live output trace and imported sample waveform display actual audio data.
