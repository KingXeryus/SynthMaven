fm1-base d8ff8f41aa511e03d70a7c565c9fa0f1dd18ae2f; felucca-bsp 305c372c3ec18f3917744e608998a0d7fd700135.
https://github.com/charlesvestal/fm1-base
https://github.com/hugelton/felucca-bsp
BLE stack from Henri David / Optimist, GPL-3.0-only OR MPL-2.0; used under GPL-3.0-only. HAL retains its individual SPDX notices. Local adaptation: baseband DMA block in low SRAM; static RF table include. No proprietary JieLi Bluetooth libraries linked.

The interrupt wrappers are assembled separately in fm1_ble_isr.S to preserve their external linkage under whole-program LTO. Detailed diagnostics are disabled for the constrained release image. The application adapter exposes bounded editor SysEx only.

SynthMaven moves measured RF trim code and its lookup data into SRAM, with a volatile indirect entry call, and completes startup calibration before starting the second core and audio/input IRQs. This avoids flash-fetch faults observed on the connected test unit. These changes require target disassembly and hardware checks.
