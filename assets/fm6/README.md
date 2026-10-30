# SynthMaven DX7 source voices and performance copies

`ROM1A.syx` (32 packed voices) and `ARABESC.syx` (one single voice) were supplied by the user. These source files are retained unchanged. Sound-data rights remain with their respective owners; the firmware source licence does not replace those rights.

The Melodee-based 3.5 port preserves the existing FM6 preset IDs 0–23 and INIT at 24, then appends these 33 sounds at 25–57. It does not overwrite the original factory bank or user library.

The generated **performance copies** preserve names, algorithms and all attack/sustain/timbre parameters. Each operator's release rate R4 is raised to at least 45 and its final level L4 is set to zero. Several source effects otherwise sustain indefinitely, or keep silent feedback operators allocated after key-up. This adaptation is deliberate: these copies are not byte-identical DX7 archival voices. Import the original SysEx when exact original envelope behavior is required.

`tools/gen_fm6_patches.py` validates Yamaha framing, lengths, 7-bit payloads and checksums before conversion. `tools/sync_synthmaven_editor.py` synchronizes the editor's native mock banks from a freshly compiled `build/host/desc.json`.
