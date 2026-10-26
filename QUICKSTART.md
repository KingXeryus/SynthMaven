# SynthMaven 1.2 — quick start

SynthMaven 1.2 for the M-VAVE FM1 includes **23 engines and 537 factory presets**,
including 41 FM6 voices, nine XD compatibility sounds and 88 melodic Oriental /
Middle Eastern presets. Hardware listening, Bluetooth pedal pairing/reconnect
and audio timing under wireless load remain to be validated.

## Install

Use Chrome or Edge. Connect the FM1 to your computer by USB and save a full
backup from the [web editor](https://kingxeryus.github.io/SynthMaven/webapp/editor/).
Open the [installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/)
and follow its prompts. The splash and About screen identify **SynthMaven 1.2**.
The installer links directly to this guide, the full manual and the 1.2 release notes.
See [BUILDING.md](BUILDING.md) to compile the firmware yourself.

## Find, load and edit sounds

HOME now shows four patches from the current category bank, with the loaded
patch highlighted. Turn ALGORITHM to change banks; turn SELECT (or PRESET) to load sounds within
that bank. Both directions wrap around. The four visible names follow the
current page. A final page can contain fewer than four patches.

Tap SAVE to open PRESETS for engine filtering and editing. Its controls are:

| Control | Action |
|---|---|
| KNOB 1 | Highlight a sound |
| KNOB 2 | Choose ALL, an engine, USER or ORIENTAL |
| KNOB 3 clockwise | Load the highlighted sound |
| KNOB 4 clockwise | Load it and open Edit |

ORIENTAL contains 44 melodic instrument recipes, each in Dry and Expressive
variations (88 presets). Short names end in D or E. Percussion is excluded.
Chorus, delay and reverb sends start at zero; add effects with FX or the editor.
Save changes as a USER preset. The [release notes](docs/release-1.0.md) list every
instrument and the preset totals for each engine.

The web editor provides a matching full-name Oriental / Middle Eastern selector.
FM6 includes the supplied ROM1A and ARABESC sounds. HYBRID includes the nine XD
compatibility presets; these approximate the imported Korg programs using native
engines. Custom Korg oscillator binaries do not run on the FM1.

## MIDI and Bluetooth

GLO → KEY MIDI → KEY IN selects USB, TRS or BOTH. Notes, velocity, bend,
modulation, expression, sustain and supported pressure/MPE controls follow that
route. Patch response depends on its modulation assignments.

GLO → PATCH MIDI → PROG toggles patch switching; PORT selects USB, BLE or ANY.
For user presets, send CC0=127 then Program Change 0 for U01, 1 for U02, etc.
Empty slots leave the sound unchanged.

GLO → BLUETOOTH: enable BT MIDI, SCAN, select a PEDAL, then LINK. Only MIDI READY
indicates completed service discovery/subscription. The web editor exposes the
same connection controls. FootCtrlPlus can advertise only HID service 0x1812;
1.2 lists recognized FootCtrl/Chocolate names and verifies the MIDI characteristic
after connecting. A discovered name is a candidate, not proof of MIDI readiness.
The 1.1 update fixed the SDK-ready callback that blocked
Bluetooth startup in 1.0. A zero-length SDK event is valid. OFF, STARTING and
START TIMEOUT are now distinct; a late ready event recovers correctly.
Both Chocolate models still require physical pairing/audio validation.

For relative pedal controls, open GLO → PATCH UP, PATCH DOWN, BANK UP or BANK DOWN.
Set TYPE to CC or PC, NUMBER to the raw MIDI number (0–127), VALUE to the exact
CC press value, and CH to the pedal channel (1–16) or IN CH to follow MIDI IN.
VALUE is ignored for PC. For a pedal that sends 127 on press and 0 on release,
use VALUE 127 to avoid a second action on release. Each action defaults to OFF;
assignments are saved automatically. PROG and PATCH MIDI PORT also apply.
These assignments interpret messages; they do not change the pedal's firmware.
Patch up/down use the same bank-limited wrap as PRESET; bank up/down match ALGORITHM.

USB currently operates in **device mode** and needs a MIDI host. Direct USB
keyboard host mode is not integrated; separately powering a keyboard does not
supply the host function. TRS keyboard plus BLE pedal is the intended
computer-free arrangement. See [USB status](docs/USB_MIDI_STATUS.md) and
[Bluetooth status](docs/BLUETOOTH_STATUS.md).

## Tuning, effects and saving

Use the editor Tuning panel to load Scala `.scl` files or set tuning, root and A4.
A preset does not automatically select a maqam. FX adjusts sends and effect
parameters; Edit opens the engine and modulation controls. Save edited sounds
under USER, and keep full backups before firmware updates.

## Known Bluetooth crash — experimental release

During 1.1 Bluetooth/Chocolate Plus testing, the FM1 crashed and entered USB
rescue; the user restored the original firmware. The cause remains unresolved.
Version 1.2 fixes discovery and link-status reporting, but has not been shown
to fix that crash. Hardware stability and pedal MIDI remain unverified.

