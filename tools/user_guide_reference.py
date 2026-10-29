# SPDX-License-Identifier: GPL-3.0-only
# Executed by make_user_guide.py with its layout helpers and current release metadata.
INVENTORY=json.loads((ROOT/'assets/guide/screen-inventory.json').read_text())
HELP=json.loads((ROOT/'assets/guide/control-help.json').read_text())
HELP.update({'MODL':'Select the synthesis model inside MACRO; the engine remains MACRO. Each model interprets HARM, TIMB and MRPH differently.','TRK':'Keyboard tracking of the noise resonator frequency.','VIB':'Built-in chip vibrato depth.','CHRD':'Sympathetic-string interval set; the named choices select the resonance relationships.','SYMP':'Level of sympathetic strings ringing above the played note.','CLK':'Clock/sample-hold rate of the digital noise model.','DCY':'Chip stepped decay: the range selects constant, one-shot decay or looping decay behavior.','WAV#':'Index into the LOFI waveform RAM shapes.'})
OVERRIDES={
 ('PHYS','MODEL'):'Physical synthesis model: MODAL resonator, STRNG plucked string, MEMB membrane or SYMP sympathetic strings.',
 ('PHYS','BEND'):'Pitch drop after the membrane strike, up to twelve semitones.',
 ('PHASE','ENV'):'Envelope amount applied to phase-distortion waveform shaping (DCW).',
 ('PHASE','LINE'):'Combine the two phase-distortion waveforms by mixing or ring modulation.',
 ('VOICE','TALK'):'Time taken to move between the two vowel positions.',
 ('VOICE','SHIFT'):'Formant shift in semitones; changes vocal character separately from the note pitch.',
 ('SAMPLE','TUNE'):'Sample playback transposition in semitones; this is not the master tuning system.',
 ('GRAIN','SRC'):'Source sample set used to generate grains.',
 ('GRAIN','SIZE'):'Grain duration; short grains can sound granular, longer grains smoother.',
 ('GRAIN','PTCH'):'Grain playback pitch offset in semitones.',
 ('FM6','MEG'):'Offset to the modulator envelope behavior of the underlying FM6 patch.',
 ('FM6','ALG'):'PAT retains the patch algorithm; a numbered value overrides it with a DX7-style algorithm.',
 ('FMX','TIME'):'Global offset to the FMX operator envelope times; individual operator envelopes remain available under OP ENV.',
 ('VECTOR','A'):'Select the source waveform at vector corner A.',('VECTOR','B'):'Select the source waveform at vector corner B.',('VECTOR','C'):'Select the source waveform at vector corner C.',('VECTOR','D'):'Select the source waveform at vector corner D.',
 ('VPS','D'):'Phase breakpoint/deformation position of the vector phase-shaping oscillator.',('VPS','V'):'Phase-shaping slope/ratio parameter; alters the waveform around the breakpoint.',
 ('WHEEL','SUB'):'Offset to the low organ drawbar group relative to the selected registration.',('WHEEL','BODY'):'Offset to the middle organ drawbar group.',('WHEEL','TOP'):'Offset to the upper organ drawbar group.',
 ('HYBRID','CUT'):'Offset to HYBRID filter cutoff relative to its stored patch; use the editor for the complete two-filter architecture.',
 ('HYBRID','RES'):'Offset to HYBRID filter resonance relative to its stored patch.',
 ('HYBRID','PTCH'):'Select the underlying HYBRID patch, including the Vital-derived factory bank.',
}
PURPOSE={
 'ENV':'Shape the amplitude contour of the focused sound.', 'ENV DEST':'Route the main envelope into filter, pitch and waveform shaping.', 'ENV 2':'Set the second envelope used as a modulation source.', 'ENV 2 LOOP':'Choose one-shot or repeating operation for envelope 2.', 'OP ENV':'Edit the envelope of the operator selected on FMX OP.',
 'LFO':'Set the first low-frequency oscillator.', 'LFO DEST':'Set LFO 1 destination depths.', 'MOD PRESET':'Apply an expressive modulation template.', 'MOD':'Set velocity, wheel and pressure behavior.', 'LFO 2':'Set the second modulation oscillator.', 'LFO 3':'Set the third modulation oscillator.', 'MATRIX':'Edit one of twelve source/destination/depth connections.', 'MOD SEQ':'Draw a sixteen-step modulation lane.', 'MOD SEQ 2':'Set modulation-lane timing, length, smoothing and restart.',
 'FX':'Set the focused part sends. Shared return processors are configured on later FX pages.', 'PART FILTER':'Apply a part-level DJ-style filter independently of the engine filter.', 'INSERT':'Choose and shape the focused part insert processor.', 'INSERT 2':'Blend the insert with the dry part.', 'DLY':'Set the shared delay return.', 'DLY 2':'Select shared delay topology.', 'REVERB':'Set the shared reverb return.', 'CHORUS':'Set shared chorus movement.', 'EQ+COMP':'Shape and compress the master bus.',
 'SEL':'Choose note scale, root, quantization and chord behavior. The SEL tap opens a special scale browser over this page.', 'SEL 2':'Set transposition, strum and voice leading.', 'FILTER':'Grouped engine filter controls; absent on engines without exposed filter controls.', 'FILTER GAIN':'Balance filter drive, optional output compensation and final trim.', 'EDIT 1':'Shape the engine source/model. These controls change with the selected engine.', 'EDIT 2':'Shape the remaining engine controls.', 'OP':'Select and tune an FMX operator.', 'OP 2':'Set FMX operator level and performance response.', 'ROUTE':'Connect FMX operators; FROM modulates TO, with independently controlled output.', 'FMX':'Set operator detune/frequency behavior and the FMX pitch envelope.', 'VOICE':'Choose polyphony/mono articulation and glide.', 'CHARACTER':'Choose allocation, detuning, vintage variation and stereo character.',
 'GLOBAL':'Set internal tempo, swing, metronome click and global pitch offset in cents.', 'MASTER':'Apply shared lo-fi texture and master DJ filter.', 'TUNING':'Choose the master scale, octave boundary, root and per-part tuning bypass.', 'TUNING 2':'Set the A4 frequency reference.', 'CONTROLLER':'Select automatic/MCU/HUI/controller handling and the MIDI cable/port.', 'SYSTEM':'Configure MIDI output, sync, routing and information display.', 'MIDI':'Configure channels, pitch bend and onboard local playing.', 'MIDI 2':'Configure program changes, CC output and breath control.', 'PATCH MIDI':'Enable incoming patch switching on its supported input.', 'KEY MIDI':'Choose keyboard input source and channel.', 'PEDAL LEARN':'Capture and confirm a pedal message for the selected relative action.', 'SCENES':'Capture and recall small changes inside the current program.', 'SETLIST':'Build an ordered list of saved program references.', 'PEDAL MAP':'Edit a relative action assignment manually; use the action selector/status display to identify the assignment.', 'PATCH UP':'Manually set the Patch Up MIDI assignment.', 'PATCH DOWN':'Manually set the Patch Down MIDI assignment.', 'BANK UP':'Manually set the Bank Up MIDI assignment.', 'BANK DOWN':'Manually set the Bank Down MIDI assignment.', 'USB ROLE':'Choose computer-device or pedal/controller-host operation.', 'LAYER ROUTE':'Enable the part and choose its keyboard range.', 'LAYER MIX':'Set the part volume, pan, mute and solo.', 'LAYER PITCH':'Set part transpose and enable/disable four-part layering.',
 'PROGRAM':'Name, load and save one of eight complete four-part programs.', 'TOOLS':'Clear or initialize working data after confirmation.', 'ARP':'Generate ordered note patterns from held keys.', 'ARP 2':'Set variation, hold and note ordering.', 'SPICE':'Add controlled variation to generated notes. TURNG changes SPICE/DICE to MUTAT/SEED.', 'NOTES':'Edit notes in the focused part pattern.', 'PATTERN':'Set pattern timing and length.', 'CHORDS':'Generate scale-aware chord voicings; opened by holding ENV for three seconds.'}
