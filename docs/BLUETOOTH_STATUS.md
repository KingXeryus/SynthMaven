# Native Bluetooth MIDI integration status

The development branch integrates the [fm1-base BLE MIDI runtime](https://github.com/charlesvestal/fm1-base), pinned at fa7b203e6803a23a8be0bf560c46d07db836ee89. The public 0.2 installer predates this integration. Do not infer hardware support from simulated editor tests.

The port includes central scan, explicit device selection, GATT discovery/subscription, disconnect controls, timestamp/running-status decoding, a dedicated MIDI queue and persisted radio configuration. The driver and upstream licence notices are in `firmware/vendor/fm1-base`.

All 23 synth engines remain. To make space, factory WAVE spectra use a lossless delta/DEFLATE codec (35,606 bytes instead of 131,072). All 256 decoded spectra and rendered table/mip buffers are compared against the original data. Decode workspace reuses the stopped WAVE builder's buffer. The synth build allocates engine arenas to its two active instrument/layer parts; the three-part project record layout stays unchanged. BLE task stacks use the pool and the 20-KiB RF-accessible heap uses low SRAM. USB debug CDC is disabled in BLE builds.

Target compilation and enforced flash/RAM/pool bounds must pass before publication. Hardware validation remains outstanding: both Chocolate models, reconnect and power cycles, discovery under nearby BLE traffic, sustained MIDI and audio timing. No simulated success is presented as real pairing.

Radio syscfg uses a journal in the formerly unused 0xFB000 sector, outside sample/project/preset/settings storage. Saves wait for stopped playback and no active voices. Interrupting a journal append preserves the preceding committed entry; power loss during rollover may forget bonds.

Native USB MIDI host support remains absent. A power bank supplies power, not a USB host. TRS keyboard input and BLE pedal input are the intended computer-free combination for this build.

## Published 0.2 baseline

Actions run 40228205416 compiled all engines and 41 stock FM6 voices. The optimized image measured 553352 bytes of 581564 app flash, 75352 bytes of 98304 static RAM, and 313024 bytes of 344064 pool. These measurements do not describe the new BLE development build.
