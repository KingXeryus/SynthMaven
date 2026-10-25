# SynthMaven software instrument

Mac standalone, VST3 and Audio Unit versions of the SynthMaven firmware DSP.
This open-source software is based on Clipper. This desktop project uses the
same 23 synthesis engines and all 449 factory presets, including the FM6
ROM1A/ARABESC bank and the nine XD HYBRID translations. The latter approximate
Korg program settings in the existing HYBRID engine; they do not execute Korg
user oscillator binaries or reproduce analog circuitry exactly.

The original interface has a searchable engine/preset browser, live output
scope, engine controls, amplitude/filter controls, four performance macros,
LFO/envelope and twelve-slot modulation matrix controls, effects, a complete
parameter page and an on-screen keyboard. 147 synthesis parameters are exposed
to host automation. The eight engine parameter meanings follow the selected
engine, with stable host IDs `p118`–`p125`. Hardware connection, radio, storage
and device maintenance settings are omitted.

DX7 32-voice bulk `.syx` files have checksum/7-bit validation and a voice picker.
Imported voices persist in saved `.synthmaven` presets and DAW project state.
Presets and parameter edits are recalled by the host. The desktop interface
currently exposes the firmware's performance parameters, not the web editor's
full per-operator FM6 or deep HYBRID patch editors. Arbitrary Korg user-unit
binaries, hardware flash samples and hardware sequencer projects are not loaded
by this desktop version.

## Install

See [INSTALL.txt](INSTALL.txt). The DMG contains a standard macOS installer
package and corresponding source archive. The package installs the standalone
in `/Applications`, VST3 in `/Library/Audio/Plug-Ins/VST3` and AU in
`/Library/Audio/Plug-Ins/Components`. Restart/rescan your DAW after installation.
macOS 11+; universal arm64/x86_64. Logic Pro/GarageBand use AU; compatible DAWs
such as Ableton Live, Cubase, REAPER, Studio One and FL Studio can use VST3 or AU
according to their format support. AAX/Pro Tools is excluded. Host validation
is evidence of format conformance, not certification of every DAW/version.

`Local-Test` DMGs are ad-hoc signed and **not notarized**. They are development
builds. Apple Developer ID signing/notarization requires a paid membership,
valid signing identities and notary credentials. The development Mac currently
has only an Apple Personal Team. No system security protections are disabled.

## Build

Requirements: macOS, Xcode Command Line Tools, Python 3.12+ with Pillow,
CMake 3.22+ and Ninja. The build uses the GPL v3 route of JUCE 7.0.12, pinned by
SHA-256. No paid JUCE subscription is required for this GPL distribution.

```sh
python3 -m pip install Pillow cmake ninja
./desktop/tools/build-mac.sh
python3 desktop/tools/package-mac.py /absolute/output/directory
```

`PYTHON`, `CMAKE`, `CC` and `DEVELOPER_DIR` can select installed tools.
`prepare.py` downloads/verifies JUCE, applies an app-owned-NSView snapshot patch
for modern macOS SDK compatibility, generates the firmware ROM tables and
creates the isolated desktop C source. The CMake target builds universal Mac
binaries and runs DSP/processor tests. Hardware SDKs/toolchains are unnecessary.
The complete JUCE source is included in the corresponding source ZIP.

Optional release environment variables, supplied by the account holder:

- `SYNTHMAVEN_SIGN_ID`: installed Developer ID Application identity.
- `SYNTHMAVEN_INSTALLER_ID`: installed Developer ID Installer identity.
- `SYNTHMAVEN_NOTARY_PROFILE`: existing `notarytool` Keychain profile.

The packaging script signs/verifies all three bundles, builds a PKG/DMG and,
when all release credentials are present, submits to Apple and staples the
accepted notarization ticket. Credentials and private keys are never bundled.

## DSP and instance isolation

`DSPUnity.c` adapts the existing C engines, voice allocator, modulation, effects
and MIDI processing. Device HAL calls are stubbed and engine jobs run inline.
`contextualize.py` uses Clang's resolved AST declarations/references plus ABI
sizes to relocate mutable global and function-local static data into a
preallocated per-instance arena (276,852 bytes for this revision). Immutable
ROM stays shared. A thread-local current-context pointer selects the instance
on each API entry. No global audio lock, audio-thread heap allocation or
per-block whole-state copy is used in the DSP adapter.

The original DSP runs at 44.1 kHz. Cubic interpolation adapts the host sample
rate. Native control processing occurs every 32 samples; MIDI note timing is
quantized by this control period. Factory Program Change requests are handled
on the message thread, with up to roughly 34 ms dispatch delay. CC0 selects
the program bank; the flat index is `bank * 128 + program` (zero based).
The DAW's tempo is forwarded when available; free-running firmware arpeggiator
phase is not repositioned to the DAW transport timeline.

## Validation

See `VALIDATION.txt`. Tests cover every preset, concurrent/interleaved instance
isolation, all enumeration labels, native DSP equivalence, sample rates and
block sizes, imported DX7 voices, state recall and UI construction. Intel DSP
was also executed through Rosetta. Native Intel hardware and individual DAW
sessions remain separate compatibility tests. AU and VST3 format validators
are run on the development Mac; no hardware firmware is flashed by this work.

License: GPL-3.0-only, preserving all original copyright/third-party notices.
See [THIRD_PARTY.txt](THIRD_PARTY.txt), the root LICENSE and LICENSING.md.