FAM_ORDER=['ENV','EDIT','LFO','FX','SEL','CHORDS','GLO','SAVE','ARP','SEQ']

def flow(text,y,size=10.3,color=INK):
 st=ParagraphStyle('measure',fontName='Guide',fontSize=size,leading=size*1.42)
 p=Paragraph(clean(text),st);_,hh=p.wrap(CW,2000)
 if y-hh<65:y=newpage(CURRENT+' - continued','Complete screen reference')
 return para(text,y,size=size,color=color,gap=9)

def path_for(p):
 if p['family']=='CHORDS':return 'HOME > select the part > hold ENV for 3 seconds > release.'
 return f"HOME > SELECT the part > tap {p['family']} > tap {p['family']} again until {p['title']} appears."

def controls_for(p):
 cs=p['controls']
 if p['title']=='NOTES':return [('NOTE','Move between notes/chord members and empty steps.'),('PITCH','Edit the selected MIDI pitch (1-127). On an empty step, the first turn inserts the last-used note.'),('LEN','Extend/shorten the selected note in steps, stopping at an occupied note or pattern end.'),('FORCE','Choose GHOST, SOFT, NORM, HARD or ACC articulation.')]
 return [(c['label'],c) if c else ('Unused',None) for c in cs]


CURRENT='Buttons: primary and secondary functions'
Y=newpage(CURRENT,'Panel reference',key='panel-buttons')
Y=table([['Button','Tap / primary action','Hold / secondary action'],
 ['HOME','Return to four-part HOME','3 seconds: system sections'],['EDIT','Engine/filter/voice pages','2 seconds: mixer preview; 3 seconds: latch'],['ENV','Main/second/operator envelopes','3 seconds: CHORDS'],['SEL','Scale browser; SELECT chooses scale','3 seconds: custom-scale editor'],['LFO','LFO/modulation/matrix pages','No separate held screen'],['FX','Sends, insert and shared processors','No alternate SynthMaven punch screen'],['GLO','Global/tuning/MIDI/layer pages','2 seconds: toggle layers once; release returns'],['SAVE','PROGRAM/TOOLS pages','No alternate song/section layer'],['ARP','Arpeggiator pages','2 seconds: repeat preview; 3 seconds: latch'],['SEQ','NOTES/PATTERN pages','2 seconds: step layer; 3 seconds: latch'],['PLAY','Start/stop transport; free take: discard','No menu latch'],['REC','Arm/start/finish recording','0.7 seconds + 1.3-second clear ring'],['OCT-','Lower keyboard octave; settings: close','With OCT+ at startup: panel calibration'],['OCT+','Raise keyboard octave; settings: confirm','On NOTES: 0.4 seconds opens note sheet']],Y,[52,205,CW-257])
