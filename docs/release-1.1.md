# SynthMaven 1.1 — FM1 controls and Bluetooth startup

This hardware firmware release retains **23 engines and 537 factory presets**,
including all 88 melodic Oriental sounds and the dry defaults from 1.0.
The software instrument is developed separately.

## Changes

- Fix the Bluetooth startup callback: the SDK bridge supplies length zero for
  its typed ready event. The old adapter discarded it and never enabled BLE.
  Accept the actual SDK event and recover after a delayed ready notification.
- Distinguish Bluetooth OFF, STARTING and START TIMEOUT in scan feedback.
- Add saved CC/PC assignments for patch up/down and bank up/down, each with
  message number, exact CC value and channel. Routing follows PATCH MIDI and
  works independently of keyboard input. Configure the assignments on hardware.
- Show four patches on HOME. PRESET wraps within the current category bank;
  ALGORITHM changes banks. Retain SAVE → PRESETS for engine filtering/editing.
- Add direct user-manual, quick-start and release-note links to the installer.
  Publish matching readable documentation with the site and source download.
- Make the web editor English-only and remove its language switch.

## Install and use

Back up user data in the [editor](https://kingxeryus.github.io/SynthMaven/webapp/editor/),
then use the [installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/).
GLO → BLUETOOTH: BT MIDI ON, SCAN, select PEDAL, LINK; wait for MIDI READY.
GLO → PATCH MIDI: PROG ON, PORT BLE or ANY. Configure the four GLO action pages
for the messages your pedal sends. See the [quick start](../QUICKSTART.md) and
[user manual](USER_MANUAL.md) for examples, channel handling and status messages.

The firmware/editor protocol remains 16. Existing preset IDs, tuning, routing
records and musical-data flash addresses remain unchanged. Pedal assignments
are a separate appended record with its own magic, range checks and CRC.
Older firmware ignores it; saving settings with older firmware can erase the
new assignments. They default to OFF if absent.

## Validation

The [native BLE target build](https://github.com/KingXeryus/SynthMaven/actions/runs/40247482272)
passes: 580504/581564 bytes flash, 91220/98304 static RAM, 24832/25672 low SRAM,
329008/344064 pool. The 610045-byte package identifies as FM-1_911. All 23 engines
remain; the existing size-optimization fallback is used.

The host regression exercises the real zero-length SDK callback, timeout/late
recovery, input routing, press/release values, PC override, per-bank wrapping,
saved/corrupt/legacy settings and the rendered HOME screen. Editor, Oriental,
Korg and lossless WAVE regression checks are retained. See Bluetooth status for
target build results. Physical Chocolate/Chocolate Plus pairing, reconnect,
power cycles and sustained audio with wireless traffic require device checks.
A successful compilation or simulated MIDI link is not a physical pairing test.

Native USB MIDI host mode remains unavailable; use TRS keyboard input or a USB
MIDI host. The original 1.0 [preset catalog](release-1.0.md) still applies.
