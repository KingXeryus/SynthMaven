# SynthMaven 3.6 for M-VAVE FM-1

SynthMaven 3.6 builds on Melodee 1.5.1, retaining its graphic menus, sequencer, USB audio and native instrument editors while adding SynthMaven performance and tuning features.

- **Device preferences:** ten named, numbered GLO pages, including theme-color presets, USB role, autosave, controls and recording settings. MIDI Learn remains in the editors.
- **Four-part programs:** eight named factory programs DX7 Stage, E. Organ, Rhodes E-Piano, Disco Lead, Vintage Lead, Lead Guitar, Arabic Strings and RnB Bass. Layer or split parts, adjust volume/pan, save four scenes per program, and navigate a 32-entry setlist.
- **Independent drum machine:** eight synthesized drum lanes alongside all four layers, a second device Home page, and separate editor transport controls.
- **Restored classic sounds:** 107 ANALOG, DIGITAL, VOICE, STRINGS and JUNO presets adapted from the older SynthMaven sources, including JUNO chorus. Existing engine IDs remain reserved.
- **Shared touch workflow:** Hybrid-style layer mixer, sequencer and drum grid, screen paging instead of scrolling, and consistent red/pink/light-blue/gray part colors. The iOS app is named SM FM1.
- **HYBRID and DX7 sounds:** native HYBRID patch editing, 64 factory Vital translations and additional DX7 voices. Import Vital presets as explicit approximations; edit oscillators, filters, envelopes, modulation and Macro 1–4 assignments. This is not the Vital DSP engine.
- **Global tuning:** 46 maqam settings, selectable root and octave threshold, custom chromatic offsets, per-part bypass and scale-aware chords. Tuning survives program changes unless that program explicitly includes tuning.
- **USB control:** shared web/iOS editor, a compact Perform screen and landscape-only SM FM1 iPad app, Learn → Assign → Save for up to 16 pedal/controller actions, with Relearn/Unlearn, configurable repeat suppression, MCU/HUI profiles and USB host status. Set COMPUTER for editing; PEDAL requires an appropriate powered host adapter/hub.
- **Panel shortcuts:** hold GLO two seconds to toggle layers, EDIT two seconds for the four-part mixer, ENV three seconds for chords. SEL opens tuning. Autosave remains enabled.

[Installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/) · [Web editor](https://kingxeryus.github.io/SynthMaven/webapp/editor/) · [User guide](https://kingxeryus.github.io/SynthMaven/webapp/manual/) · [PDF guide](https://kingxeryus.github.io/SynthMaven/webapp/manual/SynthMaven-3.5-User-Guide.pdf) · [Release notes](docs/RELEASE_3.5.md)

## Installation and limitations

Back up before installing: **every firmware flash resets user settings and programs**. Version 3.5 introduced a new storage format, which 3.6 keeps. Keep older whole-device backups for rollback with the matching firmware; do not restore a 3.0 binary backup into 3.5 or later.

This release is **USB-only**. Bluetooth adaptation is a separate experiment and will not be promoted without audio and timing validation. Four parts share CPU, memory and voice resources; they do not each receive an independent guaranteed voice budget. Factory backing parts use lighter mono voices to retain timing headroom. USB capture follows the physical MASTER control.

Use Chrome or Edge for Web MIDI installation/editing. The iOS app uses Apple's native CoreMIDI transport and the same editor. Controller-specific compatibility depends on the selected controller mode; automatic identity matching is not a guarantee for every model.

See [BUILDING.md](BUILDING.md) for building and recovery. Release firmware must come from the attested [FM-1 shared builder](https://github.com/zvenson/fm1-build), with all four recovery archives and checksums verified before publication. Provenance and hardware validation are separate checks.

## Credits and licence

Based on [Melodee](https://github.com/keremimo/melodee), by Kerem Kilic (Ellic Studio), pinned to 6a608ba122ac42293908d394e26713e31ec52fb0; Melodee is based on [Felucca](https://github.com/hugelton/Felucca), by Leo Kuroshita (@kurogedelic), Hügelton Instruments. SynthMaven retains applicable Clipper/3dSam-derived work and upstream third-party notices. See [LICENSING.md](LICENSING.md), [LICENSE](LICENSE) and LICENSES for details. Not affiliated with or endorsed by M-VAVE or the upstream authors. Report this build's issues to SynthMaven.

## Retained engines and space budget

Version 3.6 retains DX7/FM6, HYBRID/Vital translations, ANALOG, DIGITAL, VOICE, STRINGS, JUNO, ACID, VPS and WSEQ alongside the Melodee native engines. ACID/VPS/WSEQ add 42 presets. FMX, VECTOR, TERRAIN, WAVE and MACRO are excluded. WSEQ retains its sequences and crossfades with compact oscillator waveforms; it is not a bit-identical port of the older FMX-table oscillator. Four layers and independent drums remain available. The startup icon is losslessly packed without reducing resolution.

The shared editors include named custom-scale saving, compatible Scala import/export, modulation templates, engine-aware filter controls, two-card sound editing and paged Effects/LFO/ARP controls. Output compensation and Bluetooth are excluded. See the illustrated guide for direct USB host and powered-hub wiring, and the separate computer/iPad-hosted editing setup.
