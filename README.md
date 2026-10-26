# SynthMaven for M-VAVE FM-1

SynthMaven 2.0 builds on [Clipper by 3dSam](https://github.com/isod89/clipper-fm1) with four-part programs, a layer/split mixer, learned USB pedal controls and an instrument-style visual editor. Its 23 synthesis engines provide 537 factory entries: 536 level-matched sounds and an intentionally empty USER WT slot, including 88 Oriental/Middle Eastern sounds and nine XD compatibility sounds.

[Installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/) · [Editor](https://kingxeryus.github.io/SynthMaven/webapp/editor/) · [User manual](docs/USER_MANUAL.md) · [Quick start](QUICKSTART.md) · [Release notes](docs/release-2.0.md)

## What SynthMaven adds

- **Four-sound layering, keyboard splits and mixing.** Layer enabled parts together or play only the selected part. Each part has its own split zone and point, octave shift, volume, pan, mute and solo. Eight named program slots store four sounds together with their routing, mix and tuning; names can contain up to 12 characters.
- **USB pedal MIDI Learn and switching.** Choose Patch Up/Down, Bank Up/Down, Program Next/Previous, Scene Next/Previous or Setlist Next/Previous; choose Learn, press the pedal, review the captured CC/PC and accept it. Diagnostics distinguish USB device detection from a MIDI interface and show COMPUTER/PEDAL role, the last message, assignment matches and trigger feedback. Configurable duplicate suppression defaults OFF; a CC release immediately re-arms intentional taps. Identical PC taps inside an enabled suppression window are also suppressed, so keep the window shorter than your fastest tap.
- **Compact scenes and setlists.** Four scenes per program capture levels, mutes, effect sends, four macros per part and selected tuning settings, including master-tuning and compensation switches. Save the program to retain its scenes. A global setlist orders up to 32 references to the eight saved programs, including repeated entries, for pedal navigation. Scenes change the current program; setlists load saved programs.
- **Visual editor with the standalone instrument’s look and feel.** Shared controls and graphics include oscillator, envelope, filter and EQ visual previews, a four-part layer mixer, split routing, sound libraries and backups. The editor and installer share the SynthMaven logo palette, matching fonts and Dark, Light or System appearance.
- **Engine-aware filter access and output control.** Model, cutoff, resonance and envelope amount are grouped together; unavailable controls are disabled or omitted. HYBRID retains its own two-filter architecture. Optional estimated output compensation attenuates resonance/drive level changes, up to 12 dB, and defaults OFF; it is not measured loudness normalization. Output trim uses half-decibel steps. See the [filter capability map](docs/FILTER_CAPABILITIES.md).
- **Ready-to-use modulation templates.** Start with velocity-to-brightness, pressure-to-vibrato, wheel-to-vibrato or envelope swell. HYBRID pressure/wheel templates use a free matrix slot and set LFO1 to sine, rate 64; a full matrix is left unchanged.
- **Maqam, custom scales and tuning control.** Select among 46 Arabic maqam reference tunings with any root, four saved custom chromatic scales and Scala imports. Custom scales keep the chromatic keyboard mapping and adjust only the selected pitch classes. Each key press cycles 0 → −25 → −50 → −75 → 0 cents. QNT ALL applies the chosen scale/root to all four parts and incoming MIDI. Apply master tuning is enabled by default and can be bypassed independently per part.
- **Calibrated factory levels.** The 536 audible presets pass the reference audit within ±0.5 LU; USER WT stays silent until a user wavetable is uploaded. The reference measurement method and its limits are documented in the [release notes](docs/release-1.5.md).

These performance, tuning, editor and workflow additions build on the inherited synthesis foundation. JUNO, VECTOR, FMX, WSEQ, MACRO, the filter models, TURNG and SPICE / DICE remain credited to Clipper; their design inspirations do not establish SynthMaven authorship.

## Device navigation and shortcuts

| Control | Function |
|---|---|
| HOME → ALGORITHM | Cycle synthesis engines for the selected part. |
| HOME → PRESET | Cycle presets within the selected engine. |
| HOME → SELECT | Select one of the four program parts. |
| Hold GLO for 2 seconds | Toggle layering once; show **Layered ON** or **Layered OFF**. Release to return. GLO does not latch. Layering also remains available in GLO. |
| Hold EDIT for 2 seconds / 3 seconds | Preview / latch the four-part mixer. KNOBS 1–4 control part volumes; SELECT chooses the part, ALGORITHM sets its split point from A1 to A6, and PRESET adjusts its pan. Tap EDIT or HOME to exit. Choose Lower/Upper or full-keyboard routing in GLO or the editor. |
| Tap SEL | Open the scale browser; SELECT cycles scales, PRESET or KNOB 1 changes ROOT. |
| Hold SEL for 3 seconds | Latch custom-scale editing: SELECT chooses CUSTOM 1–4, PRESET chooses ROOT, keys cycle detuning, SAVE stores the scale. Press SEL to leave. |
| SAVE → SELECT | Browse eight saved program slots. ALGORITHM selects a name character; PRESET or KNOB 2 edits it. |
| GLO → PEDAL LEARN / SCENES / SETLIST | Learn and accept a pedal assignment, capture/recall scenes or edit the program order. |
| Hold OCT− at power-on | Enter USB recovery. |

Short taps do not flash long-press screens. The mixer preview appears after two seconds and latches at three; other function buttons retain their three-second hold behavior. Save a program to keep its mix and scenes.

## Voice budget and USB connections

The four parts share **eight synth voices total**, not eight voices per part. A note played by four layered parts normally consumes four voices, so two such notes fill the budget before unison or release tails. Extra notes can steal existing voices.

Bluetooth is removed. GLO → USB ROLE → **PEDAL** enables direct class-compliant USB MIDI host operation using a powered adapter or hub. Learn a directly connected pedal on the FM1 itself. Return to **COMPUTER** for the web editor, USB audio, backups and firmware updates; the USB port has one role at a time. Direct Chocolate Plus operation and audio timing still require physical hardware validation.

Back up before installing. See [USB MIDI status](docs/USB_MIDI_STATUS.md), the [maqam catalogue](docs/MAQAM_SCALES.md) and the [preset catalogue](docs/release-1.0.md) for details and limits.

## Building and verification

See [BUILDING.md](BUILDING.md). Linux x86-64 runs the JieLi toolchain directly; macOS requires Docker. Run `sh tools/test_synthmaven.sh`, then `sh build.sh --release 2.0`. The build enables USB host support and excludes Bluetooth. Source and licence notices are published alongside the installer.

## Credits and licence

SynthMaven is based on [Clipper by 3dSam](https://github.com/isod89/clipper-fm1), which builds on SLOOP and Felucca. Original firmware work, engine attributions and licence notices are retained in source.

GPL-3.0-only; see [LICENSING.md](LICENSING.md). USB host additions retain their MPL-2.0 notices in firmware/vendor/fm1-usb-host.
