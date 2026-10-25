# Original SynthMaven Oriental/Middle Eastern collection

88 parameter-based presets: 44 melodic instrument recipes, with Dry and
Expressive variations. Percussion is excluded; chorus, delay and reverb sends
are off so users can add effects on the FM1. The frozen reference was exported from
SynthMaven software 0.2's `Oriental & Middle Eastern` collection (software source
commit 96b2a113e133145f9f30ae8dbe18488c50cecfe6). The recipes use the existing native
SynthMaven engines. No Yamaha ESP or TAQS.IM samples or preset binaries are included.
Names describe modeled timbral families rather than exact sample-based reproductions.

`software-0.2.json` records every reference patch's 160 native values and original
base patch. `original-names.json` and `layout.json` capture the original engine
names and track/global descriptor layout. `tools/gen_oriental.py` generates a
compact factory header and matching editor metadata. `editor.json` provides full
names, short OLED names, engine/preset IDs and all 126 synthesis settings.
All data here are original SynthMaven parameter recipes under GPL-3.0-only.

Hardware use: SAVE -> PRESETS -> KNOB 2 ORIENTAL. D/E = Dry/Expressive.
KNOB 1 selects, KNOB 3 loads, KNOB 4 loads and edits. All 88 sounds are also
appended to their corresponding native engine's list; original IDs are preserved.
The web editor provides a collection selector with full names. These sounds do
not automatically change global tuning; choose the desired Scala/maqam tuning.

The compact table shares per-variation and per-instrument columns. It generates
one loaded patch directly into the existing track, with no new audio engine or
additional voice arena. Audio rendering remains the existing engine implementation.
Project recall and all parameter values are verified against the software bank.
The ASan/UBSan test excludes shift-base checks for the existing signed fixed-point
DSP shifts; memory bounds and other undefined-behavior checks remain enabled.
