# SynthMaven 0.4 development firmware

## Changes

- Adds 88 Oriental / Middle Eastern melodic presets: 44 instruments, with Dry
  and Expressive variations. No percussion is added. Chorus, delay and reverb
  sends start at zero; set effects on the FM1 or in the web editor.
- Adds an ORIENTAL filter to the four-patch hardware browser and a full-name
  collection selector to the web editor. Original factory IDs remain unchanged.
- Preserves all 23 engines, the DX7/FM6 and nine XD compatibility presets, and
  the native Bluetooth runtime. Total factory collection: 537 presets.
- Keeps patch settings through project recall and fixes a parameter-name table
  bounds error. Keyboard performance controllers follow KEY IN; patch changes
  follow the independent PATCH MIDI route.
- Editor protocol 16 supports extended preset indices/counts. Use the matching
  online editor when updating firmware.

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
The actual BLE-enabled target passed [CI run 40243154995](https://github.com/KingXeryus/SynthMaven/actions/runs/40243154995):
580568/581564 bytes app flash, 92132/98304 bytes static RAM, 24832/25672 bytes
low SRAM, and 329008/344064 bytes pool. Package size: 610045 bytes.
The publication workflow repeats the checks before deployment. This is a development release: actual FM1 listening,
Chocolate pairing/reconnect and audio timing under wireless load remain pending.
USB device MIDI works through a host; direct USB keyboard host mode is not yet
integrated. A separately powered keyboard does not supply that host function.

These are original SynthMaven parameter recipes using the existing engines;
no Yamaha ESP or TAQS.IM commercial samples or preset binaries are included.
See [bank provenance](../assets/oriental/README.md), [Bluetooth status](BLUETOOTH_STATUS.md)
and [USB MIDI status](USB_MIDI_STATUS.md).

## Melodic instrument catalog

Each instrument below has Dry and Expressive variants (88 presets total).

| Instrument | Native engine index | OLED stem |
|---|---:|---|
| Oud Amber | 10 | OUD AMBER |
| Oud Silk | 10 | OUD SILK |
| Electric Oud | 10 | ELECTRIC O |
| Qanun Pearl | 10 | QANUN PEAR |
| Qanun Felt | 10 | QANUN FELT |
| Baglama Wire | 10 | BAGLAMA WI |
| Saz Long Neck | 10 | SAZ LONG N |
| Buzuk Copper | 10 | BUZUK COPP |
| Tanbur Cedar | 10 | TANBUR CED |
| Santur Glass | 10 | SANTUR GLA |
| Semsemeya Lyre | 10 | SEMSEMEYA |
| Levant Lyre | 10 | LEVANT LYR |
| Dutar Wood | 10 | DUTAR WOOD |
| Silk Road Koto | 10 | SILK ROAD |
| Shamisen Pick | 10 | SHAMISEN P |
| Pipa Bamboo | 10 | PIPA BAMBO |
| Ney Dawn | 9 | NEY DAWN |
| Kawala Breath | 9 | KAWALA BRE |
| Duduk Velvet | 5 | DUDUK VELV |
| Mey Ember | 5 | MEY EMBER |
| Balaban Warm | 5 | BALABAN WA |
| Mizmar Fire | 5 | MIZMAR FIR |
| Zurna Festival | 5 | ZURNA FEST |
| Shehnai Gold | 5 | SHEHNAI GO |
| Kaval Air | 9 | KAVAL AIR |
| Arab Clarinet | 9 | ARAB CLARI |
| Accordion Reed | 5 | ACCORDION |
| Shawm Copper | 5 | SHAWM COPP |
| Arab Strings | 15 | ARAB STRIN |
| Turk Ensemble | 15 | TURK ENSEM |
| Persian Strings | 15 | PERSIAN ST |
| Kamanche Bow | 10 | KAMANCHE B |
| Rebab Desert | 10 | REBAB DESE |
| Erhu Silk | 10 | ERHU SILK |
| Silk Ensemble | 15 | SILK ENSEM |
| Levant Cloud | 22 | LEVANT CLO |
| Desert Ribbon | 0 | DESERT RIB |
| Dabke Pulse | 0 | DABKE PULS |
| Shabi Saw | 21 | SHABI SAW |
| Rai Brass | 9 | RAI BRASS |
| Balkan Pulse | 14 | BALKAN PUL |
| Turk Solo | 17 | TURK SOLO |
| Golden Reed | 19 | GOLDEN REE |
| Oriental Vox | 5 | ORIENTAL V |
