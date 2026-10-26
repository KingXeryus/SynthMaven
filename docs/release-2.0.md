# SynthMaven 2.0

Protocol 22 adds pedal learning, compact scenes, a global setlist and engine-aware filter editing. The release retains 23 engines and 493 sound presets. Eight voices are shared across all four program parts; four layered parts consume four voices per played note. Unison and release tails also consume that budget.

## Pedals

GLO → PEDAL LEARN: choose one of ten actions, turn LEARN to arm, press the pedal, review the captured CC/PC and channel, then turn ACCEPT. Learning never changes an assignment until accepted. Conflicting assignments are refused. PATCH/BANK actions change the selected part; PROGRAM actions navigate occupied saved slots; SCENE actions navigate populated scenes; SETLIST actions follow the saved order. Existing four CC/PC assignments remain compatible.

The editor provides the same Learn/Accept flow in Settings. A directly connected pedal requires PEDAL host mode, so learning with that connection is performed on the FM1. The editor can learn USB messages routed through a computer while in COMPUTER mode. The FM1 cannot serve as a USB peripheral to the computer while hosting a pedal.

Diagnostics distinguish USB device presence from a class-compliant MIDI interface and show the last received message, matched action, trigger, suppressed duplicate, disabled patch routing or full queue. A test indication confirms an accepted action; a program load can still fail if its target slot is empty. GLO → USB ROLE → COMPUTER restores editor, USB audio, backup and installer connectivity. Use a powered USB adapter/hub. Physical Chocolate Plus interoperability remains unverified.

Duplicate suppression is configurable from 0–500 ms and defaults to OFF. It compares identical accepted messages per action; a CC release immediately re-arms that action. Identical PC messages cannot identify physical release: intentional PC taps inside an enabled suppression window are also suppressed. Keep the window shorter than the fastest intended tap, or leave OFF.

## Programs, scenes and setlist

A **sound preset** is one engine sound. A **four-part program** stores four sounds, their routing/mix/tuning and four compact scenes. An **editor library entry** is a computer-side sound record, not a program or scene.

Each scene captures four levels, mutes, distortion/chorus/delay/reverb sends, four macros per part, tuning system/root/A4, per-part master-tuning switches and compensation switches. Scenes do not replace engines, presets, splits, pan or modulation matrices. Capture a scene, then save the program to persist it. GLO SCENES and the editor provide Capture/Recall; the editor can clear a scene. The current program has four scene slots.

The global setlist contains up to 32 ordered references to the eight saved program slots, including repeated references. Reordering never copies or overwrites a program. Empty references are reported rather than silently skipped. First NEXT starts at entry 1; first PREVIOUS starts at the final entry; subsequent navigation wraps. Program changes stop playback and wait for voices to release before flash reads; queued taps remain ordered. A five-second timeout reports a load that cannot become quiet.

Storage uses PRG3 programs and PBK2 program pairs, within the existing atomic A/B flash sectors. PRG1, PRG2 and PBK1 saves import with empty scenes and compensation OFF. No sample or wavetable sectors are reclaimed. New backups require protocol 22; the editor refuses restoring them to older firmware. Back up before updating; the new format is not a downgrade format.

## Filters and modulation

[Filter capability map](FILTER_CAPABILITIES.md) identifies each engine's actual model, cutoff, resonance, envelope and drive controls. EDIT groups model/cutoff/resonance/envelope together, followed by drive, compensation and trim where applicable. Fixed models remain read-only; nonexistent controls are disabled. Engines without an exposed subtractive filter omit these device pages. Units and ranges follow native descriptors; trim displays half-decibel steps. HYBRID controls filter 1 without changing filter 2 or routing.

Velocity→brightness, pressure→vibrato, wheel→vibrato and envelope-swell templates reuse existing modulation. HYBRID pressure/wheel templates need a free matrix slot and set LFO1 to sine, rate 64; full matrices are refused without edits. Optional compensation is estimated resonance/drive attenuation up to 12 dB, OFF by default. It is not measured loudness normalization and adds no filter DSP.

## Validation and limits

Host tests exercise MIDI learning/deduplication, scene persistence, setlist ordering, migration, capability descriptors and backup downgrade guards. Target build and memory checks remain required release gates. Hardware pedal enumeration, USB power compatibility and real-time operation still require physical FM1 testing; do not treat simulated diagnostics as hardware certification.

### Development update: catalogue and Vital import

Regional D/E pairs consolidate from 88 entries into 44 selectable sounds with editor variations. The catalogue is now 493 entries (492 audible plus USER WT); existing programs preserve all sound parameters and migrate regional IDs, and favourites deduplicate D/E references.

Vital import is a staged editor translation into HYBRID, with explicit approximation warnings, library persistence, exported translations and raw Wave Source keyframe extraction. Custom translated patches can be saved in four-part programs, not parameter-only user slots. No Vital preset content is included; factory additions await supplied files and a fresh loudness audit.
