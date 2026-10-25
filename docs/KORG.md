# Minilogue xd compatibility mode

The HYBRID engine supplies the minilogue-style signal path. It is a native approximation, not a Korg hardware emulator. No ARM user-unit binary is executed on FM1.

Select HYBRID in the editor. Its Minilogue XD panel can initialize a two-VCO/noise patch, import a `.mnlgxdprog`, choose one program from a `.mnlgxdlib`, or inspect a `.mnlgxdunit` manifest. Imported patches are previewed with LIVE disabled. **Send translated patch** applies the HYBRID patch and translated amplitude ADSR together. Ordinary HYBRID editing and JSON export remain available.

Mappings include VCO wave/octave/pitch/shape/level, mixer, noise, sync, ring, cutoff/resonance/drive/key tracking, amp/mod envelopes and LFO targets. Responses and time scales are approximate. Korg VPM and custom oscillators are muted by default. The explicitly labelled native-substitution checkbox replaces VPM with HYBRID FM and custom oscillators with a saw approximation. Korg effects, sequencing/motion, tuning and controller assignments are not reproduced. Custom units need a source-level port; a successful manifest inspection does not install an oscillator.

## Default XD presets

Nine supplied programs are translated into HYBRID factory slots 27–35 (zero based), with hardware preset names beginning XD. The original 27 HYBRID factory slots and its 32 original preset IDs retain their positions; the XD presets append after them. Existing FM6 presets and user banks are unchanged.

| Hardware name | Source program |
|---|---|
| XD 80 BASS | 80s Bass |
| XD CRYSTAL | B-SynthArp |
| XD DARKCHD | Dark Chord, library slot 254 |
| XD RHODES | EP Rhodes22 |
| XD GHOST | Ghost Orgpad |
| XD JUNOPAD | Juno 106 pad |
| XD STRINGS | STR HallStrg |
| XD SYNWAVE | B-SnthWvPad |
| XD MOOG LD | Moog Lead |

Defaults use native digital-oscillator substitutions when needed, so they remain playable. They are approximations and do not include THMoog's executable code or Korg's proprietary effects. `assets/korg/translated.json` records each source entry, converted parameters and compatibility notes. The source program remains identified even where the hardware's name is shorter.

The offsets follow Korg minilogue xd MIDI Implementation TABLE 2, cross-checked with the parameter documentation at https://github.com/oxur/minilogue-xd. Korg's user-unit architecture is documented at https://korginc.github.io/logue-sdk/. Korg/minilogue are their owners' trademarks. SynthMaven is not affiliated with Korg.