Y=box('Both octave buttons together reset the playing octave. OCT- alone during power-up enters USB rescue. Inside a menu or held function, octave buttons can have the on-screen contextual actions instead of changing pitch.',Y)
CURRENT='Knobs: know the current context'
Y=newpage(CURRENT,'Panel reference',key='panel-knobs')
Y=table([['Context','SELECT','ALGORITHM','PRESET'],['HOME','Part 1-4','Engine','Preset inside engine'],['Ordinary menu page','Previous/next page in family','No sound browsing','No sound browsing'],['SEL scale browser','Master tuning scale','No sound browsing','Tuning root'],['Held EDIT mixer','Part 1-4','Split note A1-A6','Selected part pan'],['Held SEL editor','CUSTOM 1-4','Unused','Root'],['SAVE > PROGRAM','Saved slot 1-8','Name character position','Name character value'],['SEQ > NOTES','Family pages','No sound browsing','Zoom 8-64 steps'],['Held SEQ + step','Tempo','Lock parameter','Lock value'],['HOME-hold settings','Section','Unused','Row cursor']],Y,[119,116,115,CW-350])
Y=para('KNOBS 1-4 normally edit the four columns in left-to-right order. HOME uses the focused engine controls; the held mixer uses the four part volumes. A blank column has no parameter. On a settings section, knob 1 edits row 1, knob 2 row 2, and so on. Inside note sheets, knob 1 chooses the row and knob 2 changes its value.',Y)
Y=box('A latched function can temporarily own the controls. Tap its button or HOME to leave before trying normal sound browsing. This prevents an accidental turn from replacing a sound while you are editing another parameter.',Y)
CURRENT='Build your own four-part program'
Y=newpage(CURRENT,'Complete workflow',key='build-program')
Y=steps(['Load a starting program and save a backup if it contains wanted edits. On HOME, choose part 1 with SELECT. Use ALGORITHM for its engine and PRESET for a suitable main sound.','Turn layering OFF with a two-second GLO hold while shaping the main sound. Set its source in EDIT, articulation in ENV, movement in LFO and processing in FX.','SELECT part 2 on HOME, load a contrasting supporting sound, and repeat for parts 3 and 4. Avoid making every part equally bright or equally loud.','Turn layering ON. Hold EDIT for 3 seconds and release. Set levels with KNOBS 1-4. SELECT a part and use PRESET for pan. Keep bass/foreground near center and compare in mono.','For a split, set the boundary with ALGORITHM, then use GLO > LAYER ROUTE for Lower or Upper. Lower includes the boundary; Upper begins one note higher. Disable SPLIT for a full-range layer.','Choose scales/root, FROM octave and each part master-tuning switch. Set chord/arp modes deliberately; turn them OFF when you want plain keyboard playing.','Test soft/hard playing, low/high notes, sustain and release tails. All parts share eight voices, and some engines have lower per-part limits. Reduce unison, long tails or the number of polyphonic supports if notes disappear.','Tap SAVE. SELECT the slot, choose name positions with ALGORITHM and characters with PRESET/KNOB 2. Turn SAVE twice within its confirmation window. Reload the slot to check it, then export a backup in the editor.'],Y)
CURRENT='Load, save and back up: separate destinations'
Y=newpage(CURRENT,'Complete workflow',key='storage-workflow')
Y=table([['Item','Meaning and safe workflow'],['Sound preset','One engine sound. HOME PRESET replaces only the focused part. Import/play a user sound from the editor, then save a program if it uses custom patch data.'],['Four-part program','Four sounds plus mix/routing/performance data. SAVE > PROGRAM chooses slots 1-8. Loading replaces the working program; saving overwrites that slot only after confirmation.'],['Factory program','Read-only ROM recipe. An unused saved slot falls back to it; the editor can recall the ROM recipe even when a user save occupies the slot.'],['Scene','Partial changes inside the program. CAPTURE stores current supported values, RECALL applies them; save the containing program afterward.'],['Setlist','Ordered references to program slots. Reorder the references and save the list; it does not duplicate the programs.'],['Editor library entry','Browser-side record/file until sent to the device. Export the library separately so browser-storage clearing cannot remove your only copy.'],['Full backup','Device storage export. Stop transport, read/export, keep an untouched original, and restore only a compatible backup. Imported source files should also be kept.'],['Working autosave','Resume state across power cycles. This is separate from named saves and is not a replacement for an external backup.']],Y,[111,CW-111])
Y=para('To restore a named program on the device: stop and release sounding notes, tap SAVE, choose its slot with SELECT, and turn LOAD twice. To store a new version without losing the old one, choose a different slot before confirming SAVE. Name changes do not write the slot until SAVE completes.',Y)

