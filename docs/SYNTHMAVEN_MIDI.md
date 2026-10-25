# SynthMaven MIDI protocol v15

The existing editor envelope is unchanged: `F0 7D 46 4C command ... F7`. See web/EDITOR_PROTOCOL.md. New commands use 7-bit payload bytes.

- 100 GET: no arguments.
- 101 SET: enabled (0/1), patch source (USB=0, BLE=1, ANY=2), keyboard source (USB=0, TRS=1, BOTH=2), Bluetooth enabled (0/1).
- 102 SCAN: no arguments; queues a 15-second discovery scan when Bluetooth is enabled.
- 103 DEVICES: no arguments; reply `rc, count`, then `index, name` (NUL-terminated, max 31 bytes), `RSSI+127` for each of at most eight peers.
- 104 CONNECT: one device index from DEVICES; explicit selection, no automatic first-peer connection.
- 105 DISCONNECT: no arguments.

GET, SET, SCAN, CONNECT and DISCONNECT reply: rc, schema=1, capability bits, enabled, patch source, keyboard source, Bluetooth enabled, BLE state.

rc=0 success; 1 malformed/range error; 4 unavailable. SET validates all fields before changing any. No unavailable Bluetooth configuration is saved. Capability bits: USB device input=1, TRS input=2, BLE central=4, USB host=8. Native BLE development builds advertise capabilities=7. Non-radio builds advertise 3 and refuse radio actions. BLE states: 0 off, 1 starting, 2 idle, 3 scanning, 4 connecting, 5 discovering/subscribing, 6 MIDI ready, 7 startup failure. Only state 6 indicates a usable central MIDI link.

Input packet cable tags: USB=0, TRS=1, BLE=2. The BLE runtime uses its own 64-packet ring. Program changes are filtered by patch source and PROG. Note-ons are filtered by keyboard source; note-offs remain accepted on either old/new port to release previously held notes. Bank Select state is independent for every input port and channel. MIDI IN channel selection still applies.

New preference record: magic 0x314D5353, patch source, keyboard source, Bluetooth enabled, reserved=0, CRC32. Existing settings record offsets and CRCs are unchanged. PROG remains in the original persistent MIDI flags.

## Native runtime and remaining validation

The native BLE build integrates fm1-base's task/timer shims and BLE MIDI central/peripheral runtime with the JieLi SDK. It scans for MIDI peers, explicitly connects/subscribes and reports actual central state. Radio syscfg is journalled in the previously unused 0xFB000 sector; musical data and existing settings addresses remain unchanged. Writes wait until playback and active voices stop. A power loss during sector rollover can forget radio bonds.

Chocolate original/Plus pairing, reconnection, latency and audio stability still require real hardware validation. Native USB-host operation is not implemented: USB keyboard/pedal connections require an external USB MIDI host. A power bank does not provide that host.
