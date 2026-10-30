"""Release-specific page specifications; firmware labels/ranges come from host capture.
Control descriptions adapted from the earlier SynthMaven illustrated guide (GPL-3.0-only).
"""
import json,re
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
HELP=json.loads((ROOT/'assets/guide/control-help.json').read_text())
FAMILY={1:'ENV',2:'LFO',3:'SEL',4:'EDIT',5:'GLO',6:'SAVE',7:'ARP',8:'SEQ',9:'EDIT hold',10:'FX'}
PREF_RANGES={0:'NIGHT, DAY, CONTRAST, SUNSET, OCEAN, NEON, CANDY, LO-FI, GRAY',1:'RUBIK / PIXEL',2:'OFF / LOW / MID / HIGH / FULL',3:'NEVER / 5 / 15 / 30 / 60 MIN',4:'IN+OUT / IN / OUT / OFF',5:'STEREO / 4 TRACKS',6:'COMPUTER / PEDAL',7:'OFF / ON',8:'OFF / ON',9:'OFF / ON',10:'300 / 400 / 500 / 600 ms (FX hold only)',11:'OFF / REC / ON',12:'LOW / MID / HIGH',13:'OFF / 1 BAR / 2 BARS',14:'OFF / ON',15:'HOLD / ADD',16:'Turn right to open',17:'Turn right to open',255:'Unused'}
SPECIAL={
'PRESETS':[('LIST','Choose ALL, favorites, recent sounds or a category.'),('SOUND','Browse and load the highlighted sound into the focused part.'),('FAV','Turn right to mark a favorite; left to remove it.'),('ENGINE','Choose the engine whose presets are listed.')],
'USER':[('SLOT','Choose one of 384 user sound slots.'),('LOAD','Turn right to select Load; confirm using OCT+.'),('ERASE','Turn right to select Erase; confirm only after backing up this sound.'),('SAVE','Turn right to select Save. Stop playback and confirm using OCT+.')],
'NOTES':[('NOTE','Move through steps and notes within a chord.'),('PITCH','Change the selected note pitch. An empty step can acquire a note.'),('LENGTH','Change note length; ties can extend across steps.'),('FORCE','Choose note articulation/velocity strength.')],
'DRUM SOUND':[('SOUND','Choose the drum voice in the current kit.'),('TUNE','Transpose that kit voice -24 to +24 semitones.'),('DECAY','Decay offset -64 to +63; changes the tail of this kit voice.'),('TONE','Tone offset -64 to +63; changes brightness of this kit voice.')],
'DRUM MIX':[('SOUND','Choose the kit voice to balance.'),('LEVEL','Raise/lower this kit voice. This edits the kit, not just one hit.'),('Unused','No assigned parameter.'),('Unused','No assigned parameter.')],
'DRUM HIT':[('HIT','Move through individual recorded hits and steps.'),('SOUND','Select the drum instrument for this event.'),('TUNE','Tune only the selected hit by -24 to +24 semitones.'),('LENGTH','Set the duration/gate of this hit; it does not change the whole kit.')],
'PATTERNS':[(f'TRACK {i}',f'Choose the next pattern for track {i}; changes may wait for a loop boundary during playback.') for i in range(1,5)],
'SONG':[('SECTION','Select an arrangement section.'),('TRACK','Choose the track being arranged.'),('PATTERN','Select the pattern assigned to the focused track in this section.'),('REPEATS','Set section repeats from 1 to 16.')],
'MOTION':[('ENABLE','Enable or disable motion automation.'),('LANE','Select an existing automation lane.'),('STEP','Select the step within the lane.'),('VALUE','Edit its value for the selected step.')],
}
GLO={'GLOBAL':'TUNE / CLOCK','SYSTEM':'SYSTEM','WEIGHTS':'VOICE COST','WEIGHTS 2':'CPU GUARD'}

