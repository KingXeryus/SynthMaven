# SynthMaven 2.2.4

Version 2.2.4 copies the eight-program configuration captured from the connected FM-1. P1 STAGE PIANO uses the captured HYBRID main sound; P2–P8 are unchanged from 2.2.3. Embedded patches, parameters, mix levels, pan, routes, tuning and stored scenes are preserved byte for byte. The original 2.2.3 firmware remains available. No synthesis, navigation, loader or loudness-processing changes are included. Offline checks validate the imported records and audio; no additional device timing tests or firmware installs were performed for this release.

## Installation and recovery

Use the [live installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/) with the FM-1 connected by USB in COMPUTER mode. Back up before installing: every firmware flash resets settings, saved programs, user presets, samples and autosave. Keep the cable connected until installation completes. If interrupted, reconnect in recovery mode and retry; see the included INSTALL-AND-RECOVERY.md and PDF guide for the full procedure.

## Known limitations

The eight synthesis voices are shared across four parts. Heavy layers can exceed the device CPU budget; this capture does not establish hardware timing compliance. The final 2.2.3 mono-note batching fix and this bank have host validation only. Direct USB host mode requires a powered adapter or hub and compatible hardware. Bluetooth is not supported. USER WT remains silent until a user wavetable is supplied.
