# Bluetooth / dual-core investigation — 29 October 2026

## Release status

Published firmware 3.0 remains USB-only. None of the candidates below is approved for release. All 23 engines and 557 factory entries are retained. DSP, layering and gain processing have not been changed in these candidates.

## Hardware evidence

- Validated mailbox, isolated writer cache lines: enabling the radio still restarted the device. AXI write fault, second-core idle-loop location.
- Mailbox plus 64 NOPs between idle polls: 49 Bluetooth editor requests and readbacks passed with two cores. Four layered programs then passed with no missed audio deadlines; a later program restarted. AXI read fault reported at a second-core audio-render location. This is a failed candidate.
- Separate per-core fault records: Bluetooth connected with two cores, but sustained layered editing recorded a missed audio deadline and subsequently restarted. Main-core report: PC `0x02017b14`, return `0x0201fb10`, DBG `0x10` (AXI read invalid), stack `0x01c7baf4`, AXI read ID `0x100`. Recording ownership matters: the previous shared record could be overwritten by the second core reporting the same bus fault.
- Full register capture also reproduced a main-core AXI read fault after four programs, at PC `0x0201e0e6`, return `0x02015f9e`, read ID `0x100`. The instruction trace follows `blep` → `osc_saw` and then lands at `0x0201e0de`; a valid JUNO call site returns to `0x020160de`. Those addresses differ by bit `0x8000`. The register context fits JUNO rather than the destination TRIO function. This is evidence consistent with a corrupted saved return address, not proof of a specific SRAM or power fault. Do not infer a memory-capacity fix from these results.
- The identical radio-capable binary with the radio switched OFF completed all eight programs over USB without a reset. Peak workload was 96%; one missed deadline first appeared in program 7. This too fails audio validation. It is distinct from the released USB-only binary, which does not include the radio or its link-time build changes.
- Second-core stack guard: four programs completed, at least 3156 of 4096 stack bytes remained unused, then the device restarted in program 5. The new fault is a PC-limit exception (`DBG 0x1000`) at `0x0281e0e0`; the trace again returned from `osc_saw` to `0x0281e0de` instead of the JUNO return site `0x020160de` (difference `0x00808000`). This strengthens the corrupted-return-address evidence; it does not establish whether the fault is a CPU/bus, memory, power, or software-write issue. No CPU1 stack overflow was observed.
- All candidates remain failed. Restore the verified published USB-only image after the investigation; do not package any diagnostic candidate as a release.

## Validation requirements

Retain both cores and all engines/presets. Test Bluetooth enable, connection, repeated reads/writes, all eight layered programs, reconnection and cold starts. Record resets, core state, timing peaks and audio deadline misses. A connection-only pass is insufficient. Host regression passes do not substitute for these hardware tests.

Local evidence is in `build/ble31-*.log`; verified pre-test backup is `output/backups/SynthMaven-before-ble31.backup.json`. These files contain device-specific data and are not release assets.

## iOS

Version 3.0 build 8, display name **SM FM1 Editor**, is uploaded, available to internal testing, and submitted for external Beta App Review. Next-release metadata is prepared as **3.1 build 9**, with the same spaced display name; native/bridge/Perform/simulator checks passed. The signed 3.1 archive was also built and its embedded version/name verified. Build 9 has not been uploaded.