def describe(p,c):
 l=c['label'];t=p['title'];e=p['engine']
 if not l:return HELP['']
 if t=='FM BEND' and l=='TIME':return 'Portamento transition time: higher values produce a slower glide between notes. PORTA and GLISS determine its behavior.'
 if t.startswith('EDIT') and l=='TUNE':return 'Pitch tuning of the engine oscillator; use the displayed interval to transpose this sound without changing the global scale.'
 if t=='FM BEND' and l=='PORTA':return 'Select the FM portamento behavior; TIME sets the transition duration.'
 if t=='FM BEND' and l=='BRTH':return 'Breath controller modulation range; B.DST selects its destination.'
 if t=='FM BEND' and l=='STEP':return 'Pitch-bend step size; stepped bends move in discrete pitch intervals.'
 if re.fullmatch('[RL][1-8]',l) and (e=='CZ-1' or t in ['EG RATE','EG LVL','PITCH EG','PITCH LV']):
  target='pitch' if 'PIT' in t else 'waveform brightness' if 'WAV' in t else 'operator amplitude' if e=='FM6' else 'amplitude'
  return ('Speed toward' if l[0]=='R' else 'Target value at')+f' {target} envelope stage {l[1]}. '+('Higher rates move faster; this is not a time in milliseconds.' if l[0]=='R' else 'Levels define the envelope shape; rate settings determine how quickly it moves between them.')
 if e=='CZ-1' and l=='SUS':return 'Choose the envelope stage held while the key remains down; "-" disables the sustain point.'
 if e=='CZ-1' and l=='MOD':return 'Choose no line modulation, ring modulation or noise modulation between the CZ synthesis lines.'
 if e=='CZ-1' and l=='OCT':return 'CZ base-octave setting.' if t=='CZ LINE' else 'Octave component of line detuning; combine with NOTE, FINE and SIGN.'
 if t=='MASTER TUNE' and l=='KEY':return 'Select the chromatic note to fine-tune.'
 if t=='CUSTOM TUNE' and l=='KEY':return 'Select C through B. Each edit applies to that pitch class within the active tuning range.'
 if t=='CONTROLS' and l=='HOLD':return 'Threshold for the inherited FX performance layer only. It does not change the fixed two-second mixer or three-second tuning/chord shortcuts.'
 if l=='DRUM':return 'MIDI drum-input channel: OFF or 1-16; channel 10 is the General MIDI convention.' if t=='SYSTEM' else 'Budget points per drum voice: AUTO uses the built-in cost.'
 if t=='ARP' and l=='MODE':return 'OFF, ascending, descending, up/down, random, played order or repeat patterns from held notes.'
 if t=='ARP' and l=='OCT':return 'Number of octaves traversed by the arpeggiator.'
 if t=='ARP' and l=='RATE':return 'Arpeggiator note division relative to tempo. T values are triplets.'
 if t=='TEMPO' and l=='QNT':return 'Recording grid for the next take on all tracks. OFF preserves timing; divisions snap recording to the grid. Does not rewrite an existing take.'
 if t=='GLOBAL' and l=='TUNE':return 'Global fine pitch offset in cents, added to A4 and scale tuning.'
 if t=='SYSTEM' and l=='MIDI':return 'Choose USB or TRS for the input status display. USB shows ON when configured, ENUM while enumerating, BUS when bus frames arrive, WAIT before host activity, or OFF. This selector does not change the USB COMPUTER/PEDAL role.'
 if t=='PROJECT' and l=='SLOT':return 'Choose project A-D or the template destination. This is separate from the eight PROGRAM slots.'
 if t=='PROJECT' and l in ['LOAD','SAVE']:return l.title()+' the selected project. Turn the action knob right, then confirm with OCT+; OCT- cancels. Stop playback before saving.'
 if t=='PROGRAM' and l in ['LOAD','SAVE']:return l.title()+' the complete four-part program in the selected slot. Turn right twice within 1.5 seconds; turn left to cancel the armed action.'
 if t=='PROGRAM' and l=='NAME':return 'Change the character at the current name position. ALGORITHM chooses the position; PRESET also changes the character.'
 if l=='SLOT':return 'Select the destination USER sound slot (1-384).' if t in ['STORE','P5 STORE'] else 'Choose saved program slot P1-P8.'
 if t=='OUT' and l=='OUT':return 'Output level of the selected FM operator. Carrier level changes loudness; modulator level changes harmonic content.'
 if t=='FREQ' and l=='MODE':return 'RATIO follows the played note; FIXED uses absolute-frequency behavior.'
 if l=='SYNC':
  if t=='ALGO':return 'Oscillator key synchronization: restart the FM oscillators at note-on when enabled.'
  if t=='P5 MIXER':return 'Hard-sync oscillator A to B; tuning changes then alter the harmonic shape.'
  return 'LFO tempo synchronization. OFF uses free speed; other choices follow the selected clock division.'
 if t=='LFO DEST' and l=='FLT':return 'LFO-to-filter depth. Positive and negative values reverse the cutoff movement; zero leaves cutoff unchanged.'
 if l=='MODEL' and e=='SID' and t.startswith('EDIT'):return 'Select SID chip-model character, including its oscillator/filter response.'
 if t=='P5 POLYMOD' and l=='ENV AMT':return 'Filter-envelope contribution to Poly-Mod destinations.'
 if t=='P5 POLYMOD' and l=='OSC B':return 'Oscillator B contribution to Poly-Mod destinations; audio-rate settings create complex harmonics.'
 if t in ['P5 POLYMOD','P5 WHEEL','P5 WHEEL 2'] and l in ['FREQ A','FREQ B','PW A','PW B','FILTER']:return 'Enable this modulation source for '+l.lower()+'. OFF leaves that destination unaffected.'
 if l=='RATE' and p.get('effect'):return 'Rate/time control of '+p['effect']+'. Use the displayed units: Hz denotes motion speed; note divisions follow tempo.'
 if l in ['CHAR','BITE','FOCUS','JITTER'] and p.get('effect'):return 'Character/intensity of '+p['effect']+'. Adjust alongside MIX; the audible effect depends on the selected processor.'
 if l=='FB' and e in ['FM6','DIGITAL']:return 'FM feedback amount. Higher values feed operator output back into modulation and add brighter/noisier harmonics.'
 if l not in HELP:raise ValueError((e,t,l))
 return HELP[l]

