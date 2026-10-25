# SynthMaven stock FM6 voices

`ROM1A.syx` (32 packed DX7 voices) and `ARABESC.syx` (one DX7 single voice) were supplied by the end user for inclusion as stock sounds. Original voice names and operator parameters are retained. These data files are distinct from the firmware source-code licence; the original sound-data rights remain with their respective owners.

`tools/gen_fm6_patches.py` validates Yamaha framing, lengths, 7-bit payloads and checksums before generating the firmware table. A malformed dump fails the build. `--sync-web` updates the editor's matching mock factory data.

Factory patches F1–F8 are unchanged. The 33 additions are factory patches F9–F41 in the FM6 editor. Hardware PRESETS includes all 33 under FM6; the HOME kind list appends a DX7 group. EDIT > PATCH > PTCH loads the same voices. The original B1–B27 parameter IDs (8–34) are preserved; new stock parameter IDs are 35–67. Existing user banks are not overwritten. Single-voice ARABESC is packed from 155 to 128 bytes; its synthesis parameters are preserved.
