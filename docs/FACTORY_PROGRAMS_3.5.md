# SynthMaven 3.5 factory programs

The factory bank is captured from the owner’s eight final saved FM-1 programs on 30 October 2026. Only P2 was renamed to **E. Organ**. The earlier generated recipes are superseded.

| Slot | Name | Lead engine | Supporting engines |
|---|---|---|---|
| P1 | DX7 STAGE | FM6 | Analog / Strings / Analog |
| P2 | E. Organ | FM6 | Juno / Hybrid / FM6 |
| P3 | RHODES EP | Digital | Juno / Hybrid / FM6 |
| P4 | DISCO LEAD | Hybrid | Analog / CZ / Prophet |
| P5 | VINTAGE LEAD | Analog | Hybrid / CZ / Analog |
| P6 | LEAD GUITAR | FM6 | FM6 / Digital / VPS |
| P7 | ARAB STRINGS | Strings | Strings / Prophet / Strings |
| P8 | RNB BASS | CZ | Analog / Digital / Analog |

`assets/factory/programs-3.5.bin` retains the eight complete 3556-byte saved records. Its SHA-256 and names are recorded in the adjacent JSON manifest. The generated flash representation omits inactive native-engine payloads and losslessly packs active patches and all saved parameters, effects, splits and scenes. Tests compare the loaded state against the original records.

The ensemble string engine uses four shared-budget units per voice, reflecting hardware measurements with layered chords and independent drums. This changes available polyphony under load, not saved patch values. Do not advertise fixed per-layer polyphony or a -14 LUFS guarantee.

Objective audio and timing checks establish integrity and headroom for the tested performances. They do not establish subjective instrument realism.
