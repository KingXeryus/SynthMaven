# Original SynthMaven Oriental/Middle Eastern collection

44 factory entries from 44 melodic instrument recipes, with Dry and
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

Hardware use: HOME ALGORITHM selects an engine and PRESET browses its sounds; SELECT picks one of four parts. The collection now has **44 factory entries**, one per instrument, replacing 88 D/E entries. The editor's regional selector exposes Dry/Expressive variations; both original recipes are preserved in `editor.json`, and `legacy-variants.json` freezes the old IDs and trims. Existing programs retain their complete parameters, and favourites map both old variants to the consolidated sound. Global tuning is unchanged.