CURRENT='Complete screen reference'
Y=newpage(CURRENT,'Reference map','Every reachable device page, including engine and insert-effect variants.',key='complete-reference')
Y=para('The following pages are generated from this build\'s native menu descriptors and captured using its production renderer. Parameter limits and choices come from those descriptors. Defaults mean the generic control default, not the value in every factory preset or saved program.',Y)
Y=para('Open a family with its printed button. Repeated taps advance through the available pages. SELECT normally moves backward/forward within a family, stopping at the ends; use repeated button taps for later pages in a long family. Tap HOME to return to the four-part display. A blank or "-" control is unused.',Y)
Y=para('Engine and effect pages may disappear or relabel controls when the engine/model changes. Choose the required engine on HOME before entering EDIT. Ordinary page turns do not load another sound. ALGORITHM/PRESET browse sounds only on HOME; their specialized SAVE, scale-browser, mixer and sequencer functions are described separately.',Y)
Y=para('Storage: sound controls belong to their program part; mix/routing/scenes belong to the four-part program. Explicitly save a named program to retain a deliberate version. Global USB/MIDI/display settings use device settings and may be saved when playback stops. A scene is a partial recall and does not replace all sound settings.',Y)
Y=table([['Button / family','Pages shown in this build']]+[[fam,', '.join(dict.fromkeys(p['title'] for p in INVENTORY if p['family']==fam))] for fam in FAM_ORDER],Y,[90,CW-90])
REFERENCE_PAGES=[]
for fam in FAM_ORDER:
 items=[p for p in INVENTORY if p['family']==fam]
 for n,p in enumerate(items):
  CURRENT=p['title']+(' / '+p['engine'] if p['engine']!='All applicable engines' else '')
  if p['variant']>=0:CURRENT+=' / variant '+str(p['variant']+1)
  Y=newpage(CURRENT,'Complete screen reference',PURPOSE[p['title']],key=f"screen-{p['image']}",level=1)
  REFERENCE_PAGES.append({'image':p['image'],'page':PAGE,'page_id':p['page_id']})
  Y=shots([p['image']],Y,['Native renderer example; current values may differ.'],size=158)
  Y=flow('<font color="'+RED+'">Open:</font> '+html.escape(path_for(p)),Y)
  if p['variant']>=0:
   variant='Set INSERT TYPE to '+p['controls'][0]['choices'][p['variant']] if p['title']=='INSERT' else 'This example uses a model-dependent control layout. Choose the model/waveform shown in the screenshot to obtain these labels.'
   Y=flow(variant+'.',Y)
  for k,(label,c) in enumerate(controls_for(p),1):
   title=f'KNOB {k} - {html.escape(label)}'
   if c is None:text='No assigned parameter on this page.'
   elif isinstance(c,str):text=c
   else:
    desc=OVERRIDES.get((p['engine'],label),HELP[label])
    if p['title']=='GLOBAL' and label=='TUNE':desc='Global fine-tuning offset in cents (-50 to +50). This moves all notes in addition to the A4 reference and scale tuning.'
    if p['title']=='GLOBAL' and label=='CLICK':desc='Metronome: OFF, REC only, or ON during playback.'
    if p['title']=='CHORUS' and label=='CRT':desc='Rate of the shared chorus modulation.'
    if p['title']=='CHARACTER' and label=='ALLOC':desc='ROT rotates available voice assignments; REUSE prefers reusing a matching/released voice. Unison is selected by VOIC on the VOICE page.'
    if label=='VOIC':desc='Chord voicing layout: CLOSE, SMOOTH, OPEN, first/second inversion or an octave-up layout.'
    if p['title']=='FILTER GAIN' and label=='COMP':desc='Optional estimated filter-output compensation. ON attenuates large resonance/drive level changes; this is not a measured loudness-normalization control.'
    text=html.escape(desc)+f"<br/><font color='{GRAY}'>Range: {html.escape(c['formatted'][0])} to {html.escape(c['formatted'][1])}. Generic default: {html.escape(c['formatted'][2])}. Stored steps: {c['min']} to {c['max']}.</font>"
   Y=flow(f'<font color="{RED}">{title}</font><br/>{text}',Y,size=10)
   if isinstance(c,dict) and c['choices']:
    # Split long enumerations into small, readable paragraphs without losing any choices.
    choices=c['choices']
    for start in range(0,len(choices),18):
     Y=flow('Choices'+(' (continued)' if start else '')+': '+html.escape(' / '.join(choices[start:start+18]))+'.',Y,size=9.3,color=GRAY)
  if p['title']=='NOTES':Y=flow('PRESET changes the visible zoom (8-64 steps). Hold OCT+ for about 0.4 seconds to open the six-row note sheet. Step Advance determines whether releasing played keys advances the cursor.',Y)
  elif p['title']=='PROGRAM':Y=flow('SELECT / KNOB 1: slot 1-8. ALGORITHM: character position 1-12. PRESET / KNOB 2: character. KNOB 3: LOAD, twice to confirm. KNOB 4: SAVE, twice to confirm. Browsing a slot never loads or overwrites it.',Y)
  elif p['title']=='SEL':Y=flow('In the SEL-tap scale browser, SELECT chooses the scale and PRESET its root. The normal page controls shown above remain the underlying scale settings; a second SEL tap reaches SEL 2.',Y)
  else:Y=flow('Use the four knobs in displayed order. Tap the family button for its next page. Return with HOME; to change the focused part, use SELECT on HOME, then reopen this family. Save the program after wanted sound/performance edits.',Y,size=9.6)

