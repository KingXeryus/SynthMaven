# SynthMaven 1.9 — FM1 user manual

Generated from release metadata: firmware **1.9**, protocol **21**, shared synth voice limit **8**.

## Sound presets, programs and library entries

- A **sound preset** is one engine sound and its synthesis settings. Loading one replaces the focused part’s sound. A USER sound-preset slot stores one sound.
- A **four-part program** stores four loaded sounds and their part routing, mix, tuning and layer state. The eight named program slots are device flash storage.
- An **editor library entry** is a computer/browser-side item or imported file. It becomes device state only when sent to a part or saved to a device slot; keeping a library entry does not save a four-part program. Export library files separately from device backups.


SynthMaven is an open-source firmware based on [Clipper by 3dSam](https://github.com/isod89/clipper-fm1).
This manual covers the **M-VAVE FM1 hardware firmware**, its web installer and
web editor. Version 1.9 provides **23 engines and 537 factory presets**. The Mac
software instrument has a separate feature set; its eight-layer interface and
DAW formats are not features of this hardware firmware.

[Quick-start guide](../QUICKSTART.md) · [Preset catalog](release-1.0.md) ·
[Installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/) ·
[Editor](https://kingxeryus.github.io/SynthMaven/webapp/editor/)

## Contents

1. [Before installation](#before-installation)
2. [Install and connect the editor](#install-and-connect-the-editor)
3. [Hardware controls and preset browsing](#hardware-controls-and-preset-browsing)
4. [Factory library and engines](#factory-library-and-engines)
5. [Editing a sound](#editing-a-sound)
6. [Effects](#effects)
7. [Layer, split and performance](#layer-split-and-performance)
8. [Tuning and Scala scales](#tuning-and-scala-scales)
9. [MIDI keyboards and controllers](#midi-keyboards-and-controllers)
10. [USB pedals and patch switching](#usb-pedals-and-patch-switching)
11. [Arpeggiator and sequencer](#arpeggiator-and-sequencer)
12. [Importing sounds](#importing-sounds)
13. [Saving, projects and backups](#saving-projects-and-backups)
14. [Troubleshooting](#troubleshooting)
15. [Version, validation and support](#version-validation-and-support)

## Before installation

Back up the FM1 before installing custom firmware. Bluetooth is removed. Direct USB host mode has software test coverage but still needs Chocolate Plus hardware validation.

### Connection options

| Connection | Role |
|---|---|
| Computer → FM1 | COMPUTER: editor, MIDI, audio and firmware updates |
| USB pedal → FM1 directly | PEDAL: class-compliant USB MIDI, powered adapter/hub required |
| TRS MIDI keyboard → FM1 | TRS or BOTH keyboard input |

The FM1 port cannot act as host and computer device at the same time. This host implementation does not drive 5 V VBUS. Choose a powered adapter/hub with correct host wiring; connector shape alone does not establish compatibility. Hold OCT− during power-on for USB recovery.

## Install and connect the editor

1. Open the [installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/)
   in Chrome or Edge with the FM1 connected over USB.
2. Close other programs using the FM1 MIDI ports if connection fails.
3. Follow the installer prompts and keep power connected until it finishes.
4. Check the splash/About screen for **SYNTHMAVEN 1.9**.
5. Open the [editor](https://kingxeryus.github.io/SynthMaven/webapp/editor/),
   click **Connect**, grant MIDI/SysEx access when prompted and choose the FM1.

Use the matching online editor after updating. The firmware/editor wire protocol
is version **21**; that protocol number is independent of product version 1.9.
A URL ending in `?mock=1` uses a simulated device for previewing the editor.
It does not communicate with or validate a physical FM1.

## Hardware controls and preset browsing

HOME displays the four sounds loaded in the working program. The outline marks the focused sound.

| Control | Action on HOME |
|---|---|
| ALGORITHM | Cycle all 23 engines for the focused sound |
| PRESET | Cycle factory presets within that engine, wrapping at either end |
| SELECT | Cycle focus through sound slots 1–4 |
| KNOB 1–4 | Edit the focused sound’s main parameters |
| SEL tap | Open scale browser; SELECT cycles scales |
| SEL hold 3 seconds | Latch the custom-scale editor |
| SAVE | Open PROGRAM save/load settings |
| GLO hold 2 seconds | Toggle layering ON/OFF once; GLO does not latch |

SAVE → PROGRAM provides **eight named program slots**. SELECT cycles slots 1–8, wrapping at either end; KNOB 1 also selects a slot. ALGORITHM chooses a name character (1–12); PRESET or KNOB 2 changes that character. The complete pending name appears next to the selected slot. Turn LOAD or SAVE twice to confirm. Editing a name does not overwrite the saved program until SAVE completes. Slots 1–4 and 5–8 appear in groups of four on screen.

The redundant PROGRAM MODE, PRESETS and USER pages have been removed from SAVE. Layering ON/OFF remains in **GLO → LAYER PITCH**, on GLO’s two-second hold, and in the editor. Use HOME for engine/preset browsing and the editor for library and layer editing. The SAVE → TOOLS page remains for initialization/clearing operations. All four parts share **8 voices total**. This is not 8 voices per part. Each enabled layer and each unison voice consumes that shared budget; a note using all four parts normally costs four voices, so a two-note chord fills an eight-voice budget. Additional notes can steal existing voices. Release tails and unison also use voices; stolen voices may briefly fade over one audio block.

Function-layer screens first appear at two seconds. Except GLO, function layers remain open after holding their button for three seconds. Release the button and continue editing. Tap the same function button or HOME to exit. REC’s clear action still requires its confirmation hold.

### Oriental / Middle Eastern sounds

The factory library includes 88 melodic Oriental/Middle Eastern sounds. Use the ORIENTAL browser filter or find them inside their engines. See [the collection](../assets/oriental/README.md).

## Factory library and engines

The 537 presets comprise **449 existing factory presets plus 88 new melodic
presets**. The existing set includes **41 FM6 voices** and **nine XD compatibility
sounds**; those are part of the 449, not additional sounds to add to the total.
Original factory preset IDs remain in place. Oriental presets append to their
native engines. Factory sounds are read-only; save edits to a user slot.

| Engine | Starting point for sound design |
|---|---|
| ANALOG | Virtual-analog basses, leads and polysynth tones |
| DIGITAL | Four-operator FM-style tones |
| PHASE | Phase-distortion textures |
| LOFI | Chiptune and reduced-resolution sounds |
| SAMPLE | Sample-based sound shaping |
| VOICE | Formant and vowel-like synthesis |
| TRIO | Chip-style oscillator tones |
| WHEEL | Drawbar organ textures |
| GRAIN | Granular textures |
| FM6 | Six-operator DX7-compatible voices |
| PHYS | Physical-model plucks, strings and resonant instruments |
| NOISE | Noise-based sound design |
| HYBRID | Three oscillators, two filters and a patch modulation matrix; XD translations |
| WAVE | Wavetable synthesis |
| ACID | Acid-style bass and resonant lines |
| STRINGS | String-machine and ensemble tones |
| MACRO | Macro-controlled synthesis models |
| VPS | Vector phase-shaping |
| TERRAIN | Wave-terrain synthesis |
| FMX | Six-operator configurable FM synthesis |
| WSEQ | Wave-sequenced timbres |
| JUNO | Juno-inspired virtual-analog tones |
| VECTOR | Vector-mixed textures |

The [release catalog](release-1.0.md#factory-presets-by-engine) gives exact counts
for every engine. Start with a suitable factory sound and edit it rather than
assuming that changing engines will preserve an equivalent tone.

## Editing a sound

Load a sound preset and tap **EDIT**, or choose a sound from HOME. Hardware pages expose
engine controls and shared sound parameters. Labels vary with the engine.
The editor's Sound tab exposes the available parameters and specialised HYBRID,
FM6 and FMX panels when those engines are selected.

A useful sequence is:

1. Set level and oscillator/model controls for the basic timbre.
2. Set **attack, decay, sustain and release** for the playing style. Short attack
   and decay suit plucks; longer attack/release can suit pads.
3. Adjust filter cutoff/brightness and resonance.
4. Set voice mode and glide for polyphonic chords or monophonic lead playing.
5. Set modulation and controller destinations.
6. Add effects, then save to a USER slot.

The four macros are BRITE, TIMBR, MOTN and SPACE on ordinary engines; HYBRID uses
its patch-defined M1–M4 assignments. The modulation matrix connects sources such
as LFOs, envelopes, velocity or performance controllers to available destinations.
Change one assignment at a time and listen across the keyboard and velocity range.

HYBRID has detailed oscillator/filter/envelope/LFO editing and a dedicated patch
matrix. FM6 exposes operator editing and DX7 import. FMX provides operator and
routing editing. Read from the device before editing an existing patch. With
**Live** enabled, supported panel changes are sent immediately; with Live off,
use the panel's send action when ready. Sound tools include Undo/Redo, Compare,
A/B and scoped randomization; save important versions before broad random edits.

## Effects

Tap **FX** or use the editor to adjust chorus, delay, reverb and available filter,
insert and master processing. A send determines how much of the sound enters an
effect; setting a send to zero removes that sound's contribution to that effect.
Effect type/time/tone controls determine the effect's character.

For a new Oriental sound, begin with the supplied zero chorus/delay/reverb sends.
Raise one send at a time. For a lead, try a small delay/reverb amount before
increasing release; for a plucked sound, retain enough dry signal for definition.
Save the edited sound to a user preset. Shared/master processing can affect the
whole output, including a layered part.

## Layer, split and performance

The four program sounds replace the previous two-part layer workflow. Use GLO → LAYER PITCH → LAYER, or hold GLO for two seconds. OFF plays the SELECT-focused sound; ON plays all four sounds. Selecting a different slot changes editing focus. Note releases follow the sounds that received the note-on. Release held keys before saving a program. Layering increases processing load.

## Tuning and Scala scales

Use **GLO → TUNING** for available tuning-system/root/A4 controls, or the editor's
**Tuning** panel. A4 reference is adjustable from **400 to 480 Hz**. Built-in
systems include equal temperament and alternative tunings, including 46 maqam settings. See the [maqam catalogue](MAQAM_SCALES.md) for
12-key offsets, variants and reference degrees.

To use a Scala file:

1. Connect the editor and open **Tuning**.
2. Load the `.scl` file and review the reported scale.
3. Set the desired root and A4 reference.
4. Use the panel's send action to apply the tuning.
5. Play a known phrase and verify the intended intervals.

Tuning is a system setting independent of factory sound selection. A recipe named
Oud or Ney does not automatically select a maqam or quarter-tone layout. The
Mac instrument's tuning-panel layout and master-tuning toggle are separate from
this firmware's hardware/editor controls.

## MIDI keyboards and controllers

**GLO → KEY MIDI → KEY IN** selects **USB**, **TRS** or **BOTH**. USB means incoming
USB device-mode MIDI from a host. Use the MIDI channel settings to match the
keyboard; OMNI accepts all channels, subject to the selected operating mode.

Supported performance messages include note on/off, note-on velocity, pitch bend,
mod wheel, expression, sustain, supported pressure messages and MPE. Velocity
zero is note-off. Breath CC2 requires the appropriate EWI setting. Proprietary
manufacturer articulation commands and commercial-library keyswitch mappings
are not automatically translated.

| Message | Typical function |
|---|---|
| CC1 | Mod wheel / assigned modulation |
| CC2 | Breath, when enabled |
| CC5 | Glide |
| CC7 / CC11 | Level / expression |
| CC16–19 | Four macros |
| CC64 | Sustain |
| CC71 / CC74 | Resonance / cutoff or brightness |
| CC72 / CC73 / CC75 | Release / attack / decay |
| CC91 / CC93 / CC94 | Reverb / chorus / delay sends |
| CC120 / CC123 | All sound off / all notes off |
| CC121 | Reset controllers |

Bend range can be set in MIDI settings or through RPN 0. MPE provides per-note
bend, pressure and slide on member channels when enabled. Avoid routing the
same notes through two host paths unless that is intentional. Old-source note
releases, sustain-up and safety resets remain accepted after an input change to
help prevent hanging notes. See the [MIDI reference](midi.md) for the complete list.

## USB pedals and patch switching

Bluetooth is removed. For a pedal directly connected to the FM1, choose GLO → USB ROLE → PEDAL and use a powered USB adapter or hub. A class-compliant USB MIDI pedal is required. USB MIDI CONNECTED indicates enumeration; USB DEVICE NOT MIDI means no supported MIDI interface was found. A disconnect releases notes. To return to the editor, choose COMPUTER and reconnect the USB cable. Recovery always uses device mode.

GLO → PATCH MIDI → PROG enables/disables patch switching. The source is fixed to USB. GLO → KEY MIDI independently chooses USB, TRS or BOTH for keyboard notes.

### Assign pedal messages to relative navigation

Open GLO → PATCH UP, PATCH DOWN, BANK UP or BANK DOWN. Set TYPE to CC or PC, NUMBER to 0–127, VALUE to the triggering CC value, and CH to IN CH or an explicit channel 1–16. OFF disables an action. CC release values do not trigger an action unless configured as the trigger. Patch actions wrap within the focused engine; bank actions select the next/previous engine. Save the pedal’s matching configuration using its own configuration tool if required.

Absolute Program Change remains available. Bank MSB 127 addresses physical user-preset slots, with Program Change 0 selecting user slot 1. Other banks retain factory-list mapping. The web editor exposes patch enable and keyboard routing; use the hardware for pedal-message mapping and USB role selection.

## Arpeggiator and sequencer

ARP exposes arpeggiator settings; the Sequencer tab in the editor provides step
editing and tools. Start with a simple phrase before adding timing, probability,
ratchets, chords or modulation. REC records playing into the pattern; the
hardware NOTES view allows step/note editing. Held page-button functions also
provide sequencer layers, so distinguish a tap from a hold.

The web editor can draw/edit notes and performance lanes, copy/paste patterns,
apply generators, and import/export supported pattern data. External clock
requires the appropriate USB or TRS sync setting. A stopped external clock can
make a correctly configured externally synchronized pattern appear inactive.
Stop playback before storage or backup operations when the editor requests it.

## Importing sounds

### DX7 / FM6 SysEx

Select FM6 and use its **Import SysEx** action for `.syx` files. Choose a voice
from a multi-voice bank, review it, and send/load it as offered by the panel.
Keep a source-file copy. Storing an operator-bank voice and saving a playable
user preset are separate actions: save a USER preset referencing the voice if
you want stable pedal access through the user-slot bank.

The supplied ROM1A bank and ARABESC voice are already included among the 41 FM6
factory voices. JSON support remains available for supported editor patch formats;
a DX7 SysEx file is not a universal format for the other engines.

### Korg minilogue xd files

Select HYBRID and open the **Minilogue XD** panel:

1. Import `.mnlgxdprog`, or choose a program from `.mnlgxdlib`.
2. Review the translation and compatibility report with Live disabled.
3. Decide whether to enable the explicitly labelled native digital-oscillator
   substitutions.
4. Use **Send translated patch** to send the patch and amplitude envelope.
5. Save a USER preset if you want to keep it.

This is a native approximation, not a Korg hardware emulator. Korg effects,
motion sequencing, tuning and controller assignments are not reproduced.
`.mnlgxdunit` files can be inspected but their ARM executable code cannot run on
the FM1. See [compatibility details and nine defaults](KORG.md).

### Other files and libraries

Use the relevant engine's import/export controls. The editor's browser-side
library is not automatically the device's nonvolatile preset storage. Send/load
the sound to the device and save it as a USER preset when needed. Retain exported
files outside browser storage, which can be cleared by the browser.

## Saving, projects and backups

SAVE → PROGRAM provides eight persistent named program slots. SELECT chooses SLOT 1–8. ALGORITHM chooses a name character; PRESET or KNOB 2 edits it. Turn SAVE twice to store the current four sounds and name, or LOAD twice to restore a saved program. Release held keys and stop the transport first. An empty slot reports EMPTY SLOT.

Programs contain each sound’s engine, preset identity, edited parameters, modulation sequence and HYBRID/FMX/FM6 patch, plus selected sound, tuning configuration and layer mode. The working program autosaves after an idle period when stopped and quiet. Custom-scale definitions persist in settings and are also included in programs and working-program backups. Loading a program restores its custom scales.

USER sound presets store individual sounds. Export user banks and backups before updates. Older four-track project records remain readable; PRG1 programs from 1.4 upgrade automatically; new PRG2 records require firmware 1.6 or later. Do not overwrite valuable slots while testing custom firmware.

## Troubleshooting

| Symptom | Checks |
|---|---|
| Editor does not connect | Chrome/Edge, USB data cable, MIDI/SysEx permission, correct port; close competing MIDI applications |
| Only highlight changes | Use KNOB 3 to load or KNOB 4 to load and edit |
| Oriental collection missing | Verify SYNTHMAVEN 1.9, refresh the matching editor, select ORIENTAL on the hardware |
| USB keyboard is silent | Confirm a MIDI host exists; separate power alone is insufficient; check KEY IN and channel |
| TRS keyboard is silent | Check MIDI direction, compatible cable/adapter, channel and KEY IN |
| Pedal scans but does not switch | Wait for MIDI READY; enable PROG; check PORT, channel and actual Program Change/CC0 data |
| User program does not load | Save the slot first, use CC0=127, check zero-based program numbering |
| Mod wheel/pressure has no audible effect | Check the patch's modulation assignment and input route |
| Unexpected scale | Check global tuning system, root and A4; sounds do not select maqams automatically |
| Sound continues after release | Check sustain, long release and loops; send CC123 or CC120 for a safety stop |
| Pattern does not start | Check playback state and whether external sync is waiting for clock/start |
| Audio overload/crackles | Reduce dense layering, release tails and effects; report engine/settings and CPU behavior |
| Firmware update fails | Keep the backup; follow installer recovery instructions and use documented recovery tooling |

For recovery tooling, see the [build/install guide](../BUILDING.md#install).

## Version, validation and support

Version 1.9 uses editor protocol 21. See [release notes](release-1.9.md), [USB MIDI status](USB_MIDI_STATUS.md) and [build instructions](../BUILDING.md). USB-host simulations cover direct connections, hubs, disconnect/reconnect and transfer error handling. Hardware validation of direct Chocolate Plus use and audio timing remains outstanding.



## Arabic maqam selection

Choose **GLO → TUNING → TUNE**, select one of **46 maqam settings**, then choose
**ROOT C–B**. The 12-note grid shows each chromatic key’s offset in cents.
Only the displayed nonzero offsets retune keys; all other chromatic notes stay
available. A4 remains adjustable from 400 to 480 Hz. The editor has the same
list and root control. See the [full maqam catalogue](MAQAM_SCALES.md) for every
setting, variant, reference degrees and tuning limitations.

## Custom chromatic scales

Tap SEL to open the scale browser, then turn SELECT to cycle scales. Hold SEL for three seconds to enter the latched editor. SELECT chooses CUSTOM 1–4; PRESET selects ROOT C–B. Each chromatic key press cycles its offset through −25, −50, −75 and 0 cents. Other pitch classes stay unchanged. The offsets repeat across octaves relative to ROOT. Press SAVE and wait for SCALE SAVED; press SEL to leave. These custom scales appear in the tuning list alongside the 46 maqam reference tunings and Scala USER scale.

## Appearance and factory levels (1.6)

Use **Appearance** in the editor or installer to choose Dark, Light or System. The browser remembers this choice. The editor uses the standalone instrument’s system sans-serif font, rounded panels, rotary controls and gridded parameter previews, with the logo’s red accent. Under Sound, choose Sound, Modulation, Effects, Layers, Tuning or All parameters. Drag a knob vertically, use Shift for fine pointer adjustment, use arrow keys when focused, or double-click to reset. Envelope graph handles edit attack, decay, sustain and release. Parameter diagrams are previews rather than live audio measurements.

The 537-entry catalogue has **536 audible, level-matched factory sounds plus USER WT**. USER WT deliberately stays silent until a wavetable is uploaded. Factory matching uses the complete stereo firmware mix and a −29 LUFS reference target on role-appropriate phrases. Playing different notes, changing velocity/effects or layering sounds changes loudness. Saved sounds keep their stored trim; reload the factory preset to use the new calibration.

The Tuning page includes a chromatic keyboard with the selected root and each key’s cents offset. Altered keys use the logo accent. In CUSTOM1–4, click a key or its note button to cycle 0 / −25 / −50 / −75 cents and save the change on the FM1.

## Four-sound layers, splits and mixer (1.6)

Open **Sound → Layers** in the editor. The four mixer strips adapt the standalone instrument’s controls to the FM1’s four loaded sounds. L1–L4 selects the sound to edit; Edit sound opens its Sound controls. **On** enables a slot; **M** mutes it; **S** solos it. Multiple solos can play together. The Pan knob places a sound left, centre or right, and the vertical Level fader balances its volume without changing its calibrated factory trim. Double-click Pan for centre or Level for its factory mixer level.

Turn **Layer program sounds** on to combine the enabled slots. Open a strip’s **LAYER / SPLIT** routing control. Choose **Layer · entire keyboard** for a full-range sound, or **Split · selected zone** for a Lower or Upper zone. Lower includes the split-point note; Upper starts one semitone above it. Different slots can use different split points, so zones can overlap. Disable unused slots for a two- or three-sound combination. The octave −/+ buttons move a slot by 12 semitones; the numeric field allows −24 to +24 semitones. Out-of-range notes stay silent. Changing routing releases the edited layer; live level and pan changes preserve held notes.

With Layer off, SELECT chooses one enabled sound; mute and solo still apply. On the FM1, first SELECT a sound on HOME, then use **GLO → LAYER ROUTE** (On, Split, Upper, Point), **LAYER MIX** (Level, Pan, Mute, Solo) and **LAYER PITCH** (Transpose, Layer on/off). The split point is shown as MIDI note number: 60 is C4. Turn Upper off for Lower.

Save the program to retain all four sounds’ enable states, zones, transpositions, levels, pans, mutes and solos. Working-program autosave retains them too. Firmware 1.6 imports PRG1 programs from 1.4 with every sound enabled across the keyboard, preserving their existing sounds, levels, pans and layer mode. New PRG2 programs cannot be loaded by firmware 1.4.

## Quantize all and master tuning

In SEL, choose ROOT and SCL, then set QNT to **ALL**. The focused sound supplies the scale and root for all four program sounds. Notes from the FM1 keys, USB/TRS MIDI, arpeggiator and sequencer snap down to the next allowed scale note, after each part’s transpose. At MIDI note 0, an unavailable lower pitch snaps upwards instead. Split zones use the original input note. OFF preserves incoming MIDI pitches; SNAP and WHITE keep their existing keyboard mapping. Keep the focus on the sound containing the desired common scale; changing focus selects that sound’s scale settings.

The Layers editor has **Apply master tuning** for each sound, enabled by default. GLO → TUNING → MASTER changes this switch for the HOME-selected sound. Disable it to bypass the master maqam/temperament, custom/Scala tuning and reference A4 for that sound. Pitch quantization, transpose and the global fine-tune control remain independent. Programs save these four switches; settings preserve them across power cycles. Existing programs default to all four enabled. Firmware before 1.6 cannot load a saved program using QNT ALL; export backups before upgrading or downgrading.

Hold GLO for two seconds to toggle layering once, then release. Holding longer does not toggle again. GLO never latches from a long press; other function buttons keep their existing three-second latch behavior.

## Upstream code credits

JUNO, VECTOR, FMX, WSEQ, MACRO, the filter models, TURNG and SPICE / DICE are inherited from **Clipper by 3dSam**. Their instrument and synthesis design inspirations are separate from code authorship. SynthMaven is a derivative of Clipper, which builds on SLOOP and Felucca. Existing copyright and third-party notices are retained; see [licensing and upstream notices](../LICENSING.md).

At two seconds of holding GLO, layering toggles once and the display shows **Layered ON** or **Layered OFF**. Short taps open GLO without flashing this screen. Release GLO to return to the previous screen. Other function buttons retain their three-second hold behavior.

On **SAVE → PROGRAM**, the SELECT knob cycles the saved-program slots, wrapping at either end. Browsing does not load or overwrite a program; use LOAD or SAVE to confirm the action.

## Eight named programs and backups

Program names allow up to 12 printable ASCII characters. The editor’s Backup/Programs page exposes all eight name fields, LOAD and SAVE. Type a name and press SAVE to store it with the current four-sound program. A name edit alone does not write flash.

Two losslessly packed programs share each of the four existing program A/B records: slots 1+5, 2+6, 3+7 and 4+8. Existing PRG1/PRG2 programs in slots 1–4 are imported without moving them; the paired slot starts empty. No sample or wavetable storage is reclaimed. Old groovebox project records must be loaded and saved as a four-sound program before their paired new slot can be used. Corrupt or unknown records are not overwritten by that conversion.

Backups include both programs and their names in each PBK1 object. PBK1 backups require firmware 1.7/protocol 21 or later; the editor refuses to restore them into older firmware. Legacy PRG1/PRG2 backups remain importable. Before downgrading, export a backup and individual sounds; older firmware cannot read PBK1 banks.


### Scale browsing and four-sound mixer (1.8)

Tap SEL to open the scale browser without changing the scale. SELECT cycles the tuning list; PRESET or KNOB 1 changes ROOT, KNOB 2 also browses scales, and KNOBS 3/4 retain QNT/CHORD. Hold SEL for three seconds for custom-scale editing.

EDIT now has four standard pages: EDIT 1, EDIT 2, VOICE and CHARACTER. FMX keeps its operator/routing pages; OP ENV is under ENV. Macros, morph, randomize, undo and FMX STORE remain accessible in the editor. Layer ON/OFF stays in GLO and on its two-second hold.

Hold EDIT for two seconds to preview the four-sound mixer. KNOBS 1–4 control the volumes of sounds 1–4. SELECT cycles the selected part; ALGORITHM sets that part’s split point from A1 to A6 (shown as note and octave), and PRESET/parameter controls its pan. Editing a split point enables the selected part’s existing Lower/Upper zone. GLO → LAYER ROUTE or the editor can choose the zone or restore full-keyboard layering. At three seconds the mixer latches and stays open after release. Tap EDIT or HOME to exit; save a program to keep the mix. The keyboard still plays and EDIT no longer erases sequence notes. Short presses never show a held-layer screen. HOME opens its menu after three seconds.
