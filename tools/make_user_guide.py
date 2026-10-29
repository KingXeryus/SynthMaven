#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Build the versioned, illustrated FM-1 guide from current release/program data."""
from pathlib import Path
import json,shutil,html
from reportlab.pdfgen import canvas
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import Paragraph,Table,TableStyle
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.colors import HexColor
ROOT=Path(__file__).resolve().parents[1];META=json.loads((ROOT/'release.json').read_text());V=META['version'];PROTO=META['protocol']
PROGRAMS=json.loads((ROOT/'assets/programs/catalog.json').read_text());RECIPES=json.loads((ROOT/'assets/programs/recipes.json').read_text())['programs']
OUT=ROOT/'output/pdf'/f'SynthMaven-{V}-User-Guide.pdf';OUT.parent.mkdir(parents=True,exist_ok=True)
INK='#111820';RED='#a61f2b';BLUE='#254f7d';GRAY='#4d5b6b';LINE='#d9dfe6';PALE='#f1f4f8'
pdfmetrics.registerFont(TTFont('Guide',str(ROOT/'assets/fonts/Rubik.ttf')))
pdfmetrics.registerFontFamily('Guide',normal='Guide',bold='Guide',italic='Guide',boldItalic='Guide')
C=canvas.Canvas(str(OUT),pagesize=(595.276,841.89),pageCompression=1,invariant=1)
C.setTitle(f'SynthMaven {V} - FM-1 Illustrated User Guide');C.setAuthor('SynthMaven project');C.setSubject('Step-by-step operation, eight four-part programs, shortcuts and secondary functions')
W,H=595.276,841.89;M=44;CW=W-2*M;PAGE=0;INDEX=[]
def clean(s):
 return str(s).replace('\u2011','-').replace('\u2013','-').replace('\u2014','-').replace('\u2212','-').replace('\u2192',' > ').replace('\u2019',"'").replace('\u201c','"').replace('\u201d','"')
def para(text,y,x=M,w=CW,size=11.1,color=INK,gap=9):
 st=ParagraphStyle('p',fontName='Guide',fontSize=size,leading=size*1.42,textColor=HexColor(color),spaceAfter=0)
 p=Paragraph(clean(text),st);_,h=p.wrap(w,1000);assert y-h>53,(PAGE,text[:70],y,h);p.drawOn(C,x,y-h);return y-h-gap

def newpage(title,section,subtitle='',key=None,level=0):
 global PAGE
 if PAGE:C.showPage()
 PAGE+=1;C.setFillColor(HexColor(RED));C.rect(0,H-8,W,8,fill=1,stroke=0)
 C.setFont('Guide',9);C.setFillColor(HexColor(GRAY));C.drawString(M,H-35,f'SYNTHMAVEN {V}  /  FM-1');C.drawRightString(W-M,H-35,section.upper())
 C.setStrokeColor(HexColor(LINE));C.line(M,43,W-M,43)
 C.setFont('Guide',8);C.setFillColor(HexColor(GRAY));C.drawString(M,28,f'Firmware {V}  |  Protocol {PROTO}  |  8 shared synthesis voices');C.drawRightString(W-M,28,str(PAGE))
 if key:C.bookmarkPage(key);C.addOutlineEntry(clean(title),key,level=level,closed=False);INDEX.append((title,PAGE,key))
 y=H-62;y=para(title,y,size=24,gap=9)
 if subtitle:y=para(subtitle,y,size=10.6,color=GRAY,gap=15)
 return y

def h(text,y):return para(text,y,size=13,color=RED,gap=6)
def steps(items,y):
 for i,text in enumerate(items,1):y=para(f'<font color="{RED}">{i:02d}</font>  '+text,y)
 return y

def table(rows,y,widths=None):
 widths=widths or [CW/len(rows[0])]*len(rows[0]);cells=[]
 for ri,row in enumerate(rows):
  st=ParagraphStyle('cell',fontName='Guide',fontSize=9.9,leading=13.4,textColor=HexColor(RED if not ri else INK))
  cells.append([Paragraph(clean(html.escape(str(c))),st) for c in row])
 t=Table(cells,colWidths=widths,hAlign='LEFT');t.setStyle(TableStyle([('VALIGN',(0,0),(-1,-1),'TOP'),('BACKGROUND',(0,0),(-1,0),HexColor(PALE)),('TOPPADDING',(0,0),(-1,-1),6),('BOTTOMPADDING',(0,0),(-1,-1),6),('LEFTPADDING',(0,0),(-1,-1),7),('RIGHTPADDING',(0,0),(-1,-1),7),('LINEBELOW',(0,0),(-1,-1),.4,HexColor(LINE))]));_,hh=t.wrap(sum(widths),800);assert y-hh>53,(PAGE,'table overflow',y,hh);t.drawOn(C,M,y-hh);return y-hh-13