SYSTEM=[
 ('SCREEN',[('COLOR','RED / WHITE / BLUE / COOL / GREY device palette; use a high-contrast option for stage visibility.'),('ZOOM','OFF/ON: enlarge the active parameter display while editing.'),('KNOB ACCEL','OFF gives fine detents; ON makes fast turns change values faster.')]),
 ('LIGHTS',[('LIGHTS','OFF/LOW/MID/HIGH label backlight brightness.'),('KEYS','OFF/C KEYS/WHITE KEYS/ALL KEYS landmarks. Requires LIGHTS above OFF.'),('NOTES','OFF/ON: illuminate sounding notes on the keyboard.')]),
 ('AUDIO',[('SPEAKER LOWCUT','OFF/ON high-pass protection/tonal adjustment for the internal speaker path.'),('USB AUDIO','MASTER follows the master output; FULL supplies the full-level USB recording path.'),('USB SERIAL','OFF/ON USB serial console; a changed setting takes effect after restart.'),('USB RATE','AUTO / 44.1 KHZ / 48 KHZ USB audio rate; restart after changing.')]),
 ('SEQ',[('STEP ADVANCE','ON moves the NOTES cursor after key release; OFF keeps it in place for manual editing.')]),
 ('MIDI',[('CLOCK OUT (USB)','OFF/ON MIDI clock transmission over USB while internal synchronization is selected.'),('MPE (CH 2-16)','OFF / 48 ST / 2 ST / 12 ST / 24 ST / 96 ST. Match the member-channel bend range to the controller; CH 2-16 provide independent expression.')]),
 ('SYS',[('2ND CORE','OFF/ON the second DSP core. NOT RUNNING means the current execution environment has no active second core.'),('HARDWARE CALIBRATION','OCT+ starts teaching the physical buttons and encoder directions. Only needed for an incorrect panel map.'),('ABOUT','OCT+ opens version, build/core and attribution information; OCT- returns.')])]
