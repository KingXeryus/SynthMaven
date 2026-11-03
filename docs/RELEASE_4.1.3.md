# SynthMaven 4.1.3 — sound lists that fit the screen

4.1.3 is the 4.1.1 firmware with fixed web and iPad editors. If you run 4.1.1 or 4.1.2, you don't need to reinstall the firmware: reload the web editor, or update SM FM1 to 4.1 (25) in TestFlight.

## Fixes in the editors

- **Search results on the same screen:** in the Library, Device sounds and My library put their search and filters on one line, and the results start right below them. A results page holds as many sounds as fit on the screen, so you no longer had to press Next page to see any.
- **Filters apply at once:** changing the search, engine or source shows the new results immediately, on the screen you are looking at.

## Validation

Checked in a browser against the editor's mock FM-1: searching "bass" in Device sounds narrowed 617 sounds to 48, the PROPHET engine to 12 and FM6 to 5, each shown under the filters on a single screen.
