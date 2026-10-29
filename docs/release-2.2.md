# SynthMaven 2.2 — eight performance programs, louder factory calibration and illustrated guide

SynthMaven is based on Clipper by 3dSam. Version 2.2 keeps the existing HOME navigation, shortcuts, 23 engines and stable 557-entry sound catalogue, including 64 Vital-derived HYBRID approximations.

## New factory performance bank

Eight four-part programs foreground Piano, Epiano, Lead Synth, Oriental Lead, Disco Synth, Brass, Strings and Bass, with three complementary support sounds each. Each includes MAIN, INTIMATE, AIR and DRY scenes. P1 STAGE PIANO starts a fresh working state; saved user slots and resumed work take precedence. Unused named slots fall back to ROM programs. The editor's Factory programs control recalls any ROM recipe without writing a saved slot. Existing custom-scale definitions survive ROM recall.

All four parts share **eight synthesis voices total**, with lower per-part limits in some engines. Mono support parts deliberately follow the last held note to leave room for the foreground polyphonic instrument. P1 is a synthesized stage piano, not an acoustic-grand multisample. This release does not change the existing engine/preset/part HOME controls.

## Audio validation and level matching

The target is **−14 LUFS integrated, within ±0.5 LU**, for all 556 audible sound presets, the 32 isolated program parts, and every layered scene of the eight programs under documented reference performances. USER WT intentionally remains silent until user data is uploaded. Stereo BS.1770-4 gating and 4× oversampled true-peak measurement operate on the production DSP output. The ceiling is **−1 dBTP**. Preview WAV files receive no normalization after rendering.

Reference phrases distinguish bass, held chords, comping, melody and single-note chord models. The explicitly listed short/transient sounds use a 16 Hz roll; sparse millisecond impulses do not fill a 400 ms loudness window. Program performances test variable velocities; additional velocity-127 renders check headroom. Loudness is performance-dependent: a quieter velocity, sparse playing, changed sound, altered effects or a custom mix can differ from −14 LUFS. This is fixed calibration, not continuous automatic gain control.

Static per-sound trim supplies factory gain. Fixed per-program-part solo compensation applies when exactly one configured part is audible, including layer OFF or solo; layered operation retains the intentional foreground/support mix. The stereo-linked look-ahead limiter uses an oversampled peak estimate, with pitch-aware recovery for selected transient sounds. Existing saved user gains are preserved; reload a factory sound/program to use its new calibration.

Machine-readable factory and program audit reports and an eight-WAV preview archive accompany the release. Those are host renders of production synthesis, not physical FM-1 recordings or a guarantee of subjective sound quality.

## Complete illustrated documentation

The [PDF user guide](SynthMaven-2.2-User-Guide.pdf) covers all 65 reachable firmware menus, engine/effect variants, holds, submenus, confirmation/status screens, every calibration prompt, primary/secondary controls, ranges/defaults/options and step-by-step sound/program workflows. Editor-only features have their own reference. Native screens are produced by the firmware renderer with simulated inputs. The installer links directly to the PDF.

## Storage and editor compatibility

Protocol **24** adds command **118**, read-only factory-program recall (slots 0–7). PRG3/PBK2 record sizes, flash addresses and wavetable/sample regions remain unchanged. Sound flag bits 2–7 encode zero for legacy unity, or 1–63 for calibrated unity through +62 dB; the calibrated state also selects consistent transient recovery. Sound trim extends to +63.5 dB in half-dB steps. New backups require protocol 24 before any restore write; older saved programs remain readable. Export a backup before updating.

## Install or recover

Use the [installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/) in Chrome or Edge with a USB data cable and COMPUTER USB role. Keep power connected until writing completes. Check the firmware checksum against SHA256SUMS.txt. Read [installation and recovery](../QUICKSTART.md) and keep original firmware available. Hold OCT− during power-on for USB rescue; both octave buttons start panel calibration instead.

## Known limitations

Physical FM-1/controller/audio validation remains pending. Direct USB controllers/pedals require powered host hardware; the FM-1 does not supply VBUS. USB role switching remains manual and Bluetooth is removed. Vital/Korg imports remain approximations. Factory-program recall waits for stopped/quiet operation; it is not a gapless program transition. USER WT remains silent until populated. The shared voice budget and release tails can cause voice stealing. Older firmware does not understand the new solo compensation/extended trim; use a compatible backup when downgrading.
