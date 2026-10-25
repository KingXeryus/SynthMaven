# SynthMaven for M-VAVE FM-1

![SynthMaven](assets/logo/synthmaven-logo.svg)

SynthMaven is an open-source firmware based on [Clipper](https://github.com/isod89/clipper-fm1), with the SLOOP and Felucca foundations credited in the retained licence notices. It adds a four-patch hardware browser, explicit Load/Edit actions and MIDI input routing. The supplied SynthMaven artwork is used on the device splash and web editor/installer. Original copyright holders and licences are retained.

[Web installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/) · [Web editor](https://kingxeryus.github.io/SynthMaven/webapp/editor/) · [Build status](https://github.com/KingXeryus/SynthMaven/actions)

## Development status

**Not a hardware-validated release. No Bluetooth pedal pairing or native USB host driver is included.** The Bluetooth setting and pairing action report unavailable; the web editor disables them based on device capabilities. Selecting USB keyboard input means USB device-mode MIDI input from a host, not direct support for a USB keyboard plugged into the FM1. A power bank provides power only.

The requested no-computer, no-MIDI-box rig using either Chocolate model over USB/Bluetooth remains unfinished. TRS keyboard input works through the existing UART path. USB MIDI input and Program Change switching can be used with a host. Do not infer direct device compatibility from the presence of a dropdown.

The website build never presents an upstream binary as a SynthMaven build. If target compilation fails, it publishes the editor and a disabled installer with a clear message. Successful CI target builds produce development `.fwsc` packages, not a tested stable release.

## Hardware controls

Tap SAVE to PRESETS. Four names are visible per page (fewer in a short final page).

| Control | Action |
|---|---|
| KNOB 1 / PRESETS | Move highlight without loading |
| KNOB 2 / ALGORITHM | Filter by engine or USER |
| KNOB 3 clockwise | Load highlighted patch |
| KNOB 4 clockwise | Load patch and open EDIT macro pages |

This replaces the synth browser's FAV knob action; existing favourites are retained. HOME browsing and groovebox mode retain their original operation. The web editor provides a matching four-patch user-preset browser with Load/Edit actions. Full DX7 operator editing remains in the web FM6 panel.

## MIDI settings

GLO > PATCH MIDI: PROG enables/disables patch switching. PORT depends on PROG. USB selects USB input; BLE is reserved for an unavailable future BLE-central driver; ANY preserves legacy routing.

GLO > KEY MIDI: KEY IN selects USB, TRS or BOTH. USB in this build needs an external host. Note-offs from the previous input are still processed after changing source, to avoid hanging notes.

GLO > BLUETOOTH exposes BT MIDI and PAIR. **These actions cannot enable or connect Bluetooth in this build.** Firmware refuses unsupported requests, and the editor disables unavailable controls rather than simulating pairing. Original Chocolate and Chocolate Plus have not been tested.

Routing preferences persist in a separately checksummed extension of the existing settings side record. Existing compatible settings keep their existing layout. Firmware protocol v15 commands are documented in [MIDI protocol](docs/SYNTHMAVEN_MIDI.md). Editor settings require SynthMaven v15 and refuse older firmware.

For stable pedal addressing, send Bank Select MSB (CC0) 127 and Program Change data 0 for U01, 1 for U02, etc. Empty/out-of-range slots do not change the sound. Each input port and MIDI channel holds independent bank selection. Factory-list bank mappings remain supported. Saved FM6 bank patches need user presets referencing them to use this user-slot addressing.

## Building and verification

See [BUILDING.md](BUILDING.md) for the JieLi pi32v2 toolchain and AC79 SDK. Linux x86-64 runs the target compiler directly; macOS requires Docker. Target builds use `sh build.sh`; the internal package remains `build/felucca.fwsc` for loader compatibility. The product name reported to USB and the UI is SynthMaven.

```
pip install Pillow
sh tools/test_synthmaven.sh
python3 tools/make_synthmaven_site.py
# After a successful target build:
python3 tools/make_synthmaven_site.py --package build/felucca.fwsc
```

Serve `build/synthmaven-site` with `python3 -m http.server --directory build/synthmaven-site 8767`. Web MIDI requires Chrome/Edge and HTTPS or localhost. `?mock=1` is a simulated editor device, not a hardware connection.

Host validation covers the browser, separate Load/Edit actions, keyboard source filtering, pedal source filtering, patch switching off, settings round-trip/CRC/legacy handling, USB/TRS MIDI parsing, editor protocol and installer mock transfers. Real radio pairing, direct USB-host operation, timing and audio under wireless load are unverified. Back up the current device before any firmware installation.

## Credits and licence

GPL-3.0-only unless stated otherwise in the retained notices. SynthMaven by isod89/3dSam, SLOOP, and Felucca by Leo Kuroshita / Hügelton Instruments provide the foundation. See [LICENSE](LICENSE), [LICENSING.md](LICENSING.md). M-VAVE/FM-1 are their respective owners' trademarks. The supplied SynthMaven logo is branding, separate from third-party code notices.
