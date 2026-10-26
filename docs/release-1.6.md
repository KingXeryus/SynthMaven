# SynthMaven 1.6 — quantize all and tuning switches

- SEL → QNT adds ALL. The focused sound’s ROOT/SCL quantizes all four program parts and notes from the keyboard, USB/TRS MIDI, arpeggiator and sequencer. Split routing uses original notes; quantization follows each part’s transpose. OFF, SNAP and WHITE retain their previous behavior.
- Layers adds Apply master tuning for each sound, enabled by default. GLO → TUNING → MASTER edits the focused sound’s switch. Turning it off bypasses master scale offsets and reference A4 for that sound; quantization and global fine tune remain independent.
- Programs save all four tuning switches. Existing programs default to enabled; settings also preserve switches across power cycles. A program using QNT ALL requires firmware 1.6 or later.
- Hold GLO three seconds to toggle layering once. GLO stays unlatched; holding longer does not repeat. Other function buttons retain their long-press latch behavior.
- Editor protocol 20 adds command 110: read [part], write [part, applyMasterTuning], reply [status, part, applyMasterTuning]. Parts are 0–3, switch values 0/1. Invalid writes leave state unchanged. Protocol 19 mixer command 109 remains unchanged.

The 537-entry factory catalogue and calibrated trims remain unchanged: 536 audible sounds and USER WT silent until uploaded. Bluetooth remains removed. Direct USB host and Chocolate Plus hardware/audio validation remain outstanding. Automated host tests and release CI gate deployment; physical hardware testing is still required.
