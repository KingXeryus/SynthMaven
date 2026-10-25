# SynthMaven MIDI protocol v15

The existing editor envelope is unchanged: `F0 7D 46 4C command ... F7`. See web/EDITOR_PROTOCOL.md. New commands use 7-bit payload bytes.

- 100 GET: no arguments.
- 101 SET: enabled (0/1), patch source (USB=0, BLE=1, ANY=2), keyboard source (USB=0, TRS=1, BOTH=2), Bluetooth enabled (0/1).
- 102 PAIR: returns unavailable in this build; no scan or connection is attempted.

All three reply: rc, schema=1, capability bits, enabled, patch source, keyboard source, Bluetooth enabled, BLE state.

rc=0 success; 1 malformed/range error; 4 unavailable. SET validates all fields before changing any. No unavailable Bluetooth configuration is saved. Capability bits: USB device input=1, TRS input=2, BLE central=4, USB host=8. Current capabilities=3; BLE state=0/unavailable. No peer identity is reported because no driver is running.

Input packet cable tags: USB=0, TRS=1; BLE=2 is reserved, not implemented. Program changes are filtered by patch source and PROG. Note-ons are filtered by keyboard source; note-offs remain accepted on either old/new port to release previously held notes. Bank Select state is independent for every input port and channel. MIDI IN channel selection still applies.

New preference record: magic 0x314D5353, patch source, keyboard source, Bluetooth enabled, reserved=0, CRC32. Existing settings record offsets and CRCs are unchanged. PROG remains in the original persistent MIDI flags.

## Native connection work still required

FM1 hardware has a radio, but SynthMaven does not initialize it. A working no-extra-box pedal connection requires a BLE central driver, MIDI service discovery, subscribe/reconnect/disconnect, device selection/bond persistence, timestamp/running-status decoding and audio-ISR-safe queues, validated with both Chocolate models. Native USB-host operation additionally needs verified USB role/power support, enumeration and class-compliant MIDI-host endpoints. UI switches cannot supply those drivers.
