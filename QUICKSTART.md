# SynthMaven 1.5 — quick start

Back up before installing. Open the [installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/) in Chrome or Edge. Connect the FM1 by USB in COMPUTER mode and follow the prompts. For rescue, hold OCT− while powering on.

On HOME, ALGORITHM cycles engines, PRESET cycles that engine’s presets, and SELECT focuses one of the four loaded sounds. KNOB 1–4 edit that sound. The four sounds form a program.

Tap SAVE for PROGRAM. Choose SLOT 1–4; turn SAVE or LOAD twice to confirm. PROGRAM MODE → LAYER OFF plays only the selected sound; ON plays all four, sharing eight voices. Release notes and stop playback before saving.

Tap SEL to cycle tuning scales. Hold SEL for three seconds, then release, to edit CUSTOM 1–4. SELECT chooses the custom slot; PRESET chooses ROOT. Repeated presses of a key cycle −25/−50/−75/0 cents for that pitch class. SAVE stores it; SEL exits. Function layers latch after a three-second hold; tap their button or HOME to exit.

For a pedal directly into the FM1, choose GLO → USB ROLE → PEDAL. Use a powered USB adapter or hub with a class-compliant USB MIDI pedal. This host implementation does not supply 5 V VBUS. Direct Chocolate Plus operation still needs hardware validation. Bluetooth is removed.

Set GLO → PATCH MIDI → PROG ON, then configure PATCH UP/DOWN and BANK UP/DOWN as CC or PC messages. Patch actions cycle presets within the selected engine; bank actions cycle engines. Set KEY MIDI independently for the keyboard.

Choose USB ROLE → COMPUTER before reconnecting the web editor or installer. The USB port supports one role at a time.

Read the [user manual](docs/USER_MANUAL.md), [release notes](docs/release-1.5.md), and [maqam catalogue](docs/MAQAM_SCALES.md).

Choose **Appearance → Dark / Light / System** in the installer or editor. In the editor’s Sound page, use the instrument tabs and rotary controls; drag an envelope handle to edit its shape. The 537-entry bank contains 536 level-matched audible sounds and USER WT, which stays silent until you upload a wavetable.
