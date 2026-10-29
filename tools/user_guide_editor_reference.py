# SPDX-License-Identifier: GPL-3.0-only
# Editor-only settings, verified against web/editor.html; native menu captures are above.
EDITOR_SECTIONS=[
 ('Editor session and sound tools',[
 ('Connect / Read','Use COMPUTER USB role, choose the FM-1 ports and allow SysEx. Read the current state before editing an existing sound. Mock mode previews the interface and does not contact hardware.'),
 ('Part / engine / preset','Choose the intended part before editing. The four-patch browser browses sounds; a complete factory program is recalled separately from Programs. Engine-specific cards appear only for the matching engine/protocol.'),
 ('Undo / Redo','Undo returns through sound-edit history; Redo reapplies an undone edit. Ctrl+Z and Ctrl+Shift+Z are the editor shortcuts. Save wanted versions before large edits/imports.'),
 ('Compare','Temporarily compare with the loaded/original sound. Return to the edited state before continuing work so you know which state is being changed.'),
 ('Set A / Set B / A-B / Morph','Capture two working sound states, switch between them, or move Morph from 0 to 127. Continuous values interpolate; discrete choices switch. Across different engines, the engine changes rather than both running as extra layers.'),
 ('Scope / Amount / Random','Choose which section to randomize and how far (0-127; initial 40). EDIT, +ENV, ALL, ENV, MOD, FX and OPS restrict the operation. Keep a saved starting version and use Undo if the result is unsuitable.'),
 ('What changed','Expand the differences view to compare edited parameter values with the reference. This is an edit aid, not a device save or backup.')]),
 ('HYBRID: oscillator settings',[
 ('Read / Send / Live','Read the selected part patch. With Live ON, supported changes send immediately; with Live OFF they remain staged until Send. Init and factory Load replace the staged patch; Export/Import use patch JSON. Name holds up to ten characters.'),
 ('OSC 1-3 Type','OFF, SAW, PULSE, TRI, SINE, MORPH, SUPER, FM, SYNC, FOLD or NOISE. OFF removes that source; the three oscillator levels form the source mix.'),
 ('Coarse / Fine / Level','Coarse -48 to +48 semitones; Fine -64 to +63 cents; Level 0-127. Coarse/fine change source pitch relative to the key, not the selected master scale.'),
 ('Shape','0-127, dependent on type: PULSE width; SINE feedback; MORPH sine/triangle/saw/square position; SUPER detune; FM index; SYNC pitch over oscillator 1; FOLD amount; NOISE darkness. Some types have no shape behavior.'),
 ('Reset / Fixed','Reset restarts phase for repeatable attacks. Fixed makes oscillator frequency independent of ordinary note tracking; use it deliberately for fixed resonances/percussion.'),
 ('Ring 1 x 2 / Drift / Drive','Each 0-127: ring modulation between sources 1 and 2, analog-style variation, and input saturation. Start with modest levels and compare at matched output volume.')]),
 ('HYBRID: filters and routing',[
 ('Route','SER sends through filters in series; PAR processes parallel paths; SPLIT uses the engine split routing. These are internal synthesis routes, distinct from the four-part keyboard split.'),
 ('Balance F1-F2','0-127, shown for PAR. Adjusts the relative parallel filter contribution.'),
 ('Filter 1 / Filter 2 Type','OFF, LP12, LP24, BP, HP, NOTCH, PEAK or MOOG. The compact firmware FILTER page addresses filter 1; the editor exposes both and their route.'),
 ('Cutoff / Resonance','Both 0-127 internally, cutoff displayed in Hz. Cutoff sets the spectral boundary; resonance emphasizes it. High resonance/drive can change level sharply.'),
 ('ENV2','-64 to +63: signed depth from HYBRID envelope 2 to cutoff. Negative values reverse the contour.'),
 ('Key / Vel','Key -64 to +63: cutoff tracking across the keyboard. Vel 0-127: velocity influence on the filter. Test low/high notes and soft/hard playing.'),
 ('Output compensation','The separate FILTER GAIN compensation switch estimates attenuation for resonance/drive changes. It does not measure integrated LUFS or replace a deliberate mixer balance.')]),
 ('HYBRID: modulation and global settings',[
 ('ENV2 / ENV3','Each has A, D, S, R stored 0-127. A/D/R display time; S displays percent. ENV2 is available directly to filter modulation; both can be matrix sources.'),
 ('LFO2 / LFO3 Wave','SINE, TRI, SAW, SQR, S&H or SMTH. KEY restarts with a note; SYNC changes rate to a tempo division; FADE delays the full modulation depth.'),
 ('LFO Rate / Fade','0-127 each. Synced rate choices cover 1/32, triplets, 1/16 through whole notes, dotted/long divisions and up to eight bars. LFO1 remains on the shared LFO page.'),
 ('16-slot matrix Source','ENV2, ENV3, AENV, LFO1-3, VEL, KEY, MW, AT, BEND, EXPR, RAND, M1-4 or ONE. An empty source/destination leaves a slot inactive.'),
 ('Via / Destination / Amount','Via scales a source by another controller; blank leaves it unscaled. Destinations include global/per-oscillator pitch, shape and levels, ring, cutoff(s), resonance(s), drive, amp, LFO2/3 and filter balance. Amount -64 to +63 sets signed depth.'),
 ('Bend / Level','Bend 0-24 semitones; patch Level 0-127. These belong to the custom HYBRID patch and are separate from the program mixer fader.'),
 ('Save a custom patch','After sending and checking the sound, save the containing program so its patch bytes travel with it. Also export the patch JSON and editor library entry for reuse.')]),
 ('FM6: patch and voice controls',[
 ('Read / Send / Live','Read and edit the selected FM6 patch. Live controls immediate sending. Bank slot Load/Store/Erase operates FM6 patch storage; it is different from saving a four-part program or a playable user sound.'),
 ('Name / ALG / FB / OKS / TRNSP','Name up to ten characters. ALG chooses 1-32 algorithms; the display identifies carriers/feedback operator. FB 0-7 sets feedback. OKS 0/1 sets oscillator key synchronization. TRNSP 0-48 is the patch transpose encoding with center at 24.'),
 ('LFO Wave / Speed / Delay','Wave TRI, SAW-, SAW+, SQR, SIN or S&H. Speed and Delay 0-99 control rate and delayed onset.'),
 ('PMD / AMD / PMS / Sync','Pitch-mod depth and amplitude-mod depth 0-99; pitch-mod sensitivity 0-7; key sync 0/1. Operator AMS also determines amplitude-mod response.'),
 ('Pitch envelope','Four rates PR1-PR4 and four levels PL1-PL4, each 0-99. These are DX7-style rate/level stages, not ordinary ADSR time values.'),
 ('Import / Export / Init','Import supported DX7 single/bank SysEx or binary data, inspect a voice, then send/store it deliberately. Export patch or bank keeps a reusable copy. Init replaces the current patch with a starting voice.')]),
 ('FM6: every operator field',[
 ('R1-R4 / L1-L4','Each 0-99: four envelope rates and four target levels. Rates control speed; levels control stage amplitude. A carrier envelope shapes loudness, a modulator envelope shapes brightness.'),
 ('BP / LD / RD','BP 0-99 is keyboard-scaling breakpoint; LD/RD 0-99 set scaling depth below/above it.'),
 ('LC / RC','Left/right scaling curves: -LIN, -EXP, +EXP, +LIN. These determine how operator level changes away from the breakpoint.'),
 ('RS / AMS / KVS','Rate scaling 0-7 changes envelope speed with key pitch. Amplitude-mod sensitivity 0-3 controls LFO amplitude response. Key-velocity sensitivity 0-7 controls response to playing force.'),
 ('OL','Operator output level 0-99. A carrier contributes to output; a modulator changes the spectrum of the operators it feeds.'),
 ('MODE / FC / FF / DET','MODE is RATIO or FIXED. FC 0-31 sets coarse frequency; FF 0-99 fine frequency; DET 0-14 is centered at 7. The frequency column displays the resulting ratio/frequency.'),
 ('Imported bank selection','Click a voice to inspect/edit it. Tick voices to assign bank order B1 onward; Select first / None changes the selection. Choose bank-only or playable user-preset destinations before Store cartridge. Keep the source SysEx file.')]),
 ('FMX: routing and operator settings',[
 ('Read / Send / Live / Init','Same staged-versus-live patch workflow as HYBRID. Load a factory patch or Init, then edit. Import/Export use FMX JSON. Name is up to ten characters; Store To writes the selected FMX bank slot.'),
 ('Algorithm template / routing grid','A template supplies starting routes. Rows are TO operators; columns are FROM operators. Each matrix amount 0-127 sends the column operator into the row operator. Diagonal cells are feedback. Manual edits clear the template selection.'),
 ('OUT','Each operator output amount 0-127. At least one audible operator must reach output; routing modulation alone does not guarantee an audible sound.'),
 ('Wave / Mode','Wave SINE, TRI, SAW, SQR, HALF, ABS, QUART, PULSE, SIN8 or NOISE. Mode PM, FM or RING changes the modulation method.'),
 ('Fixed / Ratio / Fine','Fixed OFF uses ratios 0.5, 1-31. Fixed ON uses bases 1, 10, 100 or 1000 Hz. Fine 0-99 adds a ratio offset or a fixed-frequency multiplier.'),
 ('Level / ADSR / Vel / KSC / DTN','Level, ADSR stages, velocity and key scaling 0-127; detune stored 0-14, displayed -7 to +7. Each of six operators has these controls.'),
 ('PEG / PTIM / Level','Patch pitch-envelope amount, pitch-envelope time and output level (0-127). PEG is centered at 64 and displayed in semitones. Save the program after sending the patch.')]),
 ('Layers, scales and modulation sequencer in the editor',[
 ('Read mixer / part strips','Read the four-part mixer, then set each part level, pan, mute/solo, enabled state, keyboard split side/boundary and transpose. Layer program sounds ON combines enabled routed parts; OFF uses the focused part.'),
 ('Tuning system / Root / Apply from octave','Choose master temperament/maqam/custom tuning, its root, and C0-C8 lower boundary. Notes below the boundary retain equal-tempered intervals. This does not limit ordinary note quantization or chord generation.'),
 ('Apply master tuning / A4','Per-part checkbox ON by default; OFF bypasses master tuning for that sound. A4 400-480 Hz is the shared reference. QNT ALL applies the focused ROOT/SCL across all four parts and MIDI independently.'),
 ('Load .scl / Send','Choose a Scala tuning file, review the imported scale information and send it. Keep the original file and save the program/backup containing your setup. A Scala file provides tuning, not a new sound engine.'),
 ('Mod-sequencer Read / Send / Live','Read four 16-step lanes; edit their step values -8 to +7. Live sends edits, otherwise Send commits the staged lane data.'),
 ('Shape / Apply','Apply RAMP+, RAMP-, TRI, SINE, SQR, RND, FLAT, INVRT or SHIFT to the chosen modulation lane. Rate, length, smoothing and sync/key restart are set on MOD SEQ 2; route SEQ1-4 through the matrix.')]),
 ('Sequencer editor: notes and performance lanes',[
 ('Reload / Clear / Length','Reload reads the selected part pattern; Clear removes its notes. Length is 1-64 steps. Keep a saved version before destructive transformations.'),
 ('Note grid / piano roll','Select a note or step, adjust pitch and duration, and use the displayed selection/roll controls to choose the editing range. Read the current step detail before replacing a chord member.'),
 ('Copy/paste / shift / transpose / reverse / double','These transform the selected pattern/range. Double repeats/extends material within the 64-step limit. Check ties and pattern length after transformations.'),
 ('Chord / accent / slide tools','Choose a chord transform for selected notes, or switch accent/slide flags on/off for the selected range. These edit pattern data, separate from the live CHORDS generator.'),
 ('Velocity / nudge / ratchet / hit level lanes','Velocity sets note force; nudge offsets timing; ratchet repeats hits; hit level shapes individual step strength. Draw the lane or enter a range value and Apply.'),
 ('Step list','Expand the list for a numerical view of notes/step attributes. It is another view of the same working pattern, not a second saved sequence.')]),
 ('Sequencer generators and pattern library',[
 ('Key / scale / base','Generator root, scale and base register shape generated note choices. These are generation settings, not an automatic replacement for master maqam tuning.'),
 ('Euclidean','Hits 0-64, steps 1-64, rotation 0-63, note text (initial C3), accent and merge options. Make distributes hits; Merge adds to existing notes instead of replacing the selected material.'),
 ('Random melody','Density, motion, ties, accent, slide and chords each 0-100; octaves 1-4; seed 0-999999. Make uses those settings; Dice changes the random seed/result.'),
 ('Text/pattern presets','Choose a preset or enter supported pattern text, then Make. Check the preview/result before saving the program.'),
 ('Humanize','Velocity variation up to +/-64 and nudge up to +/-32 in the generator controls. Apply introduces bounded variation; it does not make all notes equally loud.'),
 ('Pattern library','Name up to 40 characters, Save/Load/Delete, export one pattern or the bank, and import pattern JSON. This browser library is separate from named device programs and should be exported for backup.')]),
 ('Library, user bank, imports and backups',[
 ('Search / engine / tags / sort','Filter and order browser library entries. Rename changes its display name; Tags classify it; Duplicate makes another library record; Delete removes that record.'),
 ('Audition / To Slot / Save to Library','Audition loads a working sound for listening. To Slot writes the chosen device user-sound destination. Save to Library captures a browser entry; it is not a four-part program save.'),
 ('Import / Export patch / Export library','Patch JSON and full library JSON support reuse. Importing files into the browser does not automatically populate every device storage area.'),
 ('Device bank Load / Store / Get / Erase','Load recalls a user sound; Store writes the working sound to that slot; Get copies it into the editor library; Erase clears the selected device user sound. Refresh rereads the bank. Export Bank keeps an external copy.'),
 ('Vital / minilogue xd / DX7','Vital and minilogue xd imports translate supported synthesis settings into HYBRID approximations. Review unsupported-feature reports before sending. Korg custom-unit binaries cannot execute on FM-1. DX7 import targets FM6.'),
 ('Wavetables','Upload supported user wavetable data, then select the corresponding user table in WAVE. USER WT intentionally stays silent until data has been uploaded; it has no automatic audible factory fallback.'),
 ('Programs / full backup','Recall a read-only factory program or load/save a named user program. Full backup exports/restores device storage; do not confuse this with Export Library. Stop playback/release notes and keep the original backup file.')]),
 ('Controller and pedal setup: complete workflow',[
 ('Automatic controller identity','Host mode reads vendor/product/interface identity. Alesis V/VI/V Mini, Novation Launchkey/FLkey/Impulse/modern Launchpad, Arturia MiniLab/KeyLab/KeyStep/BeatStep/MicroLab, Akai MPK/MPD/LPD8/LPK25/MIDImix/APC and Mackie families have documented handling.'),
 ('Standard / MCU / HUI override','Automatic selects the profile; Standard MIDI disables surface decoding; MCU or HUI forces the supported control-surface interpretation. DAW port AUTO or 1-16 selects its USB MIDI cable. Musical and DAW traffic remain separate.'),
 ('MCU/HUI supported controls','First four channel strips map faders to part volume, encoders to pan, and switches to mute/solo/select/record arm. Play/Stop/Record controls transport. Feedback updates supported controls; touched motor faders are not overwritten.'),
 ('Pad mappings / limits','Supported modern Launchpads use a chromatic 8x8 C2-D#7 layout and feedback. APC Mini uses chromatic pads and the first four volume faders. Unknown devices keep class-compliant standard MIDI. Extra strips, scribble strips and arbitrary DAW scripts are not implemented.'),
 ('Pedal Learn / Accept / test','Enable patch switching, choose the action, arm Learn, press the pedal, inspect CC/PC/channel/number/value, Accept, then press again to test. Cancel discards the candidate. Duplicate/conflicting assignments are refused.'),
 ('Repeat suppression / USB role','Choose 0-500 ms suppression only for unwanted duplicates; zero is OFF. CC release re-arms a tap; identical PC taps inside the window can be suppressed. Return to COMPUTER for editing/updates; host mode needs powered hardware and physical compatibility validation.')])]
for title,rows in EDITOR_SECTIONS:
 CURRENT=title;Y=newpage(title,'Editor-only feature reference',key='editor-ref-'+str(EDITOR_SECTIONS.index((title,rows))))
 Y=para('These controls live in the web editor. The preceding device-screen reference shows the corresponding on-device controls where available. Editor-only panels are described here; native LCD captures are not presented as browser screenshots.',Y,size=9.6,color=GRAY)
 for label,desc in rows:Y=flow('<font color="'+RED+'">'+html.escape(label)+'</font><br/>'+html.escape(desc),Y)
