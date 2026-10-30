# SM FM1 — 3.5

This landscape-only iPad app bundles the same editor and protocol handlers as `web/editor.html`.
It retains the existing SynthMaven CoreMIDI transport, bounded SysEx framing, endpoint
matching, native connection wizard, and supplied app icon. It does not inject the
old SynthMaven 3.0 parameter bindings into the Melodee-based protocol.

Run `python3 ios/generate_project.py`, then `ios/test.sh`. The tests build an unsigned
iOS Simulator application; they do not install firmware or distribute TestFlight.
The current target is version 3.5, build 14, home-screen name **SM FM1**. Both landscape orientations are supported; iPhone and portrait mode are excluded.

## Connecting

1. Set the FM-1 USB role to COMPUTER.
2. Connect a USB data cable, with an adapter or powered hub when required by the iPad.
3. Choose Connect → USB cable. Matching input and output ports are selected together.
4. The editor reads the firmware capabilities before enabling supported controls.

Only the matching 3.5 firmware should be used for migration validation.
A port name such as “Melodee FM-1” is not sufficient evidence of its protocol version.
Direct Bluetooth is excluded from this candidate and has its own later validation gate.

## Learning a controller

Connect both the FM-1 and controller through a powered USB hub. In Settings →
Pedal & MIDI Learn, choose Controller input and select the controller (not the
FM-1). Tap Learn new action, operate the controller, select an action or parameter after capture, then save. Up to 16 learned actions can be relearned or unlearned. Only channel messages are routed; the app does not
forward controller SysEx or MIDI clock. Routing stops on disconnect/background
and releases sustain/notes. Saved assignments remain in the firmware for direct
PEDAL/HOST use later. The first USB-only candidate contains no Bluetooth UI.

Build 14 fixes the truncated MIDI-engine favorites handshake and adds the four-track song arranger: 16 sections, per-track patterns, repeats, copy/reorder/delete and project saving.
