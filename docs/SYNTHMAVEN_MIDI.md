# SynthMaven 1.7 MIDI settings — protocol 21

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

Backups use PRG2 records (PRG1 imports from 1.4 remain supported) for the working program and saved program slots. These contain all four parts, patches, tuning, custom-scale definitions, per-part split/transpose/enable state and the solo mask. Older firmware must reject them. Legacy FUN project records remain readable.

USB host mode supports class-compliant full-speed USB MIDI 1.0 and one hub; it does not supply VBUS. See [USB status](USB_MIDI_STATUS.md).

Command **109** (protocol ≥19): read with `part` 0–3; write with `part, enabled, split, upper, splitPoint, transpose+24, level, pan+64, mute, solo`. Boolean values are 0/1; splitPoint is 0–127; transpose+24 is 0–48; level and pan+64 are 0–127. Reply is `rc, part` followed by the same nine fields on success. Invalid requests change nothing. Focus stays unchanged. Lower includes the split point; Upper begins above it. Routing changes release the edited part, while level/pan changes use the existing mixer glide. All keyboard channels continue to follow the global MIDI input setting.

## Master tuning switches (1.6)

Command **110** (protocol ≥20): read `[part]`, write `[part, applyMasterTuning]`; reply `[rc, part, applyMasterTuning]` on success and `[rc, part]` on failure. Part is 0–3 and switch is 0/1. Every sound defaults to 1. Invalid writes change nothing. Programs and settings persist these switches. Command 109 remains unchanged.

SEL QNT value 3 (**ALL**) snaps keyboard, incoming USB/TRS MIDI, arp and sequencer notes across the four program sounds into the focused sound’s ROOT/SCL, following per-part transpose. Original notes determine split routing and note-off ownership. Master tuning bypass independently controls maqam/temperament/Scala offsets and reference A4.

## Eight named program slots (protocol 21)

PROJECT addresses slots 0–7. Command 111 reads `[slot]` or stages a name with `[slot, twelve zero-padded printable ASCII bytes]`. Success returns `[0, slot, used, twelve name bytes]`. Staging does not write flash; PROJECT SAVE commits the name with the sound program. PROJECT status 0 is success, 1 busy, 2 invalid, 4 flash error. PBK1 backup objects 2–5 each contain a pair of programs (1+5, 2+6, 3+7, 4+8); format 9 requires protocol 21. Existing object IDs and sample regions remain unchanged.

## Performance assignments

Current firmware supports ten learned actions: Patch Up/Down, Bank Up/Down, Program Next/Previous, Scene Next/Previous and Setlist Next/Previous. Learn captures CC/PC number, value and channel, then requires Accept; it never replaces a map on capture. GLO diagnostics distinguish USB attachment from a MIDI interface and show match/trigger status. Settings in the editor offers the same controls in COMPUTER mode. Direct pedal learning uses the device in PEDAL mode.

Duplicate suppression defaults OFF. Enable 1–500 ms only when a pedal emits duplicates. A CC release re-arms immediately; PC has no release and intentional identical taps within the chosen interval will also be suppressed. Setlists reference saved program slots; scenes recall compact changes within the loaded program. See [release details](release-2.0.md) and [protocol](EDITOR_PROTOCOL.md).
