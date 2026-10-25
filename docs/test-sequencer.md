# SynthMaven 1.1: testing the sequencer on an FM-1

The sequencer of SLOOP 2.7.2, brought to SynthMaven's single instrument. Everything below passes the host tests; this list is for checking it on a real FM-1. If a step fails, note its number and what the screen shows (a photo helps).

## 1. The NOTES page (SEQ tapped)

1. Make a short pattern: a bass with a long note and a chord (SEQ tapped, play notes on the keys: each chord or note goes into the step under edit, then the next step).
2. The page is called **NOTES**: the notes are bars as long as they last, the note under edit is outlined in white with its name, a line moves while playing.
3. **KNOB 1** walks from note to note; on a chord, its notes one by one (PITCH shows 1/3, 2/3...); it skips a long note's ties.
4. **KNOB 2** changes the pitch of that note only. **KNOB 3** makes it longer or shorter. **KNOB 4 FORCE**: GHOST (dark), SOFT (hollow), NORM, HARD, ACC (a white cap); you hear the difference when it plays.
5. **PRESETS** zooms: 8, 16, 32, 64 steps. The sound does not change.
6. **EDIT tapped** deletes the note under edit (a chord keeps its others); **EDIT + OCT-** brings it back.
7. **OCT+ tapped**: the keyboard octave, as before. **OCT+ held** half a second: the note's sheet (SLIDE, NUDGE, FILL, MOVE, COPY, DELETE). KNOB 1 picks the row, KNOB 2 sets it, OCT+ does it, OCT- closes.
8. **MOVE** + KNOB 2: the note moves a step per detent; against another note: *NO ROOM*. **COPY** + OCT+: a copy right after it. **DELETE** + OCT+: the note goes.
9. With a **MIDI keyboard**: on the NOTES page, a chord played on the keyboard writes the step, letting go moves on.
10. MENU (HOME held) > **SEQ** > **STEP ADVANCE OFF**: play a wrong chord, let go: the cursor stays; play the right one: it replaces it. Power off and on: OFF is kept. Set it back to ON.

## 2. EDIT, ARP and GLO held (layers again)

1. **EDIT held**: the ERASE screen. Stopped, EDIT + a key: that note leaves the whole pattern. Playing, hold EDIT + a key for a bar: it leaves the steps the playhead passes.
2. EDIT + **KNOB 1** SHIFT (the pattern one step later / earlier), **KNOB 2** LENGTH (x2 / half), **KNOB 3** TRANSPOSE. EDIT + **OCT-** undo, **OCT+** redo.
3. **ARP held** + a key, playing: the note repeats on the grid (ARP + KNOB 1: the rate). While recording, the repeats are written as ratchets.
4. **GLO held**: key 9 held = a fill (steps set to FILL in the sheet play, NO FILL steps are silent); key 10 = the next bar is a fill; key 16 = tap tempo; KNOB 1 = the sound's level. Keys 1 to 8 do nothing.
5. **SEQ held + a step held + KNOB 2**: that step's velocity (1 to 127).
6. **FX** and **SAVE** held: the keys still play, as before.

## 3. Recording

1. A pad (POLY): REC, hold a note, play another on top, let the second go, then the first later. On playback the first note goes on to where you let it go.
2. A chord built note by note (hold A, then B, then C), all let go together: each note starts on its step, they end together.
3. A bass in VOICE **LEGATO**, GLIDE 0: REC, play a line where each note is pressed before the previous one is let go. On playback the notes follow each other with no new attack. Played detached, each note starts again.
4. With a DAW sending MIDI clock (GLO > SYSTEM > SYNC = USB) at 140: the BPM shows 140 and stays there.

## 4. Nothing broken

PLAY / STOP, the arpeggiator, SEL held (key and chords), the sound pages, user presets, the web editor's Sequencer tab.
