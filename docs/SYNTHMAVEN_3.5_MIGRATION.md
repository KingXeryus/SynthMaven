# SynthMaven 3.5 migration — development only

This branch starts at Melodee v1.5.1 (commit 6a608ba122ac42293908d394e26713e31ec52fb0).
The source donor is SynthMaven e49239e. Published SynthMaven 3.0 remains the recovery/reference release.
This document is a work ledger, not release notes or evidence of completed validation.

## Approved sequence

1. Pin Melodee; retain scheduler, SRAM audio placement, recovery and regression gates.
2. Port HYBRID/Vital translations and DX7 bank; four-part layering/splits/mixer;
   global chromatic microtuning with explicit program override; named programs,
   scenes/setlists; pedal/controller support; USB web/iOS control.
3. Validate firmware candidate 3.5. Preserve autosave and established solo levels.
4. Add editor MIDI learn for actions and parameters, with firmware persistence.
5. Release validated USB-only 3.5, installer, guides and iOS TestFlight.
6. Independently integrate fm1-base's own BLE stack for one editor connection.
7. Test concurrent audio/editing, report evidence and pause for a recommendation.

## Integration contracts

- Existing Melodee stored engine IDs stay unchanged. HYBRID appends as 22.
- Retain SynthMaven HOME navigation and shortcuts, using Melodee visual components.
- No Bluetooth audio, central scanning or TRS polarity in the first candidate.
- Never load a SynthMaven 3.0 binary program as a Melodee project. The formats and
  parameter IDs differ. Conversion must identify unsupported engines explicitly.
- The future editor handshake must advertise SynthMaven 3.5 capabilities before
  enabling writes; Melodee and previous SynthMaven command IDs are not interchangeable.
- Stock Melodee autosave is retained while SynthMaven program semantics are ported.
- Reuse existing MIDI routing; add bounded mapping dispatch and validate every
  learned destination against the live engine descriptor. Mapping configuration
  is editor-only; mappings execute on hardware without the editor.
- Target builds must pass audio call-graph placement and resource bounds. Firmware
  release packages must be attested CI builds per AGENTS.md, not local candidates.

## Implemented foundation

- Isolated `synthmaven-3.5-melodee` branch at the pinned base. Development identity
  `3.5-dev`; release metadata remains `development`, `release_validated: false`.
- HYBRID appended as engine 22 without renumbering Melodee engines or rotating
  existing browser banks. 105 presets, including 64 Vital translations, use
  allocator-backed per-part state and independent worker scratch.
- 33 supplied DX7 voices appended after original FM6 entries and INIT. The
  source SysEx files remain unchanged. Performance copies bound R4 and zero L4;
  see `assets/fm6/README.md`. Original Melodee factory sounds remain unchanged.
- HYBRID native sound-library, project, template/autosave and undo paths preserve
  its 128-byte patch, including signed fields, within unchanged flash extents.
  Malformed patches are rejected before writes. Tests cover interrupted sound
  saves, program round-trips and undo/redo. New favorites use tagged storage so
  old native-bank favorites do not become unrelated DX7/HYBRID stars.
- HOME: ALGORITHM selects engine, PRESETS wraps within that engine, SELECT selects
  one of four audio parts. Other navigation is still Melodee's pending shortcut port.
- NIGHT palette uses SynthMaven black/white/red with blue/grey part accents.
  All existing palette contrast checks pass. This is not the completed GUI refresh.
- USB native HYBRID command 96 and shared-library native kind 5. INFO supplies
  an explicit SynthMaven capability tag. Web capture, native save/read, export and
  audition preserve the full patch and refuse unsupported firmware before writes.
  See `web/EDITOR_PROTOCOL.md`. This is not yet the final iOS/Perform interface.
- Per-voice HYBRID DC removal fixes offset observed by the audio health tests.
  Long HYBRID release validation follows the actual envelope time constant.

- Global chromatic tuning: all 46 maqam profiles, root, C0–C8 threshold,
  custom ±100-cent offsets, MIDI quantization and per-part bypass. Device fine
  controls use 10-cent steps; USB command 110 and editor controls retain one-cent
  values. Settings retain autosave and survive ordinary project loads.
