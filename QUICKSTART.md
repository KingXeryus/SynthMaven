# SynthMaven 2.1.2 — quick start

Back up before installing. Open the [installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/) in Chrome or Edge. Connect the FM1 by USB in COMPUTER mode and follow the prompts. For rescue, hold OCT− while powering on.

On HOME, ALGORITHM cycles engines, PRESET cycles that engine’s presets, and SELECT focuses one of the four loaded sounds. KNOB 1–4 edit that sound. The four sounds form a program.

Tap SAVE for PROGRAM. SELECT chooses SLOT 1–8; ALGORITHM chooses a name character and PRESET or KNOB 2 edits it. Turn SAVE or LOAD twice to confirm. GLO → LAYER PITCH → LAYER OFF plays only the selected sound; ON enables the four-sound mix. Hold GLO for two seconds for the same layering toggle. Release notes and stop playback before saving.

Tap SEL to open the scale browser, then turn SELECT to cycle tuning scales. Hold SEL for three seconds, then release, to edit CUSTOM 1–4. SELECT chooses the custom slot; PRESET chooses ROOT. Repeated presses of a key cycle −25/−50/−75/0 cents for that pitch class. SAVE stores it; SEL exits. Except GLO, function layers latch after a three-second hold; tap their button or HOME to exit.

For a pedal directly into the FM1, choose GLO → USB ROLE → PEDAL. Use a powered USB adapter or hub with a class-compliant USB MIDI pedal. This host implementation does not supply 5 V VBUS. Direct Chocolate Plus operation still needs hardware validation. Bluetooth is removed.

Set GLO → PATCH MIDI → PROG ON, then configure PATCH UP/DOWN and BANK UP/DOWN as CC or PC messages. Patch actions cycle presets within the selected engine; bank actions cycle engines. Set KEY MIDI independently for the keyboard.

Choose USB ROLE → COMPUTER before reconnecting the web editor or installer. The USB port supports one role at a time.

Read the [user manual](docs/USER_MANUAL.md), [release notes](docs/release-2.1.2.md), and [maqam catalogue](docs/MAQAM_SCALES.md).

Choose **Appearance → Dark / Light / System** in the installer or editor. In the editor’s Sound page, use the instrument tabs and rotary controls; drag an envelope handle to edit its shape. The 557-entry bank contains 556 level-matched audible sounds and USER WT, which stays silent until you upload a wavetable.

For layers or splits, open **Sound → Layers** in the editor: turn layering on, enable the sounds you need, choose each slot’s full keyboard or Lower/Upper split zone, then balance Level and Pan. M mutes; S solos. Save the four-sound program to keep this mix. On the hardware, select the sound on HOME and use GLO → LAYER ROUTE / MIX / PITCH.

Set SEL → QNT to ALL to quantize the four program sounds and incoming MIDI using the focused sound’s ROOT/SCL. Each Layers strip offers Apply master tuning, enabled by default. Hold GLO for two seconds to toggle layering once without latching GLO.

At two seconds of holding GLO, layering toggles once and the display shows **Layered ON** or **Layered OFF**. Short taps open GLO without flashing this screen. Release GLO to return to the previous screen. Other function buttons retain their three-second hold behavior.

On **SAVE → PROGRAM**, the SELECT knob cycles the saved-program slots, wrapping at either end. Browsing does not load or overwrite a program; use LOAD or SAVE to confirm the action.

Eight program slots are available. On SAVE → PROGRAM, SELECT chooses a slot, ALGORITHM selects a name character, and PRESET or KNOB 2 changes it. Names are up to 12 characters and are committed with SAVE. The editor provides eight name fields and LOAD/SAVE buttons. Layering ON/OFF remains in GLO → LAYER PITCH and on GLO’s two-second hold. PROGRAM MODE, PRESETS and USER are removed from SAVE.


### Scale browsing and four-sound mixer (1.8)

Tap SEL to open the scale browser without changing the scale. SELECT cycles the tuning list; PRESET or KNOB 1 changes ROOT, KNOB 2 also browses scales, and KNOBS 3/4 retain QNT/CHORD. Hold SEL for three seconds for custom-scale editing.

EDIT now has four standard pages: EDIT 1, EDIT 2, VOICE and CHARACTER. FMX keeps its operator/routing pages; OP ENV is under ENV. Macros, morph, randomize, undo and FMX STORE remain accessible in the editor. Layer ON/OFF stays in GLO and on its two-second hold.