for j,(name,rows) in enumerate(SYSTEM):
 CURRENT='System - '+name;Y=newpage(CURRENT,'HOME hold settings',key='system-ref-'+str(j))
 Y=shots(['system-'+str(j)],Y,['Hold HOME for 3 seconds, then release.'],size=178)
 Y=steps(['Turn SELECT to choose the '+name+' section. PRESET cycles the row cursor.', 'Turn KNOBS 1-'+str(len(rows))+' to change the corresponding rows. OCT+ steps/opens the highlighted setting; OCT- closes the menu.', 'Settings are saved when the menu closes if stopped; a save may wait until playback stops. Restart when the screen requests it.'],Y)
 for label,desc in rows:Y=flow('<font color="'+RED+'">'+label+'</font> - '+desc,Y)
CURRENT='ABOUT';Y=newpage(CURRENT,'HOME hold settings');Y=shots(['system-about'],Y,['SYS > ABOUT > OCT+.'],size=190);Y=para('Use this screen when reporting a problem. Record the firmware version and build/core information. OCT- returns to SYS; OCT- again closes settings. ABOUT is read-only.',Y)

SHEETS=[('SLIDE','KNOB 2 sets OFF/ON, or tap OCT+ to toggle. A slide changes the transition into the note.'),('NUDGE','KNOB 2 shifts the selected step earlier/later within the allowed microtiming range; zero is on the grid.'),('FILL','KNOB 2 chooses normal, fill only or no fill. This determines when the note plays.'),('MOVE','KNOB 2 moves the note by steps, stopping when there is no room.'),('COPY','Tap OCT+ to copy immediately after the note. The sheet then selects MOVE so KNOB 2 can reposition the copy.'),('DELETE','Tap OCT+ to delete the selected note. The sheet closes after deletion.')]
for j,(name,desc) in enumerate(SHEETS):
 CURRENT='Note sheet - '+name;Y=newpage(CURRENT,'Sequencer submenu');Y=shots(['note-sheet-'+str(j+1)],Y,['SEQ > NOTES > hold OCT+ about 0.4 seconds.'],size=190)
 Y=steps(['Use KNOB 1 on NOTES to choose the note to edit, then hold OCT+ to open its sheet.','Use KNOB 1 to highlight '+name+'. '+desc,'Use OCT- to close the sheet. Return to HOME when finished, and save the program to retain the edited pattern.'],Y)
 Y=box('These OCT+ and REC actions have their own timing; they are not the two-second preview / three-second latch used by the main function buttons.',Y)

