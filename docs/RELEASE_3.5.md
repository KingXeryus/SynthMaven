# SynthMaven 3.5 — Melodee base, USB-first

SynthMaven 3.5 migrates to Melodee 1.5.1 while bringing over four-part programs, HYBRID/Vital translations, DX7 sounds, global chromatic tuning and USB editor control. Bluetooth is not included in this release.

## What changes

- Eight named four-part factory programs, layering/splits, part volume/pan, four compact scenes and 32-entry setlists. Factory mixes use economical mono support parts; solo gain processing is unchanged.
- 14 Western scales, 46 Arabic maqam settings and global custom tuning, selectable root, octave threshold, scale-aware chords and per-part tuning bypass. Programs override global tuning only when explicitly saved with it.
- HYBRID native patch editing, 64 factory Vital translations, additional DX7 voices and explicit Vital JSON approximation import. Macro routes can be assigned and saved with the sound.
- MIDI Learn in web/iOS, Learn → Assign → Save, a 16-action grid with Relearn/Unlearn, live match feedback, duplicate suppression, USB device/interface status and controller profiles including MCU/HUI.
- USB-first SM FM1 iPad app (landscape only), shared Perform controls with program/scene navigation, layer mixer and tuning keyboard.
- Independent eight-lane drums alongside all four program parts, a second drum HOME page, and 107 restored ANALOG/DIGITAL/VOICE/STRINGS/JUNO presets.
- Cohesive touch workspaces for Programs, Library, Settings, sequencer and 6-OP FM, with paged controls instead of scrolling.
- Ten named and numbered GLO pages include theme-color presets, USB, autosave, control and recording preferences; MIDI mapping remains in the editors.
- Melodee graphics, sequencer, USB audio and native instrument editors. GLO two-second layer toggle, EDIT two-second mixer, ENV three-second chords, SEL tuning. HOME OCT−/OCT+ three-second holds select the previous/next program; short taps change octave. Autosave retained.

Factory programs: **DX7 Stage, E. Organ, Rhodes E-Piano, Disco Lead, Vintage Lead, Lead Guitar, Arabic Strings and RnB Bass**. The eight programs preserve the owner’s final FM-1 edits, including native patches, supporting layers, effects and scenes; P2 is renamed E. Organ.

## Install and recover

1. Make a full backup before installation. Every flash resets user settings, mappings and programs.
2. Use the live installer in Chrome/Edge, or the included firmware package with the documented USB installer. Set USB role to COMPUTER.
3. Keep USB power connected through completion and automatic restart.
4. On first boot, factory P1 loads. Check MASTER before auditioning or recording USB audio.
5. If recovery is needed, use the matching recovery archive for Windows x64, macOS arm64/x64 or Linux x64. The illustrated PDF and BUILDING.md cover recovery.

Do not restore a SynthMaven 3.0 binary backup into 3.5. Preserve it for rollback with its original firmware. Older files are not silently reinterpreted.

## Validation and limits

Physical tests of the USB-only candidate covered eight programs with one held note and three-note chords, both alone and with independent drums, USB audio capture, global tuning across program changes, program save/reload, MIDI Learn/match, and factory reset on reflash. The measured captures had no full-scale rail samples or complete-buffer deadline misses. These bounded tests do not prove every possible sound/effects combination is overload-free.

All four parts share CPU, memory and voice resources. Monophonic parts replace their previous note when a chord is played. The ensemble string engine reserves four budget units per voice to protect layered playback with drums. Vital import approximates supported settings; it is not Vital playback. Direct USB controllers need a suitable powered adapter/hub and compatible mode. Controller and iPad model coverage remains limited. The physical MASTER affects USB audio recording.

Bluetooth remains a separate experiment. No Bluetooth functionality or performance claim is made by 3.5 USB-first. Firmware/recovery checksums are in SHA256SUMS; firmware provenance is checked against the required shared builder before publication.

Based on Melodee by Kerem Kilic (Ellic Studio), Felucca by Leo Kuroshita / Hügelton Instruments, retained Clipper/3dSam-derived SynthMaven work, and the credited third-party components. See the licence archive.

## Retained engines and space budget

Version 3.5 retains DX7/FM6, HYBRID/Vital translations, ANALOG, DIGITAL, VOICE, STRINGS, JUNO, ACID, VPS and WSEQ alongside the Melodee native engines. ACID/VPS/WSEQ add 42 presets. FMX, VECTOR, TERRAIN, WAVE and MACRO are excluded. WSEQ retains its sequences and crossfades with compact oscillator waveforms; it is not a bit-identical port of the older FMX-table oscillator. Four layers and independent drums remain available. The startup icon is losslessly packed without reducing resolution.

The shared editors include named custom-scale saving, compatible Scala import/export, modulation templates, engine-aware filter controls, two-card sound editing and paged Effects/LFO/ARP controls. Output compensation and Bluetooth are excluded. See the illustrated guide for direct USB host and powered-hub wiring, and the separate computer/iPad-hosted editing setup.
