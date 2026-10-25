# Native Bluetooth MIDI integration status

The radio is present on the FM1. SynthMaven's current bare-metal audio platform does not initialize it or run a BLE stack. Enabling a UI flag cannot provide a connection.

A concrete implementation is available in [fm1-base](https://github.com/charlesvestal/fm1-base), including [BLE MIDI central discovery and subscription](https://github.com/charlesvestal/fm1-base/blob/main/ble/midi/le_midi.c), OS/task/timer shims and JieLi stack libraries. Its documented additional cost for peripheral plus central is about 132 KB flash, 46 KB static RAM and 24 KB task-stack pool. It is a port, not a single header or toggle.

The preceding SynthMaven build (Actions run 40227249183) measured:

| Region | Used | Capacity | Free |
|---|---:|---:|---:|
| App flash | 580784 B | 581564 B | 780 B |
| Static RAM | 74872 B | 98304 B | 23432 B |
| Pool | 313024 B | 344064 B | 31040 B |

The BLE port does not fit this memory layout. The supplied stock DX7 voices also consume app space; the build retries Clang `-Oz` if `-Os` reaches the fixed flash limit, retaining all engines and the supplied logo. The linker and image checks still enforce all region bounds.

The user chose to preserve all engines rather than produce an FM6-only Bluetooth edition. Native Bluetooth therefore remains pending a larger memory-layout/size redesign and a real driver integration. No capability bit, pairing-success message or mock connection is presented as working hardware.

A complete port must reserve controller/stack/heap/task memory, integrate IRQ/clock/calibration/store hooks without starving audio, add explicit scan/select/connect/disconnect controls, and deliver decoded MIDI to input port 2. It then needs device tests with both Chocolate models, program changes and simultaneous keyboard playback. Manufacturer APIs also exist in [fm1_open](https://github.com/m-vave/fm1_open/blob/main/sdk/include/fm1.h), but belong to its binary SDK/runtime; these cannot be linked into this firmware without reconciling startup, interrupts, audio ownership and memory layout.

## SynthMaven 0.2 target build

Actions run 40228205416 compiled all engines, all 33 added stock voices and the supplied logo. `-Os` exceeded app flash by 5428 bytes; `-Oz` passed with an image of 553352 bytes (28212 bytes free), static RAM of 75352 bytes (22952 free), and the same 313024-byte pool. This is still substantially below the BLE central port's requirements.

Removing VOICE, WHEEL and NOISE alone is insufficient for RAM. WHEEL owns 288 bytes of per-track state and 2208 bytes of per-voice state (2496 total). VOICE and NOISE keep their working state in shared generic voices; removing them does not release that allocation while the other engines remain. The shared engine arena also remains required by GRAIN, FM6, PHYS, HYBRID and FMX. These sizes were checked with the current C types; a final port still requires a target-linked memory audit and hardware validation.