Hold EDIT for two seconds to preview the four-sound mixer. KNOBS 1–4 control the volumes of sounds 1–4. SELECT cycles the selected part; ALGORITHM sets that part’s split point from A1 to A6 (shown as note and octave), and PRESET/parameter controls its pan. Editing a split point enables the selected part’s existing Lower/Upper zone. GLO → LAYER ROUTE or the editor can choose the zone or restore full-keyboard layering. At three seconds the mixer latches and stays open after release. Tap EDIT or HOME to exit; save a program to keep the mix. The keyboard still plays and EDIT no longer erases sequence notes. Short presses never show a held-layer screen. HOME opens its menu after three seconds.

## Performance controls

In GLO → PEDAL LEARN choose an action, arm Learn, press a pedal, then Accept the displayed message. Duplicate suppression defaults OFF; CC release re-arms immediately. Direct pedal learning uses the device in PEDAL mode; return to COMPUTER for the editor, audio, backup or installation. GLO SCENES captures/recalls four compact scenes within the current program; save the program afterward. GLO SETLIST orders up to 32 references to the eight saved programs. The editor supports scene Capture/Recall/Clear and setlist reordering. EDIT FILTER groups only controls supported by the engine; optional compensation is estimated attenuation and defaults OFF.

Eight voices are shared across all four parts. A four-part layered note uses four voices, leaving two such notes before release tails or unison. A sound preset, four-part program and editor library entry are separate kinds of storage. See the [user manual](docs/USER_MANUAL.md) for details.

### Vital files and regional variations

Use **Sound → Vital preset import** to read a `.vital` file locally, inspect the compatibility report and explicitly send an approximate HYBRID sound. Save the full custom sound in the editor library or a four-part program; parameter-only user slots cannot keep arbitrary HYBRID bytes. [Supported conversions](docs/VITAL_IMPORT.md).

The regional bank has 44 selectable sounds, with Dry/Expressive variations in the editor instead of duplicate factory entries. The catalogue has 557 entries: 556 audible sounds plus the intentionally empty USER WT.

## Vital-derived factory sounds

The development bank adds **64 native HYBRID translations** selected from 68 supplied Vital presets, for **557 factory entries: 556 calibrated audible sounds plus the intentionally silent USER WT**. These are approximations using the FM1 engines, not Vital playback. Unsupported effects, wavetable motion and modulation can change the sound substantially.

On a fresh start or reset, the four program parts contain **V01 STROBE, V02 DARK, V03 REESE and V04 PLUCK**, with the first part selected and layering off. Existing autosaved work takes precedence. HOME navigation is unchanged: ALGORITHM selects the engine, PRESET cycles its sounds, and SELECT chooses one of the four parts. Within HYBRID, the 64 new sounds come first, followed by the existing HYBRID sounds in the same cycle. Existing preset IDs remain stable.

Selection uses compatibility checks and native waveform spectral fitting, with no duplicate translated parameter sets. Listening comparison and device validation remain pending. See the [selection record](assets/vital/README.md) for source attribution and conversion limits.

## Chord controls in 2.1

Use the existing SEL page CHORD control to choose a legacy or new chord mode. SEL 2 provides VOIC and STRUM. AUTO follows in-scale harmony; AUTO2 places harmony below your melody. When microtonal master tuning applies, the selector offers OPEN 5 and NEUTRAL instead; incompatible saved AUTO modes play OPEN 5. See [the current chord reference](docs/release-2.1.2.md). Hold ENV for three seconds to open CHORDS, with CHORD, VOIC and STRUM on the first three assignable knobs. Release ENV to keep editing; tap ENV for envelopes or HOME to return. The chord screen appears only after the full three seconds.

## Scale-aware chords in 2.1.2

Chord mode plays all physical keys, including black keys. Generated tones follow the selected scale and root; named shapes adapt to the scale. Maqam chords use the catalogue’s documented degree keys with existing tuning offsets. Microtonal tuning offers OFF, OPEN 5 and NEUTRAL; other templates are hidden until tuning is equal-tempered or bypassed for that part. OPEN 5 selects scale tones near a fifth and octave; NEUTRAL selects scale tones near a neutral third and fifth, using actual tuned pitch. Neither inserts new detuned notes. If an older saved mode is incompatible, OPEN 5 is heard without overwriting its saved ID. Hold ENV for three seconds to open CHORDS.
