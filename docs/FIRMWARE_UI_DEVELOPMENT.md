# Firmware UI and global tuning development

Unreleased work on `firmware-ui-tuning`, based on 2.2.4. The public release, installer and FM-1 have not been changed. Development binaries keep the existing identity; do not upload them over the published 2.2.4 assets. New explicit program tuning flags are understood by this branch; older firmware rejects these newly saved program records. Update the release/protocol metadata before publishing this storage-policy change.

Implemented:
- Red/blue/white/ice-blue accents and tinted parameter, program, mixer and settings cards. Same navigation and physical controls. RGB565 text contrast minimum 4.92:1 including selected cards.
- Global scale, root, A4, octave boundary and custom definitions survive ordinary program recall. GLO > TUNING 2 > SAVE ON opts into storing/restoring program tuning. Explicit equal temperament is supported. Legacy non-default tuning is retained as an override; legacy equal-tempered defaults follow global tuning. A recalled override becomes the current tuning, which subsequent global programs preserve. Per-part bypass and explicitly captured scene tuning remain separate.
- LFO FILTER combines actual engine cutoff, signed depth, rate and wave. Static base/sweep preview; unsupported engine cutoff/depth disabled. Existing LFO pages and DSP remain unchanged.
- EDIT opens and latches the mixer after two seconds, with no early screen flash. GLO remains unlatched; other hold timings are unchanged.

Validation: full editor/installer web suite, full SynthMaven host regression, global tuning and migration tests, early/release-at-two-seconds mixer tests, all-engine cutoff descriptors, RGB565 contrast, production renderer screen capture, 245-page PDF coverage/bounds check, target toolchain build. Image 556484 bytes, RAM 87432/98304, pool 329480/344064. No additional physical-device flash, audio timing or candidate test. Audio DSP, loudness, voice allocation and loader sources are unchanged.

Design research used the actual guide screenshots and Firecrawl's reads of [Elektron Digitone II](https://www.elektron.se/explore/digitone-ii) and [Arturia MiniFreak](https://www.arturia.com/products/hardware-synths/minifreak-49/overview). The adopted principles are clear parameter hierarchy, directly grouped filter/modulation controls and static visual feedback. [Lovable design study](https://lovable.dev/projects/198a63a7-f191-4626-9e31-f1500cce1116) supplied colored-card and sweep-graph references; it is not firmware and was not publicly deployed. Its initial assumed layouts were corrected against the uploaded screenshots. Production firmware preserves its own geometry, fonts and navigation.

## TRS polarity limitation

The requested Type A/B toggle is not implemented. The [MIDI Association's wiring diagrams](https://midi.org/updated-how-to-make-your-own-3-5mm-mini-stereo-trs-to-midi-5-pin-din-cables) show that this is a tip/ring current-loop wiring reversal. The current FM-1 HAL exposes PH8 -> input channel 1 -> UART1 RX. Neither that driver nor the installed JieLi UART API documents a switch for the jack's tip/ring wiring. No register was guessed or changed. A compatible MIDI adapter or tip/ring crossover is the supported practical remedy; a verified board schematic showing a controllable polarity-switching circuit would be needed to implement this as a device setting. UART logic inversion alone does not establish Type A/B support.

## Rounded controls and piano tuning view

The [X0X firmware emulator](https://charlesvestal.github.io/fm1-x0x/emu/) informed the rotary-indicator layout. New integer-only dial geometry and rounded cards are implemented locally; no X0X code or assets were copied. Existing navigation, DSP, and parameter ranges are unchanged. The tuning keyboard uses the existing maqam membership masks and actual tuning table. Red denotes included detuned keys; grey denotes excluded keys, white/black preserve the piano geometry, and blue underlines mark played pitch classes. Cents describe the selected FROM octave. Host pixel checks cover colours, root rotation and the tuning boundary.

Rounded UI validation: SynthMaven regression suite (full browser suite omitted for this firmware-only change), dedicated pixel tests, all 152 guide layouts/66 menus, and 245-page guide checks pass. Final target image 557300 bytes; RAM 87448/98304; lowram 4352/29496; pool 329480/344064. No hardware candidate tests or flashes.
