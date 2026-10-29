# SynthMaven 2.2.1

Firmware installation now clears saved programs, settings, presets, samples and autosave after verifying the update. Reinstalling the same version also resets user storage. Back up before installing.

Reverts the 2.2 loudness boosts, solo compensation and modified output limiter to the earlier gain structure. All eight factory programs are regenerated without added loudness boosts. The -14 LUFS target no longer applies.

Loader tests cover erased user storage, same-version reinstall, package corruption and missing host confirmation. Audio checks use production-DSP host renders; physical FM-1 validation remains pending. The illustrated 2.2 guide is included as a reference; this patch changes installation reset behavior and removes the loudness target.
