# SynthMaven 4.1.1 — an octave per layer, cleaner layered chords

4.1.1 is the first published build of 4.1: the 4.1 tag's firmware didn't fit the FM-1's app area and was never released.

## New in 4.1

- **An octave per layer:** with layering on, OCT- and OCT+ on Home move only the selected layer, an octave at a time (two octaves either way). Press both together to put that layer back to 0. Each layer can play the same keys in its own octave, for example a bass two octaves under a pad. The OCT lights show when the selected layer is shifted down or up. With layering off, the octave buttons work as before.
- **Octave buttons in the editors:** every layer card on the layer mixer has Oct − and Oct + at the top.
- **The split stays on the keys:** a layer's octave no longer moves the split point.
- **Prophet envelopes on screen:** FLT ENV and AMP ENV draw the envelope instead of the oscilloscope, as the other engines' ENV pages do.
- **Sound search in the editors:** the Sounds page has a search box and a Factory / User filter, like the Library.

## Fixes

- **Layered chords no longer duck:** with layering on, the master is turned down 6 dB before the limiter (switching layering on or off while notes sound changes the level at once). A held chord no longer drops when other layers come in. Layered programs play a little quieter; turn up your amplifier to match.
- **Programs 3–8 after restoring a backup:** restoring a backup with empty program banks made those programs fail to load. They now load as the factory programs.
- **Library filters:** the engine list shows only engines in your library, and entries saved without tags no longer stop the list updating.

## Installation

1. Export a complete backup first: installation resets settings and musical storage to factory defaults.
2. Set USB ROLE to COMPUTER and install with the web installer and a data cable. Restore your backup afterwards.

## Validation and limitations

Host tests cover the layer octaves, the limiter, program loading after a restore and the editor filters. This firmware, built as the release is, was installed on an FM-1: a backup restored and all eight programs loaded. With all four layers sounding, three-key chords stayed inside the audio deadline on seven of the eight programs (P6 missed it once); chords of six or more keys miss it now and then on most programs, as in 4.0, so very large layered chords can glitch. The voice budget while layering stays at 16 points: 20 and 24 measured worse. The ducking fix was measured on the host, not on the FM-1. P7 is the loudest layered program and still pumps the limiter a little. The iPad app is 4.1 (23), matching this firmware.