HOLDS=[
 ('EDIT mixer','layer-mixer',['Hold EDIT for 2 seconds for the mixer preview; keep holding to 3 seconds, then release to latch it.','KNOBS 1-4 change the four part levels. SELECT chooses the focused part. PRESET sets its pan. ALGORITHM sets the split boundary from A1 to A6.','Use GLO > LAYER ROUTE to choose lower/upper/full range. Tap EDIT or HOME to leave. Save the program after changes.']),
 ('GLO layer toggle','layered-toggle',['Hold GLO for 2 seconds. The screen explicitly shows Layered ON or Layered OFF.','The toggle fires once per hold. Release to return; GLO never latches. A short tap opens GLO pages.']),
 ('SEL custom tuning','custom-scale',['Hold SEL for 3 seconds, then release to latch the custom-scale editor.','Use SELECT to choose CUSTOM 1-4 and PRESET to set its root. Press a note class repeatedly: -25, -50, -75, then 0 cents. It affects that class wherever master tuning applies.','Press SAVE to store the scale (the screen may report SCALE SAVE PENDING until playback stops), then select its root and octave boundary under TUNING. Tap SEL or HOME to leave.']),
 ('ENV chords','chords',['Hold ENV for 3 seconds, then release to open CHORDS. A tap opens amplitude-envelope pages.','KNOB 1 chooses a compatible voicing, KNOB 2 chord voicing, KNOB 3 strum. Play notes across the selected scale.','For maqam/microtonal tuning, incompatible choices are hidden. Turn chords OFF for ordinary single-note playing.']),
 ('SEQ step layer','hold-seq',['Hold SEQ for 2 seconds to preview the step keyboard, or 3 seconds to latch.','The sixteen white keys address steps in the current page; the black page keys select the step group. Hold a step and use KNOBS 1-4 for note, level, ratchet and nudge.','With a step held, ALGORITHM chooses a lock parameter and PRESET changes its locked value. OCT- clears its nudge/locks/fill condition; OCT+ cycles its fill condition. Tap SEQ or HOME to exit.']),
 ('ARP repeat layer','hold-arp',['Hold ARP for 2 seconds for the repeat layer, or 3 seconds to latch.','Hold a playing key to repeat it; KNOB 1 selects the repeat rate. Release the key to stop its repeat.','Tap ARP or HOME to leave the layer. The normal ARP pages configure patterned note generation separately.']),
 ('REC setup','record-armed',['Tap REC to arm recording or begin recording in the current transport context.','KNOB 1 selects FREE/TEMPO when the project is empty. KNOB 2 chooses 1, 2 or 4 bars for tempo recording. KNOB 3 chooses note-start/count-in. KNOB 4 switches record timing quantization.','Play to start when note-start is selected, or follow the count-in. Tap REC to finish recording. Use PLAY for transport and NOTES to edit the result.']),
 ('REC clear confirmation','hold-rec-clear',['Hold REC. After about 0.7 seconds the initial recording press is undone and the clear ring begins.','Release before the ring completes to cancel. Continue for another 1.3 seconds to clear the selected track/pattern.','This is a destructive clear, not a three-second menu latch. Save a copy or export a backup before clearing wanted material.'])]
for title,image,items in HOLDS:
 standard(title,'Secondary functions','Short taps and timed holds have different outcomes.',[image],items,key='hold-'+image)


