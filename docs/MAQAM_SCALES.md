# Arabic maqam reference tunings — SynthMaven 1.4

The firmware includes **46 named maqam settings**, selected with **GLO → TUNING →
TUNE**, alongside **ROOT C–B** and **A4 400–480 Hz**. The editor exposes the same
list. The hardware displays the full selected name and a 12-note offset grid.
Offsets in that grid are cents relative to ordinary equal temperament at the
selected A4 reference.

## Twelve chromatic keys, not seven remapped keys

Every MIDI pitch class stays on its chromatic key. Only the specified keys get
microtonal offsets; all other notes remain available. The pattern repeats every
12 keys and ROOT rotates it to all 12 tonics. This does not quantize notes or
move successive keyboard keys through a seven-note scale.

- Rast, ROOT C: E and B are lowered 50 cents; C, C#, D, D#, F, F#, G, G#, A and A# remain unchanged.
- Rast, ROOT D: F# and C# are lowered 50 cents; the root D stays unchanged.
- Bayati, ROOT D: E is lowered 50 cents; all other chromatic keys stay unchanged.
- Hijaz, ROOT D: play D, E-flat, F#, G, A, B-flat, C. Its reference tuning needs no quarter-tone offsets. It still has its own named entry.

ROOT is the sounding tonic at its ordinary equal-tempered pitch. For maqamat
traditionally rooted on a half-flat note (such as Sikah and Iraq), the interval
pattern is normalized to that chosen tonic. For example, Sikah ROOT C has
relative degrees 0, 150, 350, 550, 700, 850 and 1050 cents. It microtunes D, E,
F#, A and B by -50 cents, leaving C unchanged. Choosing ROOT E means an E-natural
tonic with the same intervals, rather than silently lowering the selected root.

The tuning is global and independent of sound selection. Existing settings
storage saves the system, root and A4; existing system IDs 0–13, including RAST,
BAYATI and Scala USER (13), are preserved. New maqam IDs are appended. Protocol
17 identifies the expanded list; the editor offers only the original list to
older firmware. Loading a Scala file still selects USER 13.

## Reference catalogue

The notes below are **relative to ROOT C**. Rotate every note name by the ROOT
setting. “None” means there is no microtonal adjustment; play the maqam's stated
chromatic degrees. Several maqamat intentionally share the same offsets. Their
melodic course, register, modulation and choice of upper jins distinguish them.

These are fixed reference realizations, mainly using 50-cent half-flats. Arabic
performance is not a single fixed equal-tempered system. Jiharkah and Sikah
Baladi are explicitly approximate. A repeating 12-key map cannot express a
note that changes by octave, ascending versus descending direction, or melodic
context: separate listed variants cover some of those choices, while other
chromatic notes remain playable. This is a broad catalogue, not a claim to
encode every regional or historical maqam realization.

