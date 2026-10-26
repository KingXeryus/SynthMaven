# SynthMaven 1.7 — layering screen and eight named programs

Hold GLO to replace the current display with **Layered ON** or **Layered OFF**. At **two seconds**, layering toggles once and the label updates. Holding longer does not repeat. Release GLO to restore the previous page or menu. GLO stays unlatched; other function buttons retain their three-second hold behavior. Layering ON/OFF remains in GLO → LAYER PITCH and in the editor.

SAVE → PROGRAM now offers **eight named program slots**. SELECT cycles slots, wrapping at either end. ALGORITHM selects a name character; PRESET or KNOB 2 changes it. Names allow 12 printable ASCII characters. LOAD and SAVE remain explicit confirmed actions; selecting a slot or editing a name does not overwrite saved data. The editor also exposes all eight names and LOAD/SAVE controls.

PROGRAM MODE, PRESETS and USER are removed from the device SAVE menu. HOME engine/preset navigation, GLO layering controls and editor library/layer controls remain available. SAVE → TOOLS remains available.

The four existing atomic A/B program records now each hold two losslessly packed named programs: 1+5, 2+6, 3+7, 4+8. Each pair is 3,116 bytes and fits the existing record. Existing PRG1/PRG2 programs in slots 1–4 import in place; the added paired slot starts empty. Sample and wavetable flash regions are unchanged. Old groovebox projects must be loaded/resaved as programs before adding their paired slots. Invalid records are refused.

Editor protocol **21** adds command **111**: read `[slot]`, stage a save name with `[slot, twelve zero-padded ASCII bytes]`, reply `[rc, slot, used, twelve name bytes]`. Slot numbers are 0–7. A staged name is committed by PROJECT SAVE. PROJECT now addresses all eight slots and reports an optional operation status. PBK1 backups include all eight programs and their names; older firmware is blocked from restoring the newer format. PRG1/PRG2 backups remain supported.

QNT ALL, default-on per-sound master tuning and corrected Clipper by 3dSam credits remain. The catalogue retains 536 calibrated audible presets and the intentionally silent USER WT slot. Physical FM1 and direct USB pedal validation remain pending.
