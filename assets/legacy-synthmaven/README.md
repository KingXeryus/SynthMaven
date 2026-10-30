# SynthMaven factory restoration sources

Source: KingXeryus/SynthMaven, commit `e49239e` (older SynthMaven firmware).
The five original engine files were clean in the donor checkout at export.
Original copyright and GPL-3.0-only notices are retained in each source excerpt.

`catalogue.json` preserves 107 factory sounds from ANALOG, DIGITAL, VOICE,
STRINGS and JUNO, plus the eight captured factory programs. `parameter_names`
records the original 126-field layout: conversion must use names, not assume
that a parameter index has the same meaning in the Melodee-derived firmware.
The eight programs originate from the public `assets/programs/device-capture.json`
factory source, not the private user backup.

These inputs are mapped into the active 3.5 classic engines and factory programs.
Engine IDs are appended (23–27), preserving Melodee's existing stored identities.
Parameter conversion uses names. Poly-unison modes use the available unison mode;
old insert/effect sends and per-sound loudness trims are not applied blindly to the
new mixer. JUNO's stereo chorus is retained as a fixed per-part post process.

Provenance check: the public captured 1,748-byte programs, each extended with
52 zero bytes for PBK3 fine-tuning defaults, exactly match the exported 14,400-byte
factory binary used for the decoded catalogue. This is not private backup data.