- Four-part layer routing uses existing bounded chord ownership and weighted
  voice admission. Command 111 and editor mixer controls expose layering and
  per-part inclusive note ranges. Physical keys, USB, sustain and releases
  through mode changes have focused tests. Final program/shortcut integration
  is still pending; these settings currently persist globally.

## Validation evidence (local development, not release certification)

- `build/host/regression-audio-adapted-2.log`: 475 renders, zero changed/gone
  existing goldens, zero health failures, zero routing failures and zero crashes.
- `build/host/regression-golden-additions.log`: 138 new render baselines added;
  no existing golden lines replaced. These are deterministic regression hashes,
  not an independent listening review or device CPU measurement.
- `build/host/audio-matrix-hybrid.log`: 2,401 ordered engine assignments including
  HYBRID/Vital, serial versus real concurrent host worker PCM identical under
  controls, releases, retriggers, spread and shared chorus. Weighted admission
  remains enabled. Host timing does not establish FM-1 deadline compliance.
- `build/host/hybrid-storage.log`: native sound save, interrupted-write recovery,
  full project signed-byte round-trip, malformed rejection, undo and redo pass.
- `build/host/editor-hybrid.log`: real C USB handlers pass full native HYBRID
  round-trips, invalid writes, wrong engine, bounds and shared-library storage.
- `build/hybrid-target-3.log`: target build passes stack, SRAM audio-call-graph
  and memory checks; application 548,056 bytes, development package 610,583 bytes.
- Broader suite logs and final web follow-up are in `build/host/`; inspect their
  final result, not an earlier failed run, before declaring the whole suite green.

### Tuning and layer checkpoint

- `build/host/regression-layers-fixed.log`: complete non-audio suite passes
  (132 seconds), including persistence, interrupted updates, USB protocol,
  diagnostics, UI layout lint, input behavior and web protocol tests.
- `build/host/regression-layers.log`: audio checks pass with 475 unchanged
  golden renders and 2,401 serial/concurrent combinations. Its non-audio
  failures were corrected in the follow-up above; do not label that earlier
  whole run passing.
- `build/layers-target.log`: target stack, SRAM placement and audio-resource
  checks pass; app 552,576 bytes, loader 7,427 bytes, development package 610,583
  bytes. Audio resources 119,436 bytes, plus optional 28,672-byte cache arena.
- `build/host/layer-routing.log`: split boundaries, USB/physical layers,
  sustain, changing modes while held, shared keyboard/MIDI ownership, one-part
  panic, bounded ownership exhaustion and settings persistence pass.
- `build/host/editor-layer-check.log`: real C native tuning/layer handlers pass.
- `web/test_master_tuning.mjs`: DOM interaction checks pass for single-tap,
  double-tap, cents readout, one-cent wheel, octave and per-part bypass.
- `build/host/web-branded.log`: web protocol, library and updater checks pass.
- Browser mock inspection: tuning controls and layer mixer respond without
  console errors. Dark-only shell uses supplied SynthMaven artwork and palette;
  full responsive iOS/Perform design is still pending.
- New command IDs are **110 (tuning)** and **111 (layers)**. IDs 97–101 are
  reserved by Melodee diagnostic builds and must not be reused.

## Remaining work — release blockers

- Explicit program tuning overrides and final tuning UI integration. Global
  chromatic tuning is implemented, but legacy Melodee scale pages still coexist.
- Program integration of layer/split routing, eight named programs, scenes/setlists, factory
  program migration and shortcut semantics. Four Melodee tracks alone do not
  complete the requested four-part program behavior.
- Old HYBRID preset FX/extra parameters have not been translated. The adapter
  currently carries oscillator/filter patches and amplitude envelopes; full
  sound equivalence and level calibration must not be claimed.
- Actions plus parameters MIDI Learn, conflicts, repeat suppression, durable
  assignments, and the final web/iOS mapping interface.
