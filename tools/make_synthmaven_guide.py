#!/usr/bin/env python3
"""Fixed-page illustrated screen reference. Each screen is exactly one PDF page."""
import argparse,json,math,re
from pathlib import Path
from html import escape
from bs4 import BeautifulSoup
from reportlab.pdfgen import canvas
from reportlab.lib import colors
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle
from reportlab.platypus import Paragraph
from reportlab.lib.utils import ImageReader
from guide_screen_reference import records
ROOT=Path(__file__).resolve().parents[1]
W,H=A4;M=40;WIDTH=W-2*M

def clean(s):
 return str(s).replace('−','-').replace('–','-').replace('—',' - ').replace('→',' > ').replace('‹','<').replace('›','>')
def extra(title,image,controls,steps,section='Getting started',opening=''):
 return dict(title=title,image=image,controls=controls,rows=[],steps=steps,section=section,open=opening,buttons='',save='',source=title)

def pages():
 out=[]
 out.append(extra('Start here: controls and storage','web/manual/controls.png',[
 ('Sound preset','One instrument sound. Factory sounds are read-only starting points; USER slots store individual edited sounds.'),('Program','Four parts with levels, pan, splits and scenes. P1-P8 are separate from the 384 USER sound slots.'),('Project','A-D stores the wider working setup, patterns and arrangement. Save a project after arranging a song.'),('Editor library','A named collection on your browser or iPad; it is not automatically a device save.'),('Master level','The physical MASTER knob controls listening level. Start low and raise gradually. Four layers share one processing budget; they do not each have a separate eight-voice allocation.')],['Connect power and let HOME appear. Choose a program, then play at a comfortable level.','Select one part before editing its engine or sound. Save the complete program after mixing.','Export a full backup before updating: a firmware flash restores factory settings and programs.']))
 out.append(extra('Install, back up and recover','web/manual/usb_save.png',[
 ('Installer','Use the SynthMaven installer in a browser supporting Web MIDI/SysEx. Allow MIDI permission and select the FM-1.'),('Backup','Use Projects & backup > Save backup. Keep the downloaded archive outside browser storage.'),('Install','Select the current firmware and follow the installer prompts. Keep USB power connected throughout writing and verification.'),('Recovery','If the normal editor cannot connect, reopen the installer and follow its detected recovery/loader instructions. Use the release recovery package if required; never interrupt a write.'),('After update','Allow the device to restart, reconnect the editor and verify its version. Restore a backup only if you want its previous settings/programs.')],['Download the guide and release files first. Back up the working device.','Run the installer through its final verification and restart.','If a restart stalls after writing has completed, power-cycle and reconnect; do not power-cycle while writing.']))
 out.append(extra('USB computer, iPad and powered-host setup','web/manual/sm_program.png',[
 ('COMPUTER role','Use GLO > USB / SAVE > ROLE = COMPUTER for the web editor or iPad. The computer/iPad is the USB host.'),('HOST role','Use HOST for a class-compliant USB MIDI keyboard, pad controller or pedal connected to the FM-1. This changes the FM-1 to host role. There is no separate keys-only or pedal-only USB mode.'),('Power and hub','The FM-1 cannot power the USB port, so a bus-powered controller needs a powered hub or powered OTG adapter. Connect the hub upstream cable to the FM-1, the controller to a USB-A data port and the charger to the hub power input. Hubs with two internal hub chips are supported. A slow hub can take up to about 3 seconds to be found.'),('TRS together','TRS MIDI IN stays active in either role. With GLO > MIDI/CPU > ROUT = PARTS, MIDI channels 1-4 play layers 1-4, so a USB controller and a TRS controller on different channels can play different layers.'),('Return to editor','One USB port cannot act as device and host simultaneously. Return to COMPUTER, reconnect the computer/iPad cable and choose the FM-1 MIDI ports.'),('iPad','Open SM FM1 in landscape and choose Connect > USB cable. USB Audio defaults to IN+OUT. iPadOS may cache the FM-1 name as SoundMaven; the corrected app also checks its USB identity and selects matching input/output automatically. Bluetooth is excluded.')],['Start in COMPUTER and confirm a normal editor connection.','Disconnect the editor, choose HOST, connect the powered hub with the controller, wait up to 3 seconds and verify received MIDI.','To edit again, return to COMPUTER before reconnecting the iPad or computer.']))
 out.append(extra('HOME: four-part program','web/manual/home.png',[
 ('SELECT','Choose the focused part, 1-4.'),('ALGORITHM','Cycle engines for that part.'),('PRESET','Cycle sounds within the selected engine.'),('OCT- / OCT+','A short press transposes the keyboard. On HOME, hold OCT- for three seconds for Program Down; hold OCT+ for Program Up.'),('EDIT / GLO holds','Hold EDIT two seconds for the persistent four-volume mixer. Hold GLO two seconds to toggle layering and display Layered ON/OFF.'),('HOME','Press HOME again to switch to the independent drum screen. Ordinary page controls are documented on their own pages.')],['Choose a program. Focus a part with SELECT.','Choose its engine and preset; keep the other three parts as support.','Hold EDIT to balance the four volumes, then SAVE the program.'],opening='Press HOME.'))
 out.append(extra('Shortcuts and secondary actions','web/manual/controls.png',[
 ('EDIT: two seconds','Open the mixer; release the button and continue mixing.'),('GLO: two seconds','Toggle layering, with a temporary Layered ON/OFF notification.'),('ENV: three seconds','Open the chord screen. Normal short ENV presses open envelopes.'),('SEL: three seconds','Open custom microtuning; a short press opens master tuning.'),('OCT buttons: three seconds','On HOME only, previous/next program. Short taps retain octave behavior.'),('SEQ hold','Open pattern selection; the normal hold threshold is about 0.7 seconds.'),('HOME hold','Open system/settings. FX hold is a separate configurable performance overlay; its threshold is set in GLO.'),('Confirm / cancel','OCT+ confirms an offered action; OCT- cancels/backs out. PROGRAM uses its own two-turn save/load confirmation.')],['Use a short press for the normal family page.','For a special screen, hold until it opens, then release.','Use HOME to return; navigating alone does not create a named save.']))
 out.append(extra('NAME: entering a sound or project name','web/manual/name_typing.png',[
 ('KNOB 1 / KNOB 2','Move the cursor / choose the character. KNOBS 3-4 have no naming assignment.'),('White keys','Enter letters with repeated taps, like a phone keypad. Wait briefly to move on; digits are available in number mode. Keys enter text instead of playing notes.'),('Black keys','C#: delete. D#: letters/numbers. F#: cursor left. G#: space. A#: cursor right.'),('OCT+ / OCT-','Confirm the name / cancel. This naming keyboard is separate from the PROGRAM inline name controls.')],['Choose a sound/project Save action and enter the name screen.','Enter a short recognizable name; check the destination.','Confirm with OCT+, or cancel with OCT-.'],section='SAVE',opening='Open a sound/project naming action from SAVE.'))
 out.append(extra('SCALE PICKER: categories and favorites','web/manual/scale_picker_favorites.png',[
 ('Scale list','Browse the available scales; firmware prefixes W/, A/ and T/ keep names short. Editors use the full family names.'),('Favorites','A favorite narrows browsing; it does not change the scale intervals or create a new scale.'),('Confirm / return','OCT+ selects an offered choice. OCT- returns without accepting the picker choice.'),('Microtuning','Use MASTER TUNE for root and first affected octave; CUSTOM TUNE edits individual pitch classes. Lower notes remain equal-tempered below the selected octave.')],['Open the scale setting and its picker.','Choose a family and scale, then confirm.','Play a note above the tuning threshold; fine-tune on CUSTOM TUNE and save a named scale in the editor.'],section='SEL',opening='From the scale choice, open the offered list with OCT+.'))
 out.append(extra('Layering notification','web/manual/sm_layer_notice.png',[
 ('Layered ON','Incoming notes can play all four parts within their split ranges. Levels, pan and mute determine the audible balance.'),('Layered OFF','Play the selected sound rather than the complete four-part stack.'),('GLO','Hold for two seconds to toggle. The notification temporarily replaces the current screen; this is not a separate latched mixer.'),('EDIT','Hold for two seconds for the four-volume mixer. KNOBS 1-4 then adjust the corresponding part levels.')],['Hold GLO until the message appears.','Release and play to compare the two modes.','Save the program when the layer balance is ready.'],section='GLO',opening='Hold GLO for two seconds.'))
 out.extend(records())
 soup=BeautifulSoup((ROOT/'web/manual/index.html').read_text(),'html.parser')
 sec=soup.find(id='editor-walkthrough')
 for heading in sec.find_all('h3'):
  title=heading.get_text(' ',strip=True);fig=heading.find_next_sibling('figure');ol=fig.find_next_sibling('ol')
  steps=[li.get_text(' ',strip=True) for li in ol.find_all('li',recursive=False)]
  out.append(extra('Editor / '+title,'web/manual/'+fig.img['src'],[('Controls and workflow',x) for x in steps],['Connect the real FM-1 in COMPUTER USB role. Mock mode changes demonstration data only.','Choose the part and screen before editing. Touch arrows cycle choices; card tabs reveal the remaining controls.','Save the sound/program/project as appropriate; a browser library copy is separate.'],section='Web and iPad editor',opening='Open the matching editor page shown below.'))
 out.append(extra('Editor / Four-layer melodic sequencer','assets/manual/editor-melodic-sequencer.png',[
 ('Four lanes','Each visible lane belongs to the matching program layer. Tap a step to select its layer and edit that step. The selected lane and step have an outline.'),('Steps','Eight steps are visible at a time. Previous/next step arrows expose the remaining steps without changing the pattern.'),('Notes A0-C8','Choose from all 88 piano notes in each of four note selectors. Use the dash for an empty chord slot. Notes are selectable, not free text.'),('Length / Accent / Slide','NOTE triggers the selected pitches, TIE connects steps, REST leaves silence. Accent and slide retain the firmware behavior.'),('Velocity / Chance','Velocity sets note strength, from 0 to 127. Chance sets probability from 0 to 100 percent.'),('Pattern / Play / Save','Choose one of eight patterns for the selected layer. Play runs on the connected FM-1. Save the project to retain patterns and arrangement.')],['Select a layer and a pattern. Tap a step and choose one or more notes.','Set note length, velocity, chance, accent and slide as needed; repeat for other layers.','Open Projects > Song arranger, arrange the patterns, then save the project. Use Drums for independent drum patterns.'],section='Web and iPad editor',opening='Studio > Sequencer > Steps.'))
 out.append(extra('Editor / Four-track song arranger','assets/manual/editor-arranger.png',[
 ('Timeline','Read sections from left to right. Each column contains four color-coded layer patterns, a repeat count and reorder/copy/delete controls.'),('Track 1-4 patterns','Each section selects one of eight patterns independently for each track. Side arrows cycle the pattern number; track colors identify the four parts.'),('Repeats','Play this section 1-16 times before moving to the next. A song contains up to 16 ordered sections.'),('Add / Copy / Move earlier / Delete','Add a section, duplicate a section, move it earlier or remove it. Structural edits are disabled while the song is playing.'),('Section pages','Previous/next arrows show four sections at a time. They change only the view, never the order or sound.'),('Play from start / Stop','Play the arrangement on the connected FM-1. The current section and remaining repeats are reported by the device.'),('Projects & backup','Save the current project in A-D to retain the arrangement, then export a backup. Patterns must already contain melodic notes to produce sound; independent drums are edited on the Drums page.')],['Create patterns in Studio > Sequencer for each of the four tracks.','Open Studio > Projects > Song arranger; add sections and choose four patterns and repeats for each.','Play from start, stop to edit, then use Projects & backup to save.'],section='Web and iPad editor',opening='Studio > Projects > Song arranger.'))
 return out

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--version',default='3.5.8');ap.add_argument('--output',type=Path,default=ROOT/'output/pdf/SynthMaven-3.5.8-User-Guide.pdf');args=ap.parse_args()
 specs=pages();indexpages=math.ceil(len(specs)/33);args.output.parent.mkdir(parents=True,exist_ok=True)
 c=canvas.Canvas(str(args.output),pagesize=A4);c.setTitle('SynthMaven '+args.version+' - Screen-by-screen User Guide');manifest=[]
 def text(txt,x,y,w,size=10,bold=False):
  st=ParagraphStyle('p',fontName='Helvetica-Bold' if bold else 'Helvetica',fontSize=size,leading=size*1.26,textColor=colors.HexColor('#222832'))
  p=Paragraph(escape(clean(txt)),st);_,h=p.wrap(w,10000);p.drawOn(c,x,y-h);return y-h
 def footer(n,label):
  c.setStrokeColor(colors.HexColor('#d2d6dc'));c.line(M,32,W-M,32);text('SynthMaven '+args.version+' | '+label,M,24,WIDTH-40,8);text(str(n),W-M-25,24,25,8)
 c.setFillColor(colors.HexColor('#10141b'));c.rect(0,0,W,H,fill=1,stroke=0)
 c.drawImage(str(ROOT/'web/branding/firmware-icon.png'),W/2-85,H-240,170,170,mask='auto',preserveAspectRatio=True)
 c.setFillColor(colors.white);c.setFont('Helvetica-Bold',32);c.drawCentredString(W/2,H-290,'SYNTHMAVEN');c.setFont('Helvetica',22);c.drawCentredString(W/2,H-330,args.version+' USER GUIDE');c.setFont('Helvetica',14);c.drawCentredString(W/2,H-370,'One screen. One page. Every control.');c.setFont('Helvetica',11);c.drawCentredString(W/2,180,'M-WAVE FM-1 | Firmware, web editor and iPad editor');c.drawCentredString(W/2,158,'Screens rendered from firmware; editor examples may use mock data.');c.drawCentredString(W/2,136,'Based on Melodee and the credited SynthMaven / Clipper lineage.');c.showPage()
 for j in range(indexpages):
  text('SCREEN INDEX',M,H-40,WIDTH,20,True);y=H-78
  for i,p in list(enumerate(specs))[j*33:(j+1)*33]:
   num=2+indexpages+i
   text(p['title'],M,y,WIDTH-40,9);text(str(num),W-M-30,y,30,9);c.linkRect('',f'p{i}',(M,y-17,W-M,y),relative=0,thickness=0);y-=21
  footer(j+2,'Screen index');c.showPage()
 for i,p in enumerate(specs):
  number=2+indexpages+i;c.bookmarkPage(f'p{i}');c.addOutlineEntry(p['title'],f'p{i}',0)
  text(p['section'].upper(),M,H-35,WIDTH,9,True);y=text(p['title'],M,H-54,WIDTH,19,True)-14
  image=ROOT/p['image'];is_editor=p['section']=='Web and iPad editor'
  if not image.exists():raise FileNotFoundError(image)
  iw,ih=ImageReader(str(image)).getSize();h=170 if is_editor else 137;w=min(WIDTH if is_editor else 180,h*iw/ih);h=w*ih/iw
  c.drawImage(str(image),M,y-h,w,h,mask='auto');
  if not is_editor:text('OPEN THIS SCREEN\n'+p.get('open',''),M+w+16,y,WIDTH-w-16,10)
  y-=h+12
  blocks=[]
  if is_editor and p.get('open'):blocks.append(('Open',p['open']))
  blocks+=p['controls']+p.get('rows',[])
  if p.get('buttons'):blocks.append(('BUTTONS AND NAVIGATION',p['buttons']))
  if p.get('save'):blocks.append(('KEEP YOUR CHANGES',p['save']))
  blocks.append(('STEP BY STEP',' '.join(f'{k+1}. {v}' for k,v in enumerate(p['steps']))))
  # Measure the full page before drawing; never split a screen across PDF pages.
  size=10
  def height(size):
   result=0
   for label,body in blocks:
    for val,width,bold in [(label,108,True),(body,WIDTH-120,False)]:
     st=ParagraphStyle('m',fontName='Helvetica-Bold' if bold else 'Helvetica',fontSize=size,leading=size*1.26);_,hh=Paragraph(escape(clean(val)),st).wrap(width,10000)
     if bold:lh=hh
     else:result+=max(lh,hh)+9
   return result
  while height(size)>y-45 and size>8:size-=.25
  if height(size)>y-45:raise RuntimeError(f'Page overflow: {p["title"]} ({height(size)} / {y-45})')
  for label,body in blocks:
   a=text(label,M,y,108,size,True);b=text(body,M+120,y,WIDTH-120,size);y=min(a,b)-9
   c.setStrokeColor(colors.HexColor('#e4e7eb'));c.line(M,y+4,W-M,y+4)
  footer(number,p['section']);c.showPage();manifest.append(dict(page=number,title=p['title'],source=p['source'],font_size=size,bottom=y))
 c.save();(args.output.parent/'guide-screen-coverage.json').write_text(json.dumps(manifest,indent=2)+'\n');print(f'{len(specs)} dedicated screen/workflow pages, {1+indexpages+len(specs)} total pages: {args.output}')
if __name__=='__main__':main()