| Setting | Adjusted chromatic keys at ROOT C (cents) | Reference degrees (cents from tonic) | Notes / source |
| --- | --- | --- | --- |
| Rast | E -50; B -50 | 0, 200, 350, 500, 700, 900, 1050 | [MaqamWorld](https://www.maqamworld.com/en/maqam/rast.php) |
| Bayati | D -50 | 0, 150, 300, 500, 700, 800, 1000 | [MaqamWorld](https://www.maqamworld.com/en/maqam/bayati.php) |
| Ajam | None | 0, 200, 400, 500, 700, 900, 1100 | [MaqamWorld](https://www.maqamworld.com/en/maqam/ajam.php) |
| Ajam Ushayran | None | 0, 200, 400, 500, 700, 900, 1100 | Transposed Ajam. [MaqamWorld](https://www.maqamworld.com/en/maqam/ajam_ushayran.php) |
| Athar Kurd | None | 0, 100, 300, 600, 700, 800, 1100 | [MaqamWorld](https://www.maqamworld.com/en/maqam/athar_kurd.php) |
| Awj Iraq | D -50; E -50; F -50; G# -50; A -50; B +50 | 0, 150, 350, 450, 750, 850, 1150 | Tonic normalized from a half-flat reference; descending outline. [MaqamWorld](https://www.maqamworld.com/en/maqam/awj_iraq.php) |
| Bastanikar | D -50; E -50; G -50; G# -50; A# +50; B +50 | 0, 150, 350, 500, 650, 750, 1050, 1150 | First octave only; upper ajnas extend beyond the octave. [MaqamWorld](https://www.maqamworld.com/en/maqam/bastanikar.php) |
| Bayati Shuri | D -50 | 0, 150, 300, 500, 600, 900, 1000 | [MaqamWorld](https://www.maqamworld.com/en/maqam/bayati_shuri.php) |
| Dalanshin | E -50; B -50 | 0, 200, 350, 500, 700, 900, 1050 | First-octave Rast outline; upper modulation is played separately. [MaqamWorld](https://www.maqamworld.com/en/maqam/dalanshin.php) |
| Farahfaza | None | 0, 200, 300, 500, 700, 800, 1100 | Transposed Nahawand. [MaqamWorld](https://www.maqamworld.com/en/maqam/nahawand.php) |
| Hijaz | None | 0, 100, 400, 500, 700, 800, 1000 | Nahawand upper jins reference. [MaqamWorld](https://www.maqamworld.com/en/maqam/hijaz.php) |
| Hijazkar | None | 0, 100, 400, 500, 700, 800, 1100 | [MaqamWorld](https://www.maqamworld.com/en/maqam/hijazkar.php) |
| Hijazkar Kurd | None | 0, 100, 300, 500, 700, 800, 1000 | Lower-octave Kurd reference; octave-dependent alterations cannot be encoded in a repeating 12-key table. [MaqamWorld](https://www.maqamworld.com/en/maqam/hijazkar_kurd.php) |
| Husayni | D -50; A -50 | 0, 150, 300, 500, 700, 850, 1000 | Upper Bayati reference; B-flat alternative remains on its chromatic key. [MaqamWorld](https://www.maqamworld.com/en/maqam/husayni.php) |
| Huzam | D -50; E -50; F -50; G# -50; A -50; B -50 | 0, 150, 350, 450, 750, 850, 1050 | Tonic normalized from a half-flat reference. [MaqamWorld](https://www.maqamworld.com/en/maqam/huzam.php) |
| Iraq | D -50; E -50; G -50; A -50; B -50 | 0, 150, 350, 500, 650, 850, 1050 | Tonic normalized from a half-flat reference. [MaqamWorld](https://www.maqamworld.com/en/maqam/iraq.php) |
| Jiharkah | E -25; F -25; B -50 | 0, 200, 375, 475, 700, 900, 1050 | Reference approximation: thirds/fourths 25 cents low. [MaqamWorld](https://www.maqamworld.com/en/maqam/jiharkah.php) |
| Kirdan | E -50; B -50 | 0, 200, 350, 500, 700, 900, 1050 | Rast tuning; descending melodic identity. [MaqamWorld](https://www.maqamworld.com/en/maqam/kirdan.php) |
| Kurd | None | 0, 100, 300, 500, 700, 800, 1000 | [MaqamWorld](https://www.maqamworld.com/en/maqam/kurd.php) |
| Lami | None | 0, 100, 300, 500, 600, 800, 1000 | [MaqamWorld](https://www.maqamworld.com/en/maqam/lami.php) |
| Mahur | E -50 | 0, 200, 350, 500, 700, 900, 1100 | [MaqamWorld](https://www.maqamworld.com/en/maqam/mahur.php) |
| Muhayyar | D -50 | 0, 150, 300, 500, 700, 800, 1000 | Bayati tuning; upper-register melodic identity. [MaqamWorld](https://www.maqamworld.com/en/maqam/bayati.php) |
| Mustaar | D# -50; E -50; F# -50; G -50; A -50; B -50 | 0, 250, 350, 550, 650, 850, 1050 | Tonic normalized from a half-flat reference. [MaqamWorld](https://www.maqamworld.com/en/maqam/mustaar.php) |
| Nahawand | None | 0, 200, 300, 500, 700, 800, 1100 | Harmonic-minor reference; descending flat seventh is still available. [MaqamWorld](https://www.maqamworld.com/en/maqam/nahawand.php) |
| Nahawand Murassaa | None | 0, 200, 300, 500, 600, 900, 1000 | [MaqamWorld](https://www.maqamworld.com/en/maqam/nahawand_murassaa.php) |
| Nairuz | E -50; A -50 | 0, 200, 350, 500, 700, 850, 1000 | [MaqamWorld](https://www.maqamworld.com/en/maqam/nairuz.php) |
| Nawa Athar | None | 0, 200, 300, 600, 700, 800, 1100 | [MaqamWorld](https://www.maqamworld.com/en/maqam/nawa_athar.php) |
| Nikriz | None | 0, 200, 300, 600, 700, 900, 1000 | [MaqamWorld](https://www.maqamworld.com/en/maqam/nikriz.php) |
| Rahat al-Arwah | D -50; E -50; F -50; G# -50; A -50; B -50 | 0, 150, 350, 450, 750, 850, 1050 | Transposed Huzam. [MaqamWorld](https://www.maqamworld.com/en/maqam/huzam.php) |
| Saba | D -50 | 0, 150, 300, 400, 700, 800, 1000 | Repeating first-octave reference; upper lowered tonic is played chromatically. [MaqamWorld](https://www.maqamworld.com/en/maqam/saba.php) |
| Saba Zamzam | None | 0, 100, 300, 400, 700, 800, 1000 | Repeating first-octave reference. [MaqamWorld](https://www.maqamworld.com/en/maqam/saba_zamzam.php) |
| Sazkar | E -50; B -50 | 0, 200, 350, 500, 700, 900, 1050 | Rast tuning; chromatic D-sharp remains available. [MaqamWorld](https://www.maqamworld.com/en/maqam/kirdan.php) |
| Shadd Araban | None | 0, 100, 400, 500, 700, 800, 1100 | Transposed Hijazkar. [MaqamWorld](https://www.maqamworld.com/en/maqam/hijazkar.php) |
| Shahnaz | None | 0, 100, 400, 500, 700, 800, 1100 | Transposed Hijazkar. [MaqamWorld](https://www.maqamworld.com/en/maqam/hijazkar.php) |
| Shawq Afza | None | 0, 200, 400, 500, 700, 800, 1100 | [MaqamWorld](https://www.maqamworld.com/en/maqam/shawq_afza.php) |
| Sikah | D -50; E -50; F# -50; A -50; B -50 | 0, 150, 350, 550, 700, 850, 1050 | Tonic normalized from a half-flat reference. [MaqamWorld](https://www.maqamworld.com/en/maqam/sikah.php) |
| Sikah Baladi approx | D -50; E -50; F -25; A -50; B -50 | 0, 150, 350, 475, 600, 700, 850, 1050 | Approximation only; variable intonation is not fully representable by a fixed table. [MaqamWorld](https://www.maqamworld.com/en/maqam/sikah_baladi.php) |
| Suzdalara | E -50 | 0, 200, 350, 500, 700, 900, 1000 | [MaqamWorld](https://www.maqamworld.com/en/maqam/suzdalara.php) |
| Suzidil | None | 0, 100, 400, 500, 700, 800, 1100 | Transposed Hijazkar. [MaqamWorld](https://www.maqamworld.com/en/maqam/hijazkar.php) |
| Suznak | E -50 | 0, 200, 350, 500, 700, 800, 1100 | [MaqamWorld](https://www.maqamworld.com/en/maqam/suznak.php) |
| Ushaq Masri | A -50 | 0, 200, 300, 500, 700, 850, 1000 | [MaqamWorld](https://www.maqamworld.com/en/maqam/ushaq_masri.php) |
| Yakah | E -50; A -50 | 0, 200, 350, 500, 700, 850, 1000 | Transposed Nairuz. [MaqamWorld](https://www.maqamworld.com/en/maqam/nairuz.php) |
| Zanjaran | None | 0, 100, 400, 500, 700, 900, 1000 | [MaqamWorld](https://www.maqamworld.com/en/maqam/zanjaran.php) |
| Rast descending | E -50 | 0, 200, 350, 500, 700, 900, 1000 | Flat seventh descending reference. [MaqamWorld](https://www.maqamworld.com/en/maqam/rast.php) |
| Bayati upper Rast | D -50; A -50 | 0, 150, 300, 500, 700, 850, 1000 | Rast upper jins. [MaqamWorld](https://www.maqamworld.com/en/maqam/bayati.php) |
| Hijaz upper Rast | A -50 | 0, 100, 400, 500, 700, 850, 1000 | Rast upper jins. [MaqamWorld](https://www.maqamworld.com/en/maqam/hijaz.php) |

## Sources and implementation

Catalogue names, aliases, notated outlines and intonation limitations were
checked against [MaqamWorld, by Johnny Farraj](https://www.maqamworld.com/en/maqam.php),
including each linked maqam page. These are newly encoded factual reference
tables; no recordings or notation images are bundled. The 25-cent adjustments
in Jiharkah and Sikah Baladi are SynthMaven approximations, not exact performance
measurements or an assertion of universally fixed intonation.

Machine-readable source: `assets/maqam/scales.json`; compact firmware data:
`firmware/src/maqam_scales.h`. Run `python3 tools/gen_maqam.py` after editing the
catalogue, and `python3 tools/gen_maqam.py --check` to check the generated firmware
and editor data. The host regression compares all 46 settings at all 12 roots
across all 128 MIDI notes with the catalogue, including unaltered keys, octave
repeat, tonic, A4 addition, hardware controls and legacy Scala selection.

Version 1.4 removes Bluetooth and adds four custom chromatic tunings. See the [manual](USER_MANUAL.md).
