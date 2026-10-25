# SynthMaven 1.0 firmware

## Changes

- Adds 88 Oriental / Middle Eastern melodic presets: 44 instruments, with Dry
  and Expressive variations. No percussion is added. Chorus, delay and reverb
  sends and SPACE start at zero; set effects on the FM1 or in the web editor.
- Adds an ORIENTAL filter to the four-patch hardware browser and a full-name
  collection selector to the web editor. Original factory IDs remain unchanged.
- Preserves all 23 engines, the DX7/FM6 and nine XD compatibility presets, and
  the native Bluetooth runtime. Total factory collection: 537 presets.
- Keeps patch settings through project recall and fixes a parameter-name table
  bounds error. Keyboard performance controllers follow KEY IN; patch changes
  follow the independent PATCH MIDI route.
- Editor protocol 16 supports extended preset indices/counts. Use the matching
  online editor when updating firmware.

[User manual](USER_MANUAL.md) · [Quick-start guide](../QUICKSTART.md)

## Load and edit

Back up the FM1 with the web editor before installing. Open the
[installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/) in Chrome
or Edge. After updating, tap SAVE for PRESETS and select ORIENTAL with KNOB 2.
KNOB 1 highlights; KNOB 3 loads; KNOB 4 loads and opens Edit. D/E in the short
name means Dry/Expressive. Save edits as a USER preset. In the
[editor](https://kingxeryus.github.io/SynthMaven/webapp/editor/), choose a full
name from the Oriental / Middle Eastern selector. Tuning is independent; choose
Scala/custom tuning for the desired maqam.

## Validation and limits

Host tests cover all 88 patches and 126 track parameters, hardware browser
indexing, MIDI response, project recall, routing, storage and editor transfers.
The actual BLE-enabled target passed [CI run 40243655940](https://github.com/KingXeryus/SynthMaven/actions/runs/40243655940):
580504/581564 bytes app flash, 92132/98304 bytes static RAM, 24832/25672 bytes
low SRAM, and 329008/344064 bytes pool. Package size: 610045 bytes.
The publication workflow repeats the checks before deployment. Version 1.0 still requires hardware validation: actual FM1 listening,
Chocolate pairing/reconnect and audio timing under wireless load remain pending.
USB device MIDI works through a host; direct USB keyboard host mode is not yet
integrated. A separately powered keyboard does not supply that host function.

These are original SynthMaven parameter recipes using the existing engines;
no Yamaha ESP or TAQS.IM commercial samples or preset binaries are included.
See [bank provenance](../assets/oriental/README.md), [Bluetooth status](BLUETOOTH_STATUS.md)
and [USB MIDI status](USB_MIDI_STATUS.md).

## Melodic instrument catalog

Each instrument below has Dry and Expressive variants (88 presets total).

| Instrument | Native engine | OLED stem |
|---|---:|---|
| Oud Amber | PHYS | OUD AMBER |
| Oud Silk | PHYS | OUD SILK |
| Electric Oud | PHYS | ELECTRIC O |
| Qanun Pearl | PHYS | QANUN PEAR |
| Qanun Felt | PHYS | QANUN FELT |
| Baglama Wire | PHYS | BAGLAMA WI |
| Saz Long Neck | PHYS | SAZ LONG N |
| Buzuk Copper | PHYS | BUZUK COPP |
| Tanbur Cedar | PHYS | TANBUR CED |
| Santur Glass | PHYS | SANTUR GLA |
| Semsemeya Lyre | PHYS | SEMSEMEYA |
| Levant Lyre | PHYS | LEVANT LYR |
| Dutar Wood | PHYS | DUTAR WOOD |
| Silk Road Koto | PHYS | SILK ROAD |
| Shamisen Pick | PHYS | SHAMISEN P |
| Pipa Bamboo | PHYS | PIPA BAMBO |
| Ney Dawn | FM6 | NEY DAWN |
| Kawala Breath | FM6 | KAWALA BRE |
| Duduk Velvet | VOICE | DUDUK VELV |
| Mey Ember | VOICE | MEY EMBER |
| Balaban Warm | VOICE | BALABAN WA |
| Mizmar Fire | VOICE | MIZMAR FIR |
| Zurna Festival | VOICE | ZURNA FEST |
| Shehnai Gold | VOICE | SHEHNAI GO |
| Kaval Air | FM6 | KAVAL AIR |
| Arab Clarinet | FM6 | ARAB CLARI |
| Accordion Reed | VOICE | ACCORDION |
| Shawm Copper | VOICE | SHAWM COPP |
| Arab Strings | STRINGS | ARAB STRIN |
| Turk Ensemble | STRINGS | TURK ENSEM |
| Persian Strings | STRINGS | PERSIAN ST |
| Kamanche Bow | PHYS | KAMANCHE B |
| Rebab Desert | PHYS | REBAB DESE |
| Erhu Silk | PHYS | ERHU SILK |
| Silk Ensemble | STRINGS | SILK ENSEM |
| Levant Cloud | VECTOR | LEVANT CLO |
| Desert Ribbon | ANALOG | DESERT RIB |
| Dabke Pulse | ANALOG | DABKE PULS |
| Shabi Saw | JUNO | SHABI SAW |
| Rai Brass | FM6 | RAI BRASS |
| Balkan Pulse | ACID | BALKAN PUL |
| Turk Solo | VPS | TURK SOLO |
| Golden Reed | FMX | GOLDEN REE |
| Oriental Vox | VOICE | ORIENTAL V |

## Factory presets by engine

| Engine | Existing factory presets | New melodic presets | Total |
|---|---:|---:|---:|
| ANALOG | 32 | 4 | 36 |
| DIGITAL | 30 | 0 | 30 |
| PHASE | 16 | 0 | 16 |
| LOFI | 12 | 0 | 12 |
| SAMPLE | 4 | 0 | 4 |
| VOICE | 12 | 18 | 30 |
| TRIO | 20 | 0 | 20 |
| WHEEL | 14 | 0 | 14 |
| GRAIN | 10 | 0 | 10 |
| FM6 | 41 | 10 | 51 |
| PHYS | 32 | 38 | 70 |
| NOISE | 14 | 0 | 14 |
| HYBRID | 41 | 0 | 41 |
| WAVE | 28 | 0 | 28 |
| ACID | 16 | 2 | 18 |
| STRINGS | 16 | 8 | 24 |
| MACRO | 24 | 0 | 24 |
| VPS | 10 | 2 | 12 |
| TERRAIN | 10 | 0 | 10 |
| FMX | 16 | 2 | 18 |
| WSEQ | 16 | 0 | 16 |
| JUNO | 17 | 2 | 19 |
| VECTOR | 18 | 2 | 20 |
| **Total: 23 engines** | **449** | **88** | **537** |
