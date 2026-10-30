# SynthMaven 3.5.1 — refreshed factory programs and complete guide

This maintenance release refreshes the eight factory programs from the owner’s latest saved FM-1 bank, including the updated P1. All active native patches, mix settings, effects, splits and scenes are preserved. Web and iOS editor code is unchanged from 3.5. Bluetooth remains excluded.

The expanded illustrated PDF includes step-by-step firmware and editor workflows, control tables, named GLO pages, tuning, effects, sequencing, programs, scenes, MIDI Learn, USB host/powered hub wiring and backup/recovery. Download it from this release or the live installer.

## Validation

The factory decoder is checked against all eight complete captured records. The updated bank passed 288 host audio renders, the full host/web regression suite, and firmware flash/RAM layout checks. The changed P1 passed one-note and three-note hardware timing tests with no audio deadline misses, second-core faults, timeouts or resource failures. Saved device objects still match the backup after testing. Other programs are unchanged from the previously validated capture. These bounded checks do not guarantee every performance or combination is overload-free.

## Install and recover

1. Export a full backup and any editor-local libraries/scales before updating. Every flash resets device settings and musical storage.
2. Set USB role to COMPUTER and use the live installer in Chrome or Edge with a data cable.
3. Keep USB power connected until completion and restart. Fresh startup loads factory P1.
4. Use the included recovery archive for your operating system if recovery is needed; follow the illustrated guide.
5. Restore only compatible backups. Older SynthMaven binary formats require their matching firmware.

All four layers and independent drums share processing and voice resources. Direct USB controllers require suitable external power/adapter/hub. No fixed per-layer polyphony, loudness target or Bluetooth support is claimed.

Firmware is built and attested by the required shared builder. SHA256SUMS covers the downloadable firmware, guide and recovery assets.

Based on Melodee by Kerem Kilic (Ellic Studio), Felucca by Leo Kuroshita / Hügelton Instruments, retained Clipper/3dSam-derived SynthMaven work and the credited third-party components. See the licence archive.
