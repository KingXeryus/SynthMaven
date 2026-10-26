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

## Version 1.1 startup fix

The radio shim's `bt_event_notify(from,event)` forwards `len=0` because the SDK
API has no length argument. The 1.0 adapter required `len>=1` and discarded all
ready notifications. Version 1.1 uses the typed event's source and event ID,
matching the upstream adapter. The host test now uses the actual `len=0` call,
including wrong-source/null rejection and a ready event after the 15-second
timeout. This corrects a deterministic startup blocker. Physical pairing,
reconnect and radio/audio performance still require hardware checks.

## Native 1.1 target validation

[Actions run 40247482272](https://github.com/KingXeryus/SynthMaven/actions/runs/40247482272)
built commit 32dae18c786001f9805ba4a42b5a5071ecd0277c with native BLE,
all 23 engines and 537 factory presets. The image uses **580504/581564 bytes**
of app flash, **91220/98304 bytes** of static RAM, **24832/25672 bytes** of low
SRAM, and **329008/344064 bytes** of pool. The existing -Oz fallback retained all
engines and the logo. The **610045-byte** package identifies as **FM-1_911** and
the application reports **SYNTHMAVEN 1.1**. All enforced target bounds and the
host/editor suite pass; the full web regression also passes locally.

A read-only USB check on the connected physical FM1 before updating reported
FELUCCA SYNTHMAVEN 1.0, Bluetooth enabled and BLE state 7 (startup failure),
consistent with the rejected ready event. No firmware was flashed during that
check. Physical Chocolate/Chocolate Plus pairing and audio remain unverified.

### HOME SELECT control update

[Actions run 40249130630](https://github.com/KingXeryus/SynthMaven/actions/runs/40249130630)
validates commit c8c5a65968e9398679a0c0d3b6c9d7e8ae29e6c2, adding SELECT preset
navigation on HOME. The firmware uses 580552/581564 bytes of app flash;
static RAM, low SRAM and pool remain 91220/98304, 24832/25672 and 329008/344064.
All enforced target bounds pass. Host tests verify forward/reverse wrap,
every detent, unchanged HOME tempo and retained editing-page navigation.

## 1.2 Chocolate Plus discovery

Live Mac observation found FootCtrlPlus advertising name FootCtrlPlus, HID
service 1812 and manufacturer data beginning with ASCII sinco. No standard
MIDI service UUID was advertised. The old filter discarded such peers.
Version 1.2 admits recognized FootCtrl/Chocolate names as candidates and keeps
the existing GATT MIDI-characteristic requirement for readiness. Discovery
keys include address type. The client accepts enhanced connection-complete
events as well as legacy events. Host tests run the actual advertisement parser
with the observed name/service combination under ASan and UBSan.
Physical connection/subscription and MIDI on the updated FM1 remain pending.

Read-only Mac GATT inspection subsequently connected to FootCtrlPlus and verified
the standard MIDI service 03B80E5A-EDE8-4B33-A751-6CE34EC4C700 and characteristic
7772E5DB-3868-4112-A1A9-F2669D106BF3 (properties 22). Physical FM1 subscription
and pedal events remain unverified. Status now also recognizes incoming links:
an incoming link without notification subscription shows discovery; a subscribed
link shows MIDI READY. DISCONNECT handles both link directions.

## Known Bluetooth crash — experimental release

During 1.1 Bluetooth/Chocolate Plus testing, the FM1 crashed and entered USB
rescue; the user restored the original firmware. The cause remains unresolved.
Version 1.2 fixes discovery and link-status reporting, but has not been shown
to fix that crash. Hardware stability and pedal MIDI remain unverified.

