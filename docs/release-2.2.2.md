# SynthMaven 2.2.2

Fixes the update loader reset: the hardware flash-range guard now permits erasing settings sectors 0xFC000–0xFEFFF while preserving the factory head and final sector. Reset loops service USB and the watchdog after each sector.

Tests use the same range guard as the actual hardware and cover slow erases, settings erase failures, recovery-record retention, retries, damaged packages and same-version reinstall.

Solo levels and program controls are unchanged. Known limitation: layered programs can exceed the FM-1 audio deadline (135% CPU reported on hardware), causing crackles even at low levels. Output headroom does not solve processor overload. Use Layer OFF for affected programs while this is investigated. Host audio renders do not validate real-time CPU deadlines on the FM-1.

Firmware installation clears user settings, programs, presets, samples and autosave. Back up before installing.