- USB iOS adaptation, shared Perform interface, dark-only responsive editor
  design, installer, guide/PDF, attested CI release and TestFlight distribution.
- Physical FM-1 audio/CPU validation. HYBRID admission costs remain provisional;
  successful host rendering cannot establish simultaneous four-part device cost.
- Independent BLE integration and its concurrent audio/editing tests, after the
  USB release, followed by the requested recommendation/pause.

No device flash, live installer change, TestFlight upload or public release has
been made from this development branch. The restored 3.0 firmware and original
SynthMaven checkout remain the recovery/reference state.

### Storage investigation

The current project wire record is 13,212 bytes and each atomic project bank
has 27,904 bytes of payload capacity. Two current-pattern records would occupy
26,424 bytes before program metadata/scenes, but would not retain all 32 pattern
banks of a Melodee project. Eight programs must therefore use an explicitly
versioned format and validated migration/backup handling. Do not simply enlarge
the existing four-slot array or claim that eight complete Melodee projects fit.

## Program, mapping and USB app integration (30 October, in progress)

- Commands 112/113/114 implement named programs/scenes, MIDI Learn/mappings and
  setlists. Eight sound programs share four atomic A/B objects, two per object.
  Each object is 7,512 bytes, including scenes and configuration. Sound programs
  intentionally exclude sequencer note/pattern banks and preserve the current sequence.
- Legacy project objects are protected in both directions. Program backup restore
  validates the complete object before publication. Interrupted writes retain the
  prior copy and the paired program.
- Scenes capture four levels, pans, mutes, three effect sends, HYBRID macros and
  tuning. Programs optionally override global tuning. Autosave uses the existing
  idle policy; byte-identical sound programs skip flash erasure.
- MIDI assignments capture a message, require acceptance, reject conflicts and
  survive reboot. Action dispatch is bounded and deferred out of the audio interrupt.
  Release messages rearm duplicate suppression to preserve intentional taps.
- GLO holds for two seconds before toggling layering and showing its state. EDIT
  holds for two seconds before retaining the mixer; ENV holds three seconds for
  chords. SEL opens master tuning; SELECT selects the scale. SEL held three seconds
  opens custom tuning. Undo is EDIT + OCT−; redo remains EDIT + OCT+.
- The existing iOS CoreMIDI transport and one-step USB selection are copied with
  their notices. The app bundles this branch's editor without legacy protocol bindings,
  displays “SM FM1 Editor”, and builds as 3.5 (11). No distribution has occurred.
  Bluetooth selection is absent from this USB-first candidate.
- Eight draft factory programs follow the requested instrument categories with
  mono supporting parts. These are new Melodee-native recipes, not exact copies of
  the owner-captured 3.0 programs. All 16 single-note/chord host renders are audible
  and have no full-scale samples. This is not a loudness-match, listening, or device
  CPU validation. No global solo gain or limiter change was introduced.

Evidence: `build/programs-shortcuts-target.log` passes target build/resource checks
(app 565,248 bytes, loader 7,427 bytes, development package 610,583 bytes).
`build/host/mapping-web-fixed.log` passes the web suite. `build/ios-build.log` passes
an unsigned simulator build; MIDI stream and JavaScript bridge tests pass.
`build/host/factory-audio.log` records the draft program renders. Full regression
is being rerun after shortcut and screen-label corrections; earlier failing logs
must not be described as a complete pass.

Still required beyond the blockers above: current-program/scene boot restoration,
final controller profiles, neutral-chord filtering, final shared Perform layout,
physical timing/audio/USB verification, and release documentation/attestation.

Read-only hardware check on 30 October confirms the connected “Melodee FM-1”
USB endpoint is running **FELUCCA SYNTHMAVEN 3.0** (INFO command 1). No candidate
was installed by this check.

Checkpoint validation: `build/host/program-regression-4.log` reports **ALL HOST
TESTS PASSED in 197 s** with SOUND=1, including the web suite, audio regressions,
atomic program/backup/mapping tests, new shortcuts, all UI fixtures and screen
layout/canvas parity. Device CPU/audio performance remains unverified.
