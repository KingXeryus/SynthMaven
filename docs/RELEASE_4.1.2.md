# SynthMaven 4.1.2 — editor fixes for layers and envelopes

4.1.2 is the 4.1.1 firmware with fixed web and iPad editors. If you already run 4.1.1, you don't need to reinstall the firmware: open the web editor again, or update SM FM1 to 4.1 (24) in TestFlight.

## Fixes in the editors

- **Layer octaves work on every layer:** Oct buttons were disabled on every layer except the selected one, because the editor drew them before it knew the FM-1 can change any layer. Each layer card now has one ‹ Oct n › row, and the MIDI routing screen has the same control for each layer.
- **ENV on Prophet, FM6 and CZ-1:** these engines play their own envelopes and ignore the shared ADSR, as on the FM-1, so the editor's ENV controls did nothing on them. On a Prophet, the Envelopes page and the Perform ENV knobs now edit its filter and amplifier envelopes, live. FM6 and CZ-1 point to their own envelope editors.
- **Device sounds filters:** the Library's Device sounds list has a search box, an engine filter and a Factory / User filter, like My library.
- **Editor version:** the editor shows its version (4.1.2), so you can tell which one is open.

## Envelopes while layering

With layering on, ENV changes only the selected layer. If other layers play the same keys, they can hide the change: in testing, a quiet layer under two loud ones changed attack from 40 ms to 1 s and release from 50 ms to 0.9 s, but the whole sound hardly changed. Lower or mute the other layers, or give them other key ranges, to hear one layer's envelope.

## Validation

Host tests drive the editor against its mock FM-1: Prophet envelope edits reach the patch, Oct on a layer that isn't selected moves its transpose, and the Device sounds filters narrow the list. On an FM-1 running 4.1.1, ENV changes were recorded from its USB audio, with layering off and with it on.
