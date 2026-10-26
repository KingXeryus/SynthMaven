# SynthMaven 1.4 release notes

SynthMaven 1.4 keeps 23 engines and 537 factory presets. This release changes the home screen into a four-sound program and removes Bluetooth.

- ALGORITHM selects the engine of the focused sound. PRESET wraps through that engine’s factory presets. SELECT selects sound 1–4 without changing its contents.
- SAVE opens PROGRAM. Choose one of four program slots; turn LOAD or SAVE twice to confirm. PROGRAM MODE selects LAYER OFF (focused sound only) or ON (all four sounds). The global eight-voice budget is shared by all layers.
- Programs retain all four sounds’ parameters, modulation steps and HYBRID/FMX/FM6 patches, tuning selection, focused sound and layer mode. Working programs autosave when stopped and quiet. Custom-scale definitions are stored separately in settings.
- Tap SEL to cycle tuning scales. Hold SEL for three seconds, then release, to edit CUSTOM 1–4. SELECT chooses the custom slot; PRESET chooses ROOT. Press a chromatic key repeatedly for 0 → −25 → −50 → −75 → 0 cents. SAVE stores the scales; SEL exits. All octaves of the same pitch class share the offset relative to ROOT.
- Function layers latch after a three-second hold. Tap a function button or HOME to leave the latched layer. REC retains its destructive clear confirmation behavior.
- Bluetooth radio, scanning and pairing controls are removed. Pedal messages are accepted only over USB.
- GLO → USB ROLE: COMPUTER is USB device mode for the editor, computer MIDI and updates. PEDAL makes the FM1 a USB MIDI host for a directly connected class-compliant pedal. Use a powered USB adapter or hub: this port implementation does not supply 5 V VBUS. No computer is needed in PEDAL mode. Return to COMPUTER before connecting the editor.
- USB recovery remains device-only: hold OCT− at power-on. Do not connect two USB hosts together.
- The installer links to the manual and quick start. The editor is English-only. Protocol 18 adds four parts, four custom scales and USB-host status.

Direct-host support passes simulated enumeration, hubs, transfer ownership, disconnect/reconnect and mode-switch tests. Direct operation with a Chocolate Plus and sustained audio load still require hardware validation. The earlier BLE crash path is removed; this does not establish that all possible hardware faults are resolved. Back up before installing.

The 46 built-in Arabic maqam reference tunings and selectable ROOT remain available. See [maqam catalogue](MAQAM_SCALES.md) for reference intonation and representational limits.