def shots(names,y,captions=None,size=174):
 captions=captions or [n.replace('-',' ') for n in names];gap=24;total=len(names)*size+(len(names)-1)*gap;x=M+(CW-total)/2
 for i,name in enumerate(names):
  path=ROOT/'docs/screenshots'/V/(name+'.png');assert path.exists(),path
  C.drawImage(str(path),x,y-size,width=size,height=size,mask='auto')
  para(captions[i],y-size-7,x=x,w=size,size=8.3,color=GRAY,gap=0);x+=size+gap
 return y-size-45

def box(text,y):
 st=ParagraphStyle('note',fontName='Guide',fontSize=10.4,leading=14.6,textColor=HexColor(BLUE));p=Paragraph(clean(text),st);_,hh=p.wrap(CW-24,800);assert y-hh-23>53,(PAGE,'box overflow');C.setFillColor(HexColor(PALE));C.roundRect(M,y-hh-18,CW,hh+18,5,fill=1,stroke=0);p.drawOn(C,M+12,y-hh-9);return y-hh-31

def standard(title,section,subtitle,images,items,extra=None,key=None):
 y=newpage(title,section,subtitle,key=key)
 if images:y=shots(images,y)
 y=steps(items,y)
 if extra:y=box(extra,y)
 return y

# 1 - cover
Y=newpage('Illustrated user guide','Edition '+V,'Step-by-step operation, shortcuts, secondary functions and the eight factory programs.',key='cover')
C.drawImage(str(ROOT/'assets/logo/synthmaven-logo.png'),M+145,Y-205,width=205,height=205,mask='auto');Y-=224
Y=shots(['home'],Y,['HOME: four loaded sounds, one selected part.'],size=176)
Y=para('Start with P1 Piano, P2 Epiano, P3 Lead Synth, P4 Oriental Lead, P5 Disco Synth, P6 Brass, P7 Strings or P8 Bass. Every program has four enabled parts with one main instrument.',Y)
Y=para(f'<link href="https://kingxeryus.github.io/SynthMaven/webapp/installer/" color="{BLUE}">Installer</link>  |  <link href="https://kingxeryus.github.io/SynthMaven/webapp/editor/" color="{BLUE}">Web editor</link>  |  <link href="https://github.com/KingXeryus/SynthMaven" color="{BLUE}">Source and support</link>',Y,size=10.3)
Y=para('Screens are generated by the production 240 x 240 firmware renderer with simulated panel/USB input. They are not photographs or proof of physical-device validation.',Y,size=8.6,color=GRAY)

# 2 - contents, filled with fixed chapter page references below.
Y=newpage('Find your next step','Contents','Use the bookmarks or the links below to jump between tasks.',key='contents')
TOC=[('Install and connect',3,'install'),('HOME controls and first sound',5,'home'),('The eight factory programs',6,'bank'),('P1-P8: layer roles and playing tips',7,'p1'),('Layering, mixer, pan and splits',15,'mixer'),('Save, name and load a program',17,'save'),('Edit, filters, envelopes, LFO and effects',18,'edit'),('Scales, roots and custom microtuning',21,'tuning'),('Chords, voicings and strum',23,'chords'),('USB pedals and MIDI controllers',24,'pedal'),('Scenes and setlists',26,'scenes'),('Arpeggiator and sequencer',27,'arp'),('Editor, imports and backups',28,'editor'),('Shortcut reference',29,'shortcuts'),('Troubleshooting and voice budget',30,'troubleshooting'),('Research, validation and credits',31,'credits')]
for text,p,key in TOC:Y=para(f'<link href="#{key}" color="{BLUE}">{text}</link> <font color="{GRAY}"> / {p}</font>',Y,size=11.2,gap=12)
Y=para('<link href="#complete-reference" color="'+BLUE+'">Complete screen-by-screen controls reference</link> | <link href="#reference-index" color="'+BLUE+'">Detailed page finder</link> | <link href="#editor-ref-0" color="'+BLUE+'">Editor reference</link>',Y,size=10)
Y=box('Three different things: a sound preset is one sound. A program stores four sounds plus their performance setup. An editor library entry is a browser-side file or record until you send and save it.',Y)

