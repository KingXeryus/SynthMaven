# SynthMaven 1.5 MIDI settings — protocol 18

USB is the only pedal-message source. Keyboard input may be USB, TRS or BOTH. The USB host role is independent of MIDI source filtering.

Commands 100/101/106 reply with ten bytes: `rc, 2, caps, patchEnabled, 0, keyPort, 0, 0, usbRole, hostConnected`. Caps: USB device 1, TRS 2, USB host 8; Bluetooth bit 4 is never set. KeyPort: USB 0, TRS 1, BOTH 2. Role: COMPUTER/device 0, PEDAL/host 1.

- 100: no arguments, read routing/status.
- 101: `patchEnabled, patchPort, keyPort, bluetooth`. Send patchPort=0 and bluetooth=0. Bluetooth requests return unavailable. Legacy ANY normalizes to USB.
- 102–105: removed Bluetooth operations; return unavailable. 103 retains its empty-list error shape for old clients.
- 106: `usbRole` 0/1. Switching to host disconnects the computer editor. Select COMPUTER on the FM1 to reconnect.
- 107: custom scale. Read with `slot` 0–3; write with `slot, step[12]`. Each step is 0–3 for 0/−25/−50/−75 cents relative to ROOT. Reply `rc, slot, step[12]` on success. Writes schedule settings persistence.

- 108: no arguments reads program performance mode; one argument 0/1 sets LAYER OFF/ON. Reply `rc, focusedSound, layer`.

Existing pedal mapping commands retain TYPE OFF/CC/PC, NUMBER, VALUE and channel IN CH/1–16. Configure these on GLO → PATCH UP/DOWN and BANK UP/DOWN. Patch changes wrap within the focused engine; bank changes cycle engines. PROG OFF blocks them.

INFO now reports five runtime tracks: four synth sounds and the hidden legacy drum track. Synth editor operations accept parts 0–3. Tuning IDs 0–57 retain their prior meanings; CUSTOM 1–4 use 58–61. Scala USER remains 13.

Backups use PRG1 records for the working program and saved program slots. These contain all four parts, patches, tuning and custom-scale definitions. Older firmware must reject them. Legacy FUN project records remain readable.

USB host mode supports class-compliant full-speed USB MIDI 1.0 and one hub; it does not supply VBUS. See [USB status](USB_MIDI_STATUS.md).
