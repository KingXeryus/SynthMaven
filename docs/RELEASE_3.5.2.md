# SynthMaven 3.5.2 — iPad USB detection and personal factory defaults

SM FM1 now recognizes the FM-1 when iPadOS retains an older USB MIDI name such as **SoundMaven**. Discovery checks the physical USB MIDI driver and device identity as well as the visible name, pairs input/output from the same MIDI entity, and retains firmware-protocol validation. Unrelated virtual SoundMaven ports are not selected by name alone.

## Firmware and defaults

- Correct USB control endpoint handling: zero-length status packets no longer replay stale SETUP bytes; malformed short SETUP packets are rejected.
- Restore the working program without replacing committed saved-program banks, and accept valid settings with tagged CPU-budget preferences.
- Incorporate the verified owner backup as factory defaults. Saved P1 and P3 replace their factory records; the remaining slots retain the existing user-curated factory fallback programs. Active native sounds, effects, mixes, splits and scenes are preserved.
- Factory preferences use COMPUTER USB role and stereo USB Audio **IN+OUT**. Existing valid saved preferences remain respected until a factory reset/update.
- GLO page 2 is clearly labelled **SYSTEM**. The illustrated guide retains one page per screen, with knob/button explanations and step-by-step workflows.

## Installation and recovery

1. Export a complete backup before flashing: installation resets device settings and musical storage to factory defaults.
2. Set USB role to COMPUTER and use the installer with a data cable. Keep power connected until writing and verification finish.
3. On iPad, use the corrected SM FM1 3.5.2 editor and choose Connect → USB cable. Reconnect the cable with the iPad unlocked if needed.
4. Use the included operating-system recovery tool if normal installation is unavailable. Restore only compatible backups.

## Validation and limitations

The installed candidate passed the iPad USB read-and-edit test, confirmed by the owner. iPadOS logs showed successful MIDI input/output initialization and USB audio enumeration; the Mac exposed two audio input and two output channels. All persistent backup objects matched their original CRCs after restoration. USB endpoint/descriptor, malformed-packet, backup, settings, persistence, firmware UI, Swift MIDI discovery and native bridge tests passed locally; the release workflow also runs the blocking host/web regression suite and target memory checks.

This is not a new audio-quality or sustained-performance certification. Four parts and independent drums share voice/processing resources. Bluetooth remains excluded. USB controller hosting requires suitable external power/adapters and switching away from COMPUTER role.

Download the updated illustrated PDF and SHA256SUMS here or from the live installer. Release firmware is built and attested by the required shared builder. Attribution and licences are included in the licence archive.

## Editor arranger workflow

- Four visible melodic sequencer lanes correspond to the four program layers.
- Four note selectors per step cover the 88-key piano range (A0–C8); existing imported out-of-range notes remain intact. Drum-pattern controls stay on the independent Drums page.
- Song sections run left to right, with a separate pattern choice for each layer, repeat counts, duplication, move-earlier and deletion. Both editors share this layout.
- The illustrated guide includes the melodic sequencer and updated song timeline.