def range_text(c):
 if 'pref' in c:return PREF_RANGES[c['pref']]
 choices=list(dict.fromkeys(x for x in c.get('choices',[]) if x))
 if choices and len(choices)<=16:return ' / '.join(choices)
 if choices:return f'{len(choices)} named choices; browse the on-screen list.'
 if c.get('min')==c.get('max'):return 'Action / no continuous value'
 if c.get('low') or c.get('high'):return f"{c['low']} to {c['high']} {c.get('unit','')}".strip()
 return f"{c.get('min',0)} to {c.get('max',0)}"

def records():
 raw=json.loads((ROOT/'assets/guide/screen-inventory.json').read_text())
 out=[]
 for p in raw:
  t=p['title'];fam=FAMILY[p['family']]
  if t=='MIXER' or (p['engine']=='MIDI Track' and t=='FX'):continue
  if p['env']:fam='ENV'
  elif p['lfo']:fam='LFO'
  title=GLO.get(t,t)
  specific=p['family']==4 or p['env'] or p['lfo'] or t.startswith('DRUM')
  title=(p['engine']+' / ' if specific else '')+title
  if p.get('effect'):title+=' / '+p['effect']
  open_=f'On HOME select the part, then press {fam}. Use SELECT or repeated family-button taps to find {t}.'
  if specific:open_=f'On HOME choose {p["engine"]}, then {fam}. Use its section/page navigation to reach {t}.'
  if t=='LAYER MIX':open_='Hold EDIT for two seconds, then release. The mixer stays open until you leave it.'
  if t=='MASTER TUNE':open_='Tap SEL to open MASTER TUNE.'
  if t=='CUSTOM TUNE':open_='Hold SEL for three seconds, then release.'
  if t=='CHORD':open_='Hold ENV for three seconds, then release.'
  if t=='PROGRAM':open_='Tap SAVE; PROGRAM is the first page. Stop playback before writing a save.'
  if p.get('effect'):open_=f'Tap FX, select the part or SHARED rack, choose {p["effect"]} for the required slot, then open {t}.'
  controls=[]
  if t in SPECIAL:controls=[(f'KNOB {i+1} - {l}',d) for i,(l,d) in enumerate(SPECIAL[t])]
  elif p.get('list'):
   controls=[('KNOB 1','Edit the highlighted row.'),('KNOB 2','Move the row highlight; the list scrolls to reveal the remaining settings.'),('KNOBS 3-4','No direct row editing on this list screen.')]
  else:
   for i,c in enumerate(p['controls']):
    if not c:controls.append((f'KNOB {i+1}','No assigned parameter on this page.'));continue
    controls.append((f'KNOB {i+1} - {c["label"] or "Unused"}',describe(p,c)+' Values: '+range_text(c)+'.'))
  if t in ['FX','SHARED']:
   controls[1]=('KNOB 2 - ROW','Choose one of the three effect rows or the shared/return row.')
   if t=='FX':controls[2]=('KNOB 3','No parameter on the ordinary part effect row.')
   controls[3]=('KNOB 4 - AMOUNT','Set the selected row mix/send; NOTE TAIL uses release amount. On RETURN, set the shared wet return level.')
  if t=='MOD':controls[1]=('KNOB 2 - ROUTE','Select modulation connection 1-4; the other three knobs edit that connection.')
  rows=[]
  for c in p.get('list',[]):
   if c:rows.append((c['label'],describe(p,c)+' '+range_text(c)))
  buttons='HOME returns to the four-part display. SELECT navigates pages. The family button advances pages. PLAY starts/stops transport; REC arms the selected track. OCT- backs out and OCT+ confirms where an action or picker is offered.'
  if specific:buttons+=' Native engine sections can be opened from the EDIT section map; PRESET navigates sections/pages rather than replacing the sound there.'
  if p.get('list'):buttons='KNOB 2 selects a row and KNOB 1 changes it. OCT+ opens a choice list when available; OCT- returns to HOME. SELECT/family button moves between screens. PLAY/REC retain transport/recording roles.'
  if t=='LAYER MIX':buttons='SELECT focuses part 1-4. PRESET pans the focused part. ALGORITHM sets its lower split point A1-A6. KNOBS 1-4 always control the four volumes. HOME exits; hold GLO for two seconds to toggle layering.'
  if t=='PROGRAM':buttons='SELECT (or KNOB 1) chooses P1-P8. ALGORITHM chooses name position; PRESET/KNOB 2 edits the character. KNOB 3 loads and KNOB 4 saves with two right turns. Repeated SAVE opens PRESETS, USER and PROJECT; HOME exits.'
  if p['scope'] in [0]: pass
  if t in ['FREQ','OUT','EG RATE','EG LVL','SCALE','CURVE']:buttons+=' PRESET selects the FM operator 1-6; edits apply only to that operator.'
  save='Save the containing four-part PROGRAM to retain the sound and mix. Save a PROJECT for pattern banks and arrangements; export a USB backup for safekeeping.'
  if fam=='GLO' or t in ['MASTER TUNE','CUSTOM TUNE']:save='These are device/global settings. Master tuning survives program changes unless a saved program explicitly includes a tuning override. Named custom scales can be saved/exported in the editor.'
  steps=['Open this screen using the path above; check the selected part or row before turning a knob.','Change one setting at a time. Play a note or pattern and compare the result; inactive routes or muted parts may make a control inaudible.','Use HOME to leave, then save the appropriate program/project if you want a named version.']
  if fam=='GLO':steps=['Open GLO and select this named page.','Turn the knob beside the setting to choose the desired value.','Leave the menu; settings are remembered. Use COMPUTER USB role before reconnecting the editor.']
  out.append(dict(title=title,section=fam,image='assets/guide/screens/'+p['image']+'.png',open=open_,controls=controls,rows=rows,buttons=buttons,save=save,steps=steps,source=p['image']))
 return out
