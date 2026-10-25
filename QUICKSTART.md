# SynthMaven 1.0 — quick start

SynthMaven 1.0 for the M-VAVE FM1 includes **23 engines and 537 factory presets**,
including 41 FM6 voices, nine XD compatibility sounds and 88 melodic Oriental /
Middle Eastern presets. Hardware listening, Bluetooth pedal pairing/reconnect
and audio timing under wireless load remain to be validated.

## Install

Use Chrome or Edge. Connect the FM1 to your computer by USB and save a full
backup from the [web editor](https://kingxeryus.github.io/SynthMaven/webapp/editor/).
Open the [installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/)
and follow its prompts. The splash and About screen identify **SynthMaven 1.0**.
See [BUILDING.md](BUILDING.md) to compile the firmware yourself.

## Find, load and edit sounds

Tap SAVE to open PRESETS. The browser shows four names per page.

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
same controls. Both Chocolate models still require physical validation.

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
