# SynthMaven 3.5.4 — MIDI routing, pad recording and pooled polyphony

Assign each layer and the drummer to its own MIDI input and channel, record pads, and hold more layered notes.

## MIDI routing

- **Settings → MIDI → Routing** assigns an input (Any, USB or TRS) and a channel (Off, 1–16 or All) to layers 1–4 and the Drummer. It is off by default, so `ROUT` behaves as before until you turn it on. Routing is saved on the FM-1 and applies in COMPUTER and HOST.
- Layers on the same input and channel split or stack by note range.
- The Drummer plays and records on the first part using the DRUM engine, so that part is not available as a layer.
- MIDI Learn adds Transport Play / Stop, Record (selected part) and Record (drummer).
- The web editor forwards several controllers on a computer, each to its target's channel. The iPad app still forwards one at a time.

## Polyphony

- The default BUDGET is now 32 points, and the maximum is 64.
- ANALOG, DIGITAL, VOICE, STRINGS, JUNO, HYBRID, WSEQ and VPS can use up to 16 voices a part from the shared pool. UNISON keeps its previous voice count.
- Layered MIDI notes can use 32 tracking slots, up from 24, so four stacked layers hold 8 notes rather than 6.
- More voices use more CPU. If a heavy program drops notes or crackles, lower BUDGET in GLO → TUNE/CLOCK.

## Installation

1. Export a complete backup first: installation resets settings and musical storage to factory defaults.
2. Set USB ROLE to COMPUTER and install with the web installer and a data cable. Restore your backup afterwards.

## Validation and limitations

Host and web tests cover routing, splits, pad recording, transport actions and settings. **This release has not been tested on FM-1 hardware,** including CPU load at the new default budget. HYBRID sounds can behave differently past 4 voices because notes now ring on. The FM-1 settings screens have no routing page: set routing in the web or iPad editor while in COMPUTER. Bluetooth remains excluded.