# 3
standard('Install or recover safely','Getting started','Make a backup before changing firmware. Keep power connected throughout installation.',['usb-role'],[
 'Connect the FM-1 to the computer with a USB data cable. If necessary, select GLO > USB ROLE > COMPUTER first.',
 'Open the SynthMaven installer in Chrome or Edge. Close other applications that are using the FM-1 MIDI ports.',
 'Choose INSTALL and follow the device/port prompts. Wait for completion before unplugging or restarting.',
 f'Check that the installed firmware reports SYNTHMAVEN {V}. Use the matching editor and this guide.',
 'If normal firmware cannot start, hold OCT- while powering on to enter USB rescue, then follow the installer recovery prompts.'
],extra='Recovery is an installation path, not a sound-editing shortcut. Keep a separate backup of programs, settings, user sounds and imported source files.',key='install')
# 4
standard('Connect the editor or a controller','Connections','The FM-1 USB socket uses one role at a time. Controller identification and USB-role selection are different operations.',['usb-role','controller'],[
 'For the editor, installer, USB audio or computer MIDI: choose COMPUTER, connect the computer, then click Connect in the editor and allow MIDI/SysEx access.',
 'For a directly attached USB MIDI keyboard or pedal: choose PEDAL/HOST and use correctly wired, powered host hardware. This firmware does not supply 5 V VBUS.',
 'The controller profile is detected automatically after host mode is active. Unknown class-compliant devices keep standard MIDI input.',
 'Return to COMPUTER before reconnecting the editor or installer. The firmware does not automatically switch USB roles.'
],extra='The controller screenshot is a simulated MiniLab 3 identification example. USB/controller behavior has software-model coverage; physical compatibility remains pending.')
# 5
Y=standard('Know the HOME controls','Playing','HOME shows the four sounds in the current program. The outline identifies the part being edited.',['home'],[
 'Turn SELECT to focus parts 1-4. Focus changes the sound you edit; it does not by itself change the layer mix.',
 'Turn ALGORITHM to change the focused part\'s engine. Turn PRESET/parameter to browse presets within that engine, wrapping at the ends.',
 'Turn KNOBS 1-4 to edit the four displayed controls for that part. Their labels depend on the engine.',
 'Play the keyboard or MIDI controller. Layering ON uses the enabled parts and their routing; OFF plays the focused part.'
],extra='Changing a sound preset replaces the focused sound. It does not select a different four-part factory program. Use SAVE > PROGRAM or the editor factory-program control for that.',key='home')
# 6
Y=newpage('Choose a factory program','P1-P8','The main instrument stays in part 1. Three supporting parts add body, articulation or air.',key='bank')
Y=table([['Slot','Main instrument','Program']]+[[f'P{i+1}',p['style'],p['name']] for i,p in enumerate(PROGRAMS)],Y,[45,170,CW-215])
Y=steps(['On the device, tap SAVE and use SELECT to choose slot 1-8. Turn LOAD twice to confirm. Unused slots provide the matching factory program.','Existing saved user programs take precedence in those slots. Working-program autosave also takes precedence at startup. Updating does not replace either.','To access any factory program even when your slots are occupied, use Editor > Programs > Factory programs > Load factory program. This replaces working edits but does not overwrite saved slots.','Save explicitly if you want the new working program in a device slot. Keep a backup before overwriting a user program.'],Y)
Y=box('All eight start with layering ON, chord generation and arpeggiation OFF, equal temperament and master tuning enabled. P1 is a synthesized stage piano rather than a sampled acoustic grand.',Y)

