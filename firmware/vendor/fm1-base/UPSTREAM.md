fm1-base Bluetooth MIDI runtime, upstream commit fa7b203e6803a23a8be0bf560c46d07db836ee89:
https://github.com/charlesvestal/fm1-base

Retain the MPL-2.0 / Apache-2.0 notices in each file and the supplied licence texts.
SynthMaven adaptation: misc.c places the BLE heap in reserved low SRAM and radio delay wrappers in the dedicated RAM radio section. le_midi.c uses the SynthMaven device name in scan response to respect the advertisement packet limit. Platform hooks, flash-journal storage and MIDI routing are implemented separately in synthmaven_ble.c. SDK libraries and headers are obtained at build time; their licences remain applicable. No upstream compiled firmware is substituted for a SynthMaven build.
