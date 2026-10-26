# SynthMaven 1.2 — Chocolate Plus discovery

Version 1.2 retains all **23 engines and 537 factory presets**, pedal CC/PC
assignments, the English editor and linked manuals.

## Changes

- Accept recognized FootCtrl/Chocolate names as BLE connection candidates when
  they advertise HID rather than the MIDI service UUID. The user's FootCtrlPlus
  was observed advertising service **0x1812** with its name; the 1.1 scanner
  discarded that advertisement because it required the MIDI UUID in advertising.
- Retain GATT MIDI-characteristic validation after connection. A recognized HID
  name is a candidate; it is not automatically treated as a usable MIDI device.
- Distinguish public and random BLE addresses in discovery deduplication.
- Handle enhanced as well as legacy LE connection-complete events in the client.
- Include the HOME SELECT update: SELECT and PRESET load sounds within the
  selected bank, wrapping in either direction; ALGORITHM changes banks.
  SELECT retains its page/recording/visualizer functions outside ordinary HOME.

## Install and connect

Use the [installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/),
then verify SYNTHMAVEN 1.2. On the FM1 use GLO → BLUETOOTH: BT MIDI ON, SCAN,
select FootCtrlPlus and LINK. Wait for MIDI READY before testing pedal messages.
Use PATCH MIDI PROG ON and PORT BLE or ANY. Configure the four GLO action pages
for the PC/CC messages your pedal sends. See the [quick start](../QUICKSTART.md)
and [manual](USER_MANUAL.md). The editor protocol remains 16.

## Evidence and validation

The physical FM1 running 1.1 initializes and scans but returns an empty peer
list. An independent Mac scan observed FootCtrlPlus advertising HID UUID 1812
and the name FootCtrlPlus. This directly reproduces the old discovery filter's
rejection. The real parser now has host tests for that advertisement, original
Chocolate/FootCtrl names, standard MIDI, scan-response name updates, address
types, malformed fields and a full list. Address/undefined-behavior sanitizer
checks pass locally.

Physical GATT discovery/subscription and pedal MIDI on the updated FM1 still
require validation. Detecting the advertisement and a passing build do not
establish that the physical MIDI link works. Prior changes and target results
are recorded in the [1.1 release notes](release-1.1.md) and
[Bluetooth status](BLUETOOTH_STATUS.md).

Read-only Mac GATT inspection subsequently connected to FootCtrlPlus and verified
the standard MIDI service 03B80E5A-EDE8-4B33-A751-6CE34EC4C700 and characteristic
7772E5DB-3868-4112-A1A9-F2669D106BF3 (properties 22). Physical FM1 subscription
and pedal events remain unverified. Status now also recognizes incoming links:
an incoming link without notification subscription shows discovery; a subscribed
link shows MIDI READY. DISCONNECT handles both link directions.