DIALOGS=[('Saved slots 5-8','program-slots-5-8','On SAVE > PROGRAM, turn SELECT past slot 4. The list follows the selected group of four. Slot selection only browses; LOAD or SAVE is a separate confirmed action.'),('Load confirmation','confirm-load','Turn KNOB 3 once. AGAIN: LOAD asks for a second turn within about 1.5 seconds. If you wait or change the slot, the pending action cancels.'),('Save confirmation','confirm-save','Turn KNOB 4 once. Verify the slot and name, then turn it again within about 1.5 seconds to overwrite that slot. Wait without confirming to cancel.'),('Clear pattern confirmation','confirm-clear-seq','SAVE > TOOLS > CLRSQ: a first turn arms; a second confirms. This clears the working sequence, so save or back up first.'),('Initialize sound confirmation','confirm-init-sound','SAVE > TOOLS > INIT: confirm with a second turn only if you want to reset the focused working sound.'),('New working program confirmation','confirm-new-program','SAVE > TOOLS > NEW: confirm with a second turn to initialize the working project/program. Saved versions and an external backup are the recovery path.'),('Pedal detected','pedal-idle','GLO > PEDAL LEARN separates USB device presence from MIDI-interface presence. PEDAL DEV YES / MIDI NO means enumeration alone has not supplied a usable MIDI interface.'),('Learn armed','pedal-armed','Choose ACTION with KNOB 1, then arm with KNOB 2. Press one pedal switch. KNOB 2 cancels without committing an assignment.'),('Accept captured MIDI','pedal-captured','Check the captured CC/PC type, channel, number and value. KNOB 3 accepts; KNOB 2 cancels. An already-assigned conflicting message is rejected.'),('Test pedal action','pedal-test','Press the assigned switch again. The display reports the last message, matched action and trigger result. DUP SUPPRESSED means repeat suppression discarded it; NO MATCH means there was no matching assignment.'),('Recording count-in','record-count-in','Choose TEMPO and COUNT on the REC setup. Start the take and wait for the count-in. The selected bar length and clock source determine the recorded loop.'),('Free take recording','record-free-take','In an empty project, choose FREE and play without a fixed tempo. REC on the next downbeat closes the take and fits a loop; PLAY discards the take. Review the result before saving.')]
for title,image,desc in DIALOGS:
 CURRENT=title;Y=newpage(title,'Dialogs and status screens');Y=shots([image],Y,['Production-rendered example state.'],size=190);Y=para(desc,Y)
 Y=box('Read the on-screen state before acting. A status/confirmation screen is contextual; it does not create an extra permanent menu page.',Y)

# Calibration is captured from the actual blocking production setup routine, with simulated inputs.
CURRENT='Hardware calibration';Y=newpage(CURRENT,'Panel setup',key='calibration-reference');Y=steps(['Open HOME hold > SYS > HARDWARE CALIBRATION > OCT+, or hold both octave buttons during power-up. Release the buttons when asked.','Press the exact printed button requested at each prompt. All fourteen buttons are learned once.','Turn each requested encoder clockwise. The firmware learns both its physical position and its direction.','Finish all prompts. If you leave the setup idle, it cancels and restores the old map. Do not use calibration to repair unrelated USB or audio problems.'],Y)
BUTTONS=['FX','SEL','ENV','LFO','EDIT','GLO','HOME','SAVE','ARP','SEQ','PLAY','REC','OCT-','OCT+']
ENCODERS=['SELECT','ALGORITHM','PRESET','KNOB 1','KNOB 2','KNOB 3','KNOB 4']
for offset,names,kind in [(0,BUTTONS,'Press'),(14,ENCODERS,'Turn clockwise')]:
 for first in range(0,len(names),4):
  CURRENT='Calibration prompts';Y=newpage(CURRENT,'Panel setup',f'{kind} only the physical control named on the screen.')
  subset=names[first:first+4]
  for row in range(0,len(subset),2):
   indexes=list(range(offset+first+row,min(offset+first+row+2,offset+first+len(subset))))
   Y=shots([f'calibration-{i:02}' for i in indexes],Y,[kind+' '+names[i-offset] for i in indexes],size=180)
  Y=para('Each prompt advances after the requested input. Incorrect mappings change which physical control performs an action; repeat calibration if the panel behaves inconsistently.',Y,size=10)

CURRENT='Reference coverage and page finder';Y=newpage(CURRENT,'Reference index',key='reference-index')
Y=flow(f"This edition documents {len(set(p['page_id'] for p in INVENTORY))} menu page definitions and {len(INVENTORY)} captured layouts. Additional screens cover six system sections, ABOUT, all six note-sheet rows, secondary functions, recording and every hardware-calibration prompt.",Y)
for fam in FAM_ORDER:
 Y=flow(f'<font color="{RED}">{fam}</font>',Y,size=13)
 for title in dict.fromkeys(p['title'] for p in INVENTORY if p['family']==fam):
  matched=[(p,r) for p,r in zip([x for f in FAM_ORDER for x in INVENTORY if x['family']==f],REFERENCE_PAGES) if p['family']==fam and p['title']==title]
  pages=', '.join(str(r['page']) for p,r in matched)
  Y=flow(html.escape(title)+' - '+pages,Y,size=9.7)
(ROOT/'assets/guide/pdf-coverage.json').write_text(json.dumps({'version':V,'protocol':PROTO,'menu_pages':len(set(p['page_id'] for p in INVENTORY)),'layouts':len(INVENTORY),'reference':REFERENCE_PAGES},indent=2)+'\n')
