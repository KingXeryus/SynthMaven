# SynthMaven 1.0 — FM1 user manual

SynthMaven is an open-source firmware based on [Clipper](https://github.com/isod89/clipper-fm1).
This manual covers the **M-VAVE FM1 hardware firmware**, its web installer and
web editor. Version 1.0 provides **23 engines and 537 factory presets**. The Mac
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
10. [Bluetooth pedals and patch switching](#bluetooth-pedals-and-patch-switching)
11. [Arpeggiator and sequencer](#arpeggiator-and-sequencer)
12. [Importing sounds](#importing-sounds)
13. [Saving, projects and backups](#saving-projects-and-backups)
14. [Troubleshooting](#troubleshooting)
15. [Version, validation and support](#version-validation-and-support)

## Before installation

Back up the currently installed firmware's user data before updating. Use its
matching editor to save a full backup, and keep a copy outside the browser.
Use reliable power during installation. Start listening with the output volume
low: envelope, resonance and modulation edits can change perceived loudness.

Version 1.0 passes host tests and compiles with native Bluetooth and all engines
within the FM1 memory limits. Listening tests, pairing/reconnect with both
Chocolate models and audio timing under wireless load remain outstanding.

### Connection options

| Connection | Current operation |
|---|---|
| Computer USB → FM1 | USB MIDI device input, web editing and installation |
| MIDI keyboard → FM1 TRS MIDI input | Keyboard input; use a compatible MIDI cable/adapter |
| BLE MIDI pedal → FM1 | Native scan, select and link controls; hardware validation pending |
| USB keyboard directly → FM1 | USB host mode is not integrated; requires a MIDI host |
| Power bank → FM1 | Power only; it does not provide a USB MIDI host |

A separately powered USB keyboard still normally needs a host to communicate
with the FM1. The intended computer-free arrangement for this build is a TRS
MIDI keyboard with a BLE pedal. Do not select adapters solely by connector shape;
check the MIDI wiring requirements of the devices being connected.

## Install and connect the editor

1. Open the [installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/)
   in Chrome or Edge with the FM1 connected over USB.
2. Close other programs using the FM1 MIDI ports if connection fails.
3. Follow the installer prompts and keep power connected until it finishes.
4. Check the splash/About screen for **SYNTHMAVEN 1.0**.
5. Open the [editor](https://kingxeryus.github.io/SynthMaven/webapp/editor/),
   click **Connect**, grant MIDI/SysEx access when prompted and choose the FM1.

Use the matching online editor after updating. The firmware/editor wire protocol
is version **16**; that protocol number is independent of product version 1.0.
A URL ending in `?mock=1` uses a simulated device for previewing the editor.
It does not communicate with or validate a physical FM1.

## Hardware controls and preset browsing

HOME displays the current sound and its engine. Tap the labelled page buttons
such as EDIT, FX, ARP, GLO and SAVE to reach their settings. Some buttons also
have held sequencer functions; tap when the intention is page navigation.

Tap **SAVE** to reach **PRESETS**. Four names are visible at once, except at the
end of a short list. Highlighting a sound does not load it.

| PRESETS control | Action |
|---|---|
| KNOB 1 | Move the highlight |
| KNOB 2 | Filter ALL, an engine, USER or ORIENTAL |
| KNOB 3 clockwise | Load the highlighted sound |
| KNOB 4 clockwise | Load the sound and open Edit |

The engine filter includes that engine's appended Oriental presets. USER shows
saved user presets. XD compatibility sounds are identified by their **XD** names
inside the HYBRID list. The web editor also offers Load/Edit actions.

### Oriental / Middle Eastern sounds

Select **ORIENTAL** with KNOB 2. The collection has **44 melodic instrument
recipes × two variations = 88 presets**. It includes plucked strings, winds,
regional strings, reeds and leads. No percussion is added in this collection.

The short hardware name ends in **D** for Dry or **E** for Expressive. Both start
with chorus, delay and reverb sends at zero. Add the effects you want from FX.
The Expressive variation changes the performance/tone recipe rather than
providing an additional hall or live-effects version.

In the editor, use the **Oriental / Middle Eastern** selector for full names.
These are SynthMaven synthesis recipes using its native engines, not commercial
Yamaha ESP or TAQS.IM samples. [See every instrument](release-1.0.md#melodic-instrument-catalog).
Choosing a sound does not choose its tuning; set the desired maqam separately.

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

Load a preset and tap **EDIT**, or use KNOB 4 in PRESETS. Hardware pages expose
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

The hardware supports a main sound plus a second instrument part through the
**LAYER** controls. Available modes include OFF, LAYER, SPLIT and MULTI. Choose
the second sound, balance its level, and adjust octave/pan as needed. SPLIT uses
a split point; MULTI uses separate MIDI-channel operation. This is distinct from
the eight independent engine layers in the Mac software instrument.

In the voice controls, select POLY for chords or MONO/LEGATO for appropriate
lead phrasing. **GLIDE** controls portamento time. Modulation response depends
on the loaded patch: a received mod-wheel or pressure message only changes the
sound through its assigned response. Dense layers, long releases and heavy
effects increase voice and processing demand; reduce them if performance suffers.

## Tuning and Scala scales

Use **GLO → TUNING** for available tuning-system/root/A4 controls, or the editor's
**Tuning** panel. A4 reference is adjustable from **400 to 480 Hz**. Built-in
systems include equal temperament and alternative tunings, including Rast and
Bayati choices.

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

## Bluetooth pedals and patch switching

Patch switching has its own route, independent of KEY IN:

- **GLO → PATCH MIDI → PROG**: enable or disable MIDI patch changes.
- **PORT**: select USB, BLE or ANY when patch switching is enabled.

To attempt a BLE MIDI pedal connection:

1. Enable the pedal's documented Bluetooth MIDI mode.
2. On the FM1, open **GLO → BLUETOOTH** and enable **BT MIDI**.
3. Start **SCAN**, then select the intended **PEDAL** from the discovered list.
4. Use **LINK** to connect. Wait for **MIDI READY**.
5. Set PATCH MIDI to BLE or ANY and test one patch change at a time.

LINK also provides disconnect control. A device appearing in a scan is not yet a
working MIDI link: discovery/subscription must finish. The editor provides the
same controls and actual link status. Original Chocolate and Chocolate Plus
pairing, reconnect and sustained performance are still hardware-validation items.

For stable user-slot addressing, program the pedal to send **Bank Select MSB
CC0=127**, followed by **Program Change 0 for U01, 1 for U02**, and so on. Some
pedal applications display program numbers starting at 1 although MIDI data
starts at 0; verify the actual transmitted data. Empty or out-of-range user slots
do not load a sound. Save a factory/FM6 sound as a user preset before addressing
it this way. Bank state is separate for each input port and MIDI channel.

Turning PROG off disables patch switching while allowing keyboard performance
through KEY IN. Native USB host mode is not enabled by choosing USB in this menu.

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

Factory presets are starting points. To preserve an edit on the hardware, open
**SAVE → USER**, choose a slot with KNOB 1, and use **KNOB 4 / SAVE**. On that
page, KNOB 2 loads and KNOB 3 erases; check the slot before using either erase or
save. Saving into an occupied slot replaces that user preset.

Projects capture a broader instrument state. The firmware's project recall tests
verify that appended Oriental preset IDs and all 126 track parameters survive
recall. A project and an individual preset serve different purposes; keep full
backups as well as important individual sounds.

Use the editor **Backup** tab to save a complete backup before firmware changes.
Keep the file in a named folder with its firmware version and date. Restore with
a compatible editor/firmware and follow its stopped-playback prompts. Do not
interrupt a flash write or unplug during installation/restore. Radio bond data
uses separate storage; a radio-journal rollover interrupted by power loss can
forget bonds, requiring another connection attempt.

## Troubleshooting

| Symptom | Checks |
|---|---|
| Editor does not connect | Chrome/Edge, USB data cable, MIDI/SysEx permission, correct port; close competing MIDI applications |
| Only highlight changes | Use KNOB 3 to load or KNOB 4 to load and edit |
| Oriental collection missing | Verify SYNTHMAVEN 1.0, refresh the matching editor, select ORIENTAL on the hardware |
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

The release uses 23 engines and 537 factory presets. The product version **1.0**
is separate from editor protocol **16** and retained hardware-loader identifiers.
The automated checks cover preset values, browser indexing, MIDI response,
project recall, input routing, storage, editor transfers and target memory bounds.
They do not establish real pedal pairing or real-time audio quality on the FM1.

Report issues at [SynthMaven Issues](https://github.com/KingXeryus/SynthMaven/issues).
Include firmware version, engine/preset, connection route, steps to reproduce,
and the actual pedal/keyboard messages where possible. Do not send private
account credentials. Source and retained licences are available in the repository
and alongside the installer. See [release notes](release-1.0.md),
[Bluetooth status](BLUETOOTH_STATUS.md) and [USB status](USB_MIDI_STATUS.md).