# 7-14 - program-specific pages
ENG=['ANALOG','DIGITAL','PHASE','LOFI','SAMPLE','VOICE','TRIO','WHEEL','GRAIN','FM6','PHYS','NOISE','HYBRID','WAVE','ACID','STRINGS','MACRO','VPS','TERRAIN','FMX','WSEQ','JUNO','VECTOR']
for i,p in enumerate(PROGRAMS):
 Y=newpage(f'P{i+1} - {p["style"]}','Factory programs',p['name'],key=f'p{i+1}',level=1)
 C.drawImage(str(ROOT/'docs/screenshots'/V/f'program-p{i+1}.png'),M,Y-190,width=190,height=190,mask='auto')
 yy=Y;xx=M+210
 yy=para('FOREGROUND + THREE SUPPORTS',yy,x=xx,w=CW-210,size=10,color=RED)
 for j,(part,role) in enumerate(zip(p['parts'],p['roles'])):
  voice='Poly' if part['p'][37]==0 else 'Mono follower';octave=part['route'][4]//12
  text=f'Part {j+1}: {html.escape(role)}<br/><font color="{GRAY}">{ENG[part["engine"]]} | {voice}'+(f' | {octave:+d} octave' if octave else '')+'</font>'
  yy=para(text,yy,x=xx,w=CW-210,size=10.4,gap=10)
 Y=min(Y-212,yy-6);Y=h('How to play it',Y);Y=para(html.escape(p['play']),Y)
 Y=h('Make it your own',Y);Y=steps(['Keep SELECT on part 1 while shaping the main instrument. Use its four HOME knobs for tone and articulation.','Hold EDIT for 3 seconds, then release, to keep the mixer open. Use KNOBS 1-4 to rebalance the four parts. Keep the main instrument audible.','Try SCENE 1 MAIN, 2 INTIMATE, 3 AIR or 4 DRY from GLO > SCENES. Save a program after making edits.'],Y)
 Y=box('Scene 2 reduces ambience and bright support; scene 3 emphasizes the backing texture; scene 4 removes shared effect sends. Scenes retain their musical balance without added loudness normalization. No scene adds voices. Mono supports follow last-held-note behavior, not every chord tone.',Y)

