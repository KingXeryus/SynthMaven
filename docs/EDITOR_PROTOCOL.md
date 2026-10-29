
## Protocol 22 performance additions

112 PEDAL: `[0]` status, `[1,action]` Learn, `[2]` Accept, `[3]` Cancel, `[4,action]` mapping read, `[4,action,type,number,value,channel]` write, `[5,msLo7,msHi7]` repeat 0–500 ms, `[6,role]` role 0 COMPUTER/1 PEDAL. Actions 0–9: patch up/down, bank up/down, program next/prev, scene next/prev, setlist next/prev. Type 0 disabled/1 CC/2 PC; channel 0 follows input or 1–16 explicit. Mapping reply `[rc,action,type,number,value,channel]`. Status `[rc,role,deviceDetected,midiInterfaceDetected,statusNibble,channel,number,value,matchedActionOr127,result,learnState,learnAction,candidateType,candidateNumber,candidateValue,candidateChannel,repeatLo7,repeatHi7,triggerCountLo7,triggerCountHi7,actionRc,pendingProgramOr127]`. Result 0 unmatched/1 triggered/2 duplicate/3 learning/4 patch disabled/5 queue full. Learn state 0 idle/1 waiting/2 captured. Accept rc2 indicates conflict. Changing to PEDAL removes the computer interface; return via device GLO → USB ROLE.

113 SCENE: `[0]` returns `[rc,validMask,currentOr127]`; `[1,slot]` capture; `[2,slot]` recall; `[3,slot]` clear; `[4,slot]` raw get; `[5,slot,pack7(raw48)]` raw put. Other replies `[rc,slot]`; get appends packed raw48. Slot 0–3. Raw bytes: valid, tuning system, root, signed A4 offset, master-bypass mask, compensation mask, two reserved zeros; then four ten-byte parts: level,mute,dist,chor,dly,rev,macro1..4. Signed macro values are encoded relative to their native minima. Capture/put is working state until program save.

114 SETLIST: `[0]` read; `[1,length,programRefs...]` replace (0–32 entries, slots 0–7); `[2,direction]` next0/prev1; `[3,entry]` load. Reply `[rc,length,positionOr127,programRefs...]`. References are validated before replacing the list. Program loads are deferred until transport and voices are quiet, not performed inside the MIDI parser.

115 FILTER CAPS: empty/current or `[engine]`, reply `[rc,engine,kind,cutParamPlus1,resParamPlus1,envParamPlus1,driveParamPlus1,architectureString]`. Zero param means unavailable. Kind 0 none/1 fixed/2 multi/3 HYBRID/4 TRIO.

116 FILTER: `[0]` read; `[1,field,valueLo14,valueHi14]` native descriptor write; `[2,template]` template 0–3 (velocity brightness, pressure vibrato, wheel vibrato, envelope swell). Reply `[rc,engine,sevenDescriptors...]`. Fields model/cutoff/resonance/envelope/drive/compensation/trim. Each descriptor: signed14 value,min,max (offset8192), format byte, zero-terminated label, formatted value, unit, nameCount, zero-terminated enum names. Equal min/max means read-only. Native ranges/units are authoritative.

PRG3 (`0x33475250`) and PBK2 (`0x324b4250`) add four 48-byte scenes per program. Backup format10 requires protocol22; PRG1/2 and PBK1 remain importable. Global PRF1 settings append duplicate interval, setlist and six new action maps without changing existing records or flash sectors. See release metadata for current protocol identity.

## Protocol 23: tuning octave boundary

TUNE_GET (95) appends `from_octave` (0..8, C0..C8) after A4. TUNE_PUT (96) accepts the same optional trailing byte after A4. Base and A4-only writes remain valid and preserve the current boundary. Invalid octave values reject the complete write without changing state. MIDI C0 is 12; the inclusive boundary is `12 * (from_octave + 1)`. Scene byte 6 (previously reserved) also stores 0..8; byte 7 remains zero. Scene size stays 48 bytes. The low nibble of the program bypass byte retains its four bypass flags; the high nibble stores the octave. Flash record sizes stay unchanged. Settings MTU2 retains the 12-byte MTU1 record layout, encoding the octave in bits 4..7 of its bypass field. Legacy records imply C0.

### 117 — USB controller identity and override (protocol 23)

GET payload empty or `[0]`; SET `[1, protocol, port]`, protocol 0 AUTO / 1 MIDI / 2 MCU / 3 HUI, port 0 AUTO or 1–16. Reply: result, connected, override, port override, active protocol, DAW cable (0–15 or 127 none), VID and PID (each three little-endian 7-bit bytes), product NUL string (max 47), family NUL string (max 23). Invalid writes are rejected atomically. Physical host and computer roles are mutually exclusive.

### 118 - read-only factory program recall (protocol 24, firmware 2.2)

Payload `[slot]`, slot 0-7. Reply `[result, slot]`; 0 success, 1 invalid payload/slot, 2 invalid factory record, 3 transport or sounding voices busy. Loads the complete ROM factory program into the working program, preserving custom-scale definitions and leaving saved program objects untouched. The current program can later be saved explicitly through the existing save flow. All older command IDs and PRG3/PBK2 record formats remain unchanged.

Protocol 24 PRG3/PBK2 sound flags retain bits 0 (filter compensation) and 1 (stable preset ID). Bits 2–7 encode 0 = legacy unity; 1–63 = calibrated unity through +62 dB in whole-dB steps. A newly loaded sound clears this program-specific calibration. Record sizes/addresses are unchanged. Matching-editor backup restore rejects 2.2 backups on protocol <24 before writing any object.

### Development: program tuning policy

The `firmware-ui-tuning` branch retains program record sizes and uses previously reserved bits 4–5 of `program_t.reserved[0]` / `program_packed_t.solo`. Low four bits remain the solo mask. `0x10` means follow current global tuning; `0x20` means restore saved tuning; `0x30` and upper bits are invalid. Zero is legacy: non-equal scale, non-zero A4 offset or FROM octave imply saved tuning; otherwise global tuning is retained. New captures explicitly write one policy. The GLO TUNING 2 SAVE control is virtual parameter 102; LFO FILTER uses virtual IDs 103–106. There is no audio DSP or wire-message framing change. These new program records are not backward-readable by released firmware; assign a new release/protocol identity before distribution.
