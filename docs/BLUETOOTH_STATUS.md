# Native Bluetooth MIDI integration status

The development branch integrates the [fm1-base BLE MIDI runtime](https://github.com/charlesvestal/fm1-base), pinned at fa7b203e6803a23a8be0bf560c46d07db836ee89. Version 0.2 predates this integration. Do not infer hardware support from simulated editor tests.

The port includes central scan, explicit device selection, GATT discovery/subscription, disconnect controls, timestamp/running-status decoding, a dedicated MIDI queue and persisted radio configuration. The driver and upstream licence notices are in `firmware/vendor/fm1-base`.

All 23 synth engines remain. To make space, factory WAVE spectra use a lossless predictive/DEFLATE codec (30,778 bytes instead of 131,072). All 256 decoded spectra and rendered table/mip buffers are compared against the original data. Decode workspace reuses the stopped WAVE builder's buffer. The synth build allocates engine arenas to its two active instrument/layer parts; the three-part project record layout stays unchanged. BLE task stacks use the pool and the 20-KiB RF-accessible heap uses low SRAM. USB debug CDC is disabled in BLE builds.

Target compilation and enforced flash/RAM/pool bounds must pass before publication. Hardware validation remains outstanding: both Chocolate models, reconnect and power cycles, discovery under nearby BLE traffic, sustained MIDI and audio timing. No simulated success is presented as real pairing.

Radio syscfg uses a journal in the formerly unused 0xFB000 sector, outside sample/project/preset/settings storage. Saves wait for stopped playback and no active voices. Interrupting a journal append preserves the preceding committed entry; power loss during rollover may forget bonds.

Native USB MIDI host support remains absent. A power bank supplies power, not a USB host. TRS keyboard input and BLE pedal input are the intended computer-free combination for this build.

## Published 0.2 baseline

Actions run 40228205416 compiled all engines and 41 stock FM6 voices. The optimized image measured 553352 bytes of 581564 app flash, 75352 bytes of 98304 static RAM, and 313024 bytes of 344064 pool. These measurements do not describe the new BLE development build.

## Native 0.3 target validation

[Actions run 40231787333](https://github.com/KingXeryus/SynthMaven/actions/runs/40231787333) built the complete native BLE firmware and package. The optimized image is 578456 bytes (of 581564), static RAM is 91092 bytes (of 98304), low-SRAM data is 24832 bytes (of 25672 remaining after RAM code), and the pool is 329008 bytes (of 344064). All enforced region bounds pass. The flash driver contains 932 instructions with no calls; radio calibration runs separately in `.radio_ram`. The package is 610045 bytes.

Host validation covers MIDI routing, persistent settings, interrupted radio journal writes, explicit peer selection, real-state reporting, Korg/default patch mappings and lossless WAVE reconstruction/rendering. This establishes a buildable development package, not a hardware-validated Chocolate connection.

## Native 0.4 target validation

[Actions run 40243154995](https://github.com/KingXeryus/SynthMaven/actions/runs/40243154995)
built all 23 engines, native BLE and the 88 melodic Oriental presets. The image
uses 580568 of 581564 bytes of app flash (996 bytes spare), static RAM uses
92132 of 98304 bytes, low-SRAM data uses 24832 of 25672 bytes, and the pool uses
329008 of 344064 bytes. The 610045-byte firmware package passed the target
checks. The compiler used the existing -Oz fallback without dropping engines.
These are build and host-test results; physical Chocolate pairing and audio
validation remain outstanding.

## Native 1.0 target validation

[Actions run 40243655940](https://github.com/KingXeryus/SynthMaven/actions/runs/40243655940)
built the version-1.0 firmware with all 23 engines and 537 factory presets,
including 88 melodic Oriental sounds. The image uses 580504/581564 bytes of app
flash (1060 bytes spare), 92132/98304 bytes of static RAM, 24832/25672 bytes of
low SRAM, and 329008/344064 bytes of pool. The package is 610045 bytes. All
enforced bounds and host/editor checks pass. The full web regression also
passes locally. Physical pairing and audio validation remain outstanding.