# 15
standard('Toggle layers and open the mixer','Four-part performance','The two-second preview and three-second latch are different actions.',['layered-toggle','layer-mixer'],[
 'Hold GLO for 2 seconds. The display replaces the current page with Layered ON or Layered OFF. It toggles once per hold.',
 'Release GLO to return. GLO never latches. A normal GLO tap opens its pages without flashing the held screen.',
 'Hold EDIT for 2 seconds to preview the layer mixer. Continue to 3 seconds, then release, to keep it open.',
 'KNOBS 1-4 set the four part volumes. SELECT chooses the part whose pan and split are shown. Tap EDIT or HOME to exit.'
],extra='Four layers do not mean 32 voices. All four parts share 8. The factory voicings deliberately use mono support parts where useful to leave more room for the foreground instrument.',key='mixer')
# 16
standard('Pan and split the keyboard','Four-part performance','The supplied bank starts with full-keyboard layering. Splits remain available for your own programs.',['layer-mixer','layer-route'],[
 'In the latched EDIT mixer, turn SELECT to choose a part. Turn PRESET/parameter to set that part\'s pan.',
 'Turn ALGORITHM to set its split point from A1 to A6. The display shows note and octave. Changing it enables the part\'s current split direction.',
 'Use GLO > LAYER ROUTE or Editor > Sound > Layers to choose Full, Lower or Upper. Lower includes the displayed boundary; Upper starts on the next note.',
 'For an upper range starting at C3, use boundary B2 with UPPER enabled. Check both sides of the split, then save the program.'
],extra='A split can reduce the number of active parts per key and leave more shared voices for chords. Pan alone does not change voice count.')
# 17
standard('Save, name and reload','Programs','A slot is not overwritten until you confirm SAVE. Browsing a slot or editing its name does not save.',['save-program'],[
 'Stop playback, release keys and sustain, and allow sounding tails to finish. Tap SAVE to open PROGRAM.',
 'Turn SELECT or KNOB 1 to choose slot 1-8. ALGORITHM chooses one of the 12 name positions; PRESET or KNOB 2 changes its character.',
 'Turn SAVE twice to confirm storing the complete working program and name. Check the on-screen confirmation.',
 'To reload, select the slot and turn LOAD twice. The current four sounds, routing, mix, scenes and selected tuning are replaced.',
 'Autosave resumes your last working state after power-up. It is separate from deliberately saving a named slot. Export backups regularly.'
],extra='Factory recall in the editor loads ROM data into the working program only. Your saved program at the same numbered slot stays intact until you explicitly save over it.',key='save')
# 18
standard('Shape the sound with EDIT','Sound design','EDIT focuses on engine and voice controls. Filters appear only where the engine exposes them.',['edit-engine','edit-filter'],[
 'From HOME, SELECT the part to edit and tap EDIT. Tap EDIT again to advance its available pages.',
 'Adjust the engine controls on EDIT 1/2. For a useful filter, use the grouped FILTER page: model, cutoff, resonance and envelope amount.',
 'If a control is disabled or the page is absent, it does not affect this engine. HYBRID has its own filter architecture; the shared page addresses filter 1.',
 'FILTER GAIN groups drive, optional output compensation and trim. Use compensation when resonance/drive makes the sound jump in level, then compare at similar listening volume.'
],extra='Compensation is estimated attenuation, not a loudness meter or automatic mastering. The factory sound calibration and curated program mixes are separate from this switch.',key='edit')
# 19
standard('Envelopes, LFOs and templates','Sound design','The envelope shapes articulation; the LFO and matrix provide movement and expression.',['envelope','mod-templates'],[
 'Tap ENV for attack, decay, sustain and release. Short attack gives definition; a longer attack softens the front. Release controls the tail after key-up.',
 'Tap LFO to reach its rate, waveform and destination pages. Add a small amount first and test the full keyboard range.',
 'Use LFO > MOD PRESET for velocity-to-brightness, pressure-to-vibrato, wheel-to-vibrato or an envelope swell.',
 'Use the editor for the full matrix and graphical envelope controls. HYBRID templates need a free modulation slot; a full matrix is left unchanged.'
],extra='Long releases and sustain can keep voices occupied. Use shorter tails if new chord notes steal old ones. Holding ENV for 3 seconds opens CHORDS instead of the envelope pages.')
# 20
standard('Use effects without losing the instrument','Sound design','Start with the dry voice, then add only the space or movement it needs.',['effects','filter-gain'],[
 'SELECT the part, tap FX, then adjust the displayed distortion, chorus, delay and reverb sends.',
 'Tap FX to reach additional effect pages. The insert section offers its own type and amount controls; engine-specific chorus can be separate.',
 'Keep bass near centre and use less wet signal for it. On leads, check that delay repeats do not hide the next phrase.',
 'Compare the MAIN and DRY factory scenes. Bring effects back gradually, then save your preferred version.'
],extra='Some effect settings are global. A saved program stores each part\'s supported sends and insert settings, but changing shared effect configuration can alter how the same mix sounds later.')
# 21
standard('Select a scale, root and octave range','Tuning','Microtuning adjusts pitches; quantization chooses notes. They are separate controls.',['scale-browser','tuning-from-c3'],[
 'Tap SEL to open the scale browser. Turn SELECT to cycle scales; PRESET or KNOB 1 changes the root. The twelve cells visualize chromatic offsets.',
 'For an octave boundary, open GLO > TUNING. KNOB 1 selects system, KNOB 2 FROM cycles C0-C8 by octaves only, KNOB 3 selects root, KNOB 4 controls master tuning for the focused part.',
 'FROM C3 means MIDI 48 and higher use the selected tuning. Lower notes keep equal-tempered intervals. The A4 reference remains global; find it on TUNING 2.',
 'Use QNT ALL to apply the focused ROOT/SCL note selection across all four parts and incoming MIDI. Use Apply master tuning to let an individual part follow or bypass master microtuning.'
],extra='Bypassing master tuning does not turn off a separately selected quantizer or chord generator. Factory programs start in equal temperament; loading P4 does not secretly select a maqam.',key='tuning')
# 22
standard('Create a custom chromatic scale','Tuning','Use the physical key for the pitch class that should change. Other chromatic keys stay unaltered.',['custom-scale'],[
 'Hold SEL for 3 seconds, then release. CUSTOM SCALE EDIT remains open. SELECT chooses CUSTOM 1-4 and PRESET chooses ROOT.',
 'Press a key once for -25 cents, twice for -50, three times for -75 and four times to restore 0. Repeat for each pitch class you want to detune.',
 'For example, to lower E by 50 cents, press an E key twice. E in other octaves receives the same pitch-class adjustment, subject to FROM.',
 'Tap SAVE to store the custom scale, then SEL to exit. Save the program if you also want its complete four-part setup preserved.'
],extra='ROM factory recall preserves your four custom-scale definitions. Loading a saved user program restores the definitions stored in that program. Export a backup before replacing an important setup.')
# 23
standard('Chords, voicings and strum','Harmony','Use the three-second ENV shortcut. Normal ENV taps still open envelope editing.',['chords'],[
 'Hold ENV for 3 seconds and release. CHORDS stays open. Use KNOB 1 for CHORD, KNOB 2 for VOIC and KNOB 3 for STRUM.',
 'Play any key, including black keys. Chords follow the selected scale/root; out-of-scale trigger roots map to an available scale degree.',
 'Choose a voicing such as CLOSE, SMOOTH, OPEN or an inversion. STRUM offers signed millisecond direction and tempo-related divisions.',
 'With nonzero microtonal tuning, the selector offers OFF, OPEN 5 and NEUTRAL. Incompatible stored shapes sound as OPEN 5 until changed; they do not overwrite the saved shape.'
],extra='Neutral harmony chooses existing tuned scale notes near its target intervals; it does not add arbitrary detuned keys. Mono main voices remain monophonic, and generated chord tones consume the shared voice budget.',key='chords')
# 24
standard('Teach a USB pedal an action','Pedal control','Use Learn instead of manually typing message type, number, value and channel.',['pedal-learn'],[
 'Choose PEDAL/HOST and attach a class-compliant pedal through powered host hardware. Enable program/patch MIDI as needed.',
 'Open GLO > PEDAL LEARN. Select Patch Up, Patch Down, Program Next or the required supported action, then arm LEARN.',
 'Press the pedal. Read the captured CC/PC message and channel. ACCEPT it to store the assignment, or cancel and try again.',
 'Press again to test. Check the last-message, matched-action and trigger/suppression indication. Conflicting duplicate assignments are refused.',
 'Set REPEAT only when the pedal sends unwanted duplicates. The 0/OFF default preserves repeated taps. CC release re-arms; identical PC taps inside the window may also be suppressed.'
],extra='Return to COMPUTER for the editor or installer. Bluetooth is removed. A USB device being present does not by itself mean a MIDI interface was detected.',key='pedal')
# 25
standard('Use keyboard and surface controllers','MIDI controllers','Identification is automatic in active host mode; role switching remains manual.',['controller'],[
 'Select PEDAL/HOST, connect powered class-compliant hardware and inspect GLO > CONTROLLER for product, family and protocol.',
 'Supported Alesis, Novation, Arturia, Akai and Mackie families receive the documented profiles. Unknown controllers keep standard notes and MIDI controllers.',
 'MCU/HUI uses the first four strips for part volume, pan, mute, solo, select and record arm, plus Play/Stop/Record. Music and DAW ports are kept separate.',
 'APC Mini and supported modern Launchpads have chromatic pad mappings. Named control jacks take precedence; MiniLab 3 skips its separate DIN Thru port.',
 'Use the advanced protocol/port override only when the controller\'s layout differs. Check the compatibility guide for family and generation limits.'
],extra='Scribble strips, a full DAW, proprietary drivers and every custom controller template are not implemented. The pictured identity is simulated; physical interoperability is still subject to testing.')
# 26
standard('Move between scenes and programs','Performance','A scene edits the current program. A setlist chooses saved programs in an order.',['scenes','setlist'],[
 'Open GLO > SCENES. Factory scenes 1-4 are MAIN, INTIMATE, AIR and DRY. Select a scene and RECALL it.',
 'To make your own scene, adjust levels, mutes, effect sends, macros and tuning, then CAPTURE. Save the program to preserve the scene.',
 'Open GLO > SETLIST to choose ENTRY, PROGRAM and LENGTH, or use the editor to reorder up to 32 references and Save order.',
 'Assign Scene Next/Prev, Program Next/Prev or Setlist Next/Prev with pedal Learn. Program loads stop transport and wait for quiet; they do not provide gapless sample playback.'
],extra='Scenes do not replace engines, pan, split routing or complete modulation matrices. The setlist is global; its entries reference saved slots, so saving a different program in a slot changes what that entry loads.',key='scenes')
# 27
standard('Arpeggiate and record a pattern','Sequencing','The factory programs contain sounds and scenes, not pre-recorded songs.',['arpeggiator','sequencer'],[
 'Tap ARP and choose a mode, rate, octave range and gate. Hold notes to hear the pattern; use HOLD only when you want notes latched.',
 'Tap SEQ for the selected part\'s pattern. Set length/division as needed and use PLAY to start or stop.',
 'Use REC to arm/record the focused part, then play the keyboard or send MIDI. Press REC again to stop recording.',
 'Use the editor for detailed note, velocity and performance-lane editing. If external sync is selected, the sequencer waits for the required clock/start messages.'
],extra='REC hold is a destructive pattern-clear gesture with its own countdown: releasing before completion cancels it. It is an exception to the normal function-layer timing. Back up important patterns.',key='arp')
# 28
Y=newpage('Use the editor and back up your work','Editor','Use COMPUTER mode and the matching browser editor. A mock preview is not a physical-device connection.',key='editor')
Y=h('A reliable editing session',Y);Y=steps(['Connect, select the FM-1 and read its current state. Choose a part before editing its engine, envelope, filter or modulation panel.','Use Sound > Layers for the four levels, pan, mute/solo and routing. Use the graphical envelopes and available engine visualizations to understand edits.','Programs > Factory programs recalls any P1-P8 into the working program without replacing a saved slot. Use the saved-program name field and Save when ready.','Export a full device backup and an editor-library export separately. A browser library entry is not a saved device program. Keep imported source files as well.'],Y)
Y=h('Import limits worth knowing',Y)
Y=table([['Source','What to expect'],['DX7 / .syx','FM6 can read supported voices and banks. Operator-bank storage and a playable user preset are separate actions.'],['Vital / .vital','Imports are HYBRID approximations, not Vital playback. Unsupported effects and modulation can change the sound. Save custom patch bytes in a program and export your library.'],['Wavetables / WAV','Supported wavetable conversion can fill user slots. USER WT stays silent until user data exists.'],['Scala / .scl','Imports tuning data, not new synthesis sounds. Select the root, FROM boundary and master-tuning behavior separately.']],Y,[105,CW-105])
Y=box('Stop transport and release notes before storage/restore operations when requested. Loading a program replaces working edits. Restore only compatible backups and keep an untouched original copy.',Y)
# 29
Y=newpage('Shortcuts and secondary functions','Quick reference','Short taps and holds have different jobs. A brief tap does not show a held-layer screen.',key='shortcuts')
Y=table([['Gesture / context','Result'],['ALGORITHM on HOME','Engine for the focused part'],['PRESET on HOME','Preset within that engine; wrap at ends'],['SELECT on HOME','Focus part 1-4'],['SEL tap','Scale browser; SELECT cycles tuning scales'],['SEL hold 3 seconds','Latched custom-scale editor; repeated key presses cycle -25/-50/-75/0 cents'],['ENV tap / hold 3 seconds','Envelope pages / CHORDS menu'],['EDIT tap / hold 2 / hold 3 seconds','Edit pages / mixer preview / latched mixer'],['Mixer KNOBS 1-4','Four part volumes'],['Mixer SELECT / PRESET / ALGORITHM','Part focus / pan / split point A1-A6'],['GLO tap / hold 2 seconds','Global pages / toggle layering once; never latch'],['SAVE tap','Program load/save and naming'],['SAVE page SELECT / ALGORITHM / PRESET','Slot / name position / name character'],['HOME tap away from HOME','Return to the four-part HOME display; full-screen visualizer is disabled'],['HOME hold 3 seconds','System menu'],['Other supported function layers','Preview after 2 seconds; latch after 3; tap the same function button or HOME to leave'],['REC tap / hold','Record action / separate clear countdown; release early to cancel']],Y,[164,CW-164])
# 30
Y=newpage('Troubleshoot the sound','Practical limits','Check the performance setup before assuming an engine or preset is broken.',key='troubleshooting')
Y=table([['Symptom','Try this'],['Only one part sounds','Check layering, enable/mute/solo, focus, split zones and input routing.'],['New chord notes cut old notes','Reduce chord density, unison and release tails. Eight voices are shared across all parts.'],['Piano is not an acoustic grand','P1 is a synthesized stage-piano design. It is not a multisampled acoustic instrument.'],['Mono support does not play every chord tone','This is intentional. The polyphonic main carries the chord; mono followers supply quieter colour.'],['Unexpected detuning','Check tuning system, root, FROM, master-tuning bypass, A4, pitch bend and oscillator detune.'],['Pedal/controller detected but silent','Check that a MIDI interface was detected, KEY IN/channel settings and the controller\'s own mode.'],['Editor cannot connect','Use COMPUTER, a data cable, Chrome/Edge and MIDI/SysEx permission; close competing MIDI applications.'],['Notes keep sounding','Release sustain, check HOLD/release/loops and send an all-notes/all-sound-off safety command if needed.'],['Old sound returns after upgrade','Existing autosave is preserved. Recall a factory program explicitly to try the new bank.']],Y,[156,CW-156])
Y=box('Voice examples: four fully polyphonic layers usually allow only two simultaneous keys. One polyphonic main plus three mono followers leaves more notes for the main voice. Two polyphonic parts plus two mono supports can fill eight voices with three-note chords, before release tails.',Y)
Y=para('Physical USB/controller validation remains pending. Host tests and rendered audio do not establish noise, latency, speaker response or stability on every FM-1 and accessory combination.',Y,size=9.4,color=GRAY)
# 31
Y=newpage('Research, validation and credits','Reference','The bank is designed for recognizable foreground instruments, complementary supports and reviewable audio.',key='credits')
Y=para('Firecrawl research informed layer roles, envelope relationships, expressive controls, masking, mono compatibility and performance organization. These are design principles, not a guarantee of individual taste. The project includes recipes, source references and audio previews for review.',Y)
Y=h('Sources used',Y)
SOURCES=[('Roland: The Art of Layering','https://articles.roland.com/sound-design-the-art-of-layering-with-the-roland-jd-800/'),('Sound On Sound: Envelopes, Gates & Triggers','https://www.soundonsound.com/techniques/envelopes-gates-triggers'),('Sound On Sound: Synth Secrets conclusion','https://www.soundonsound.com/techniques/secret-big-red-button'),('iZotope: Frequency Masking','https://www.izotope.com/community/blog/what-is-frequency-masking'),('iZotope: Mono vs. Stereo','https://www.izotope.com/community/blog/mono-vs-stereo'),('Ableton: Instrument, Drum and Effect Racks','https://www.ableton.com/en/manual/instrument-drum-and-effect-racks/'),('Sequential: Prophet Rev2','https://sequential.com/hybrid-analog/prophet-rev2/'),('Spectrasonics: Live and Stack Modes','https://support.spectrasonics.net/manual/Omnisphere3/30/en/topic/using-live-mode-and-stack-mode')]
for title,url in SOURCES:Y=para(f'<link href="{url}" color="{BLUE}">{html.escape(title)}</link>',Y,size=10,gap=8)
sound_audit=json.loads((ROOT/'assets/factory-level-audit.json').read_text()); audible=[r for r in sound_audit if r['lufs'] is not None]
program_audit=json.loads((ROOT/'assets/programs/audit.json').read_text())['renders']; normalized=[r for r in program_audit if r['mode'] in ('mix','solo','layer-off')]
validation='Version 2.2.4 preserves the eight programs captured from the owner’s FM-1 byte for byte. P1 STAGE PIANO now contains the configured HYBRID main sound; P2–P8 are unchanged. Embedded patches, parameters, mixer settings, routes, tuning and scenes are retained. Solo calibration and 2.2.3 audio/loader fixes are unchanged. Host checks cover record integrity, lossless packing, clipping, true peaks, solo and layered playback, scenes and mono compatibility. No additional hardware timing tests or firmware installs were performed for this bank update. Earlier hardware tests showed 64–75% average CPU peaks; dense eight-note bursts missed a deadline in P5/P7 before the final 2.2.3 batching change. That timing change has host validation only; heavy layers can still overload the processor. All four parts share eight synthesis voices. Every firmware install resets user data, including same-version reinstalls: back up first.'
Y=h('What was checked',Y);Y=para(validation,Y,size=10.3)
Y=h('Code and image provenance',Y);Y=para('SynthMaven is based on Clipper by 3dSam. Upstream components, artwork and licences are recorded in the repository NOTICE and source files; the engine implementations are not presented as wholly original SynthMaven code. Screens in this guide use the current firmware renderer with controlled simulated input. The supplied SynthMaven logo is used as provided.',Y,size=10.3)
Y=para(f'<link href="https://github.com/KingXeryus/SynthMaven/blob/main/docs/FACTORY_PROGRAM_DESIGN.md" color="{BLUE}">Full design record</link> | <link href="https://kingxeryus.github.io/SynthMaven/docs/midi-controllers.html" color="{BLUE}">Controller coverage</link>',Y,size=9.8)
exec(compile((ROOT/'tools/user_guide_reference.py').read_text(),str(ROOT/'tools/user_guide_reference.py'),'exec'))
exec(compile((ROOT/'tools/user_guide_editor_reference.py').read_text(),str(ROOT/'tools/user_guide_editor_reference.py'),'exec'))
C.save();dest=ROOT/'docs'/OUT.name;shutil.copyfile(OUT,dest)
print(f'{OUT}: {PAGE} pages; copied to {dest}')
