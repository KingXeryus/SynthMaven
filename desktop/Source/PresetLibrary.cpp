/* SPDX-License-Identifier: GPL-3.0-only
 * Original SynthMaven performance recipes and modeled world-instrument voices.
 * Commercial Yamaha/TAQS.IM samples and preset binaries are not included.
 */
#include "PresetLibrary.h"
#include "BinaryData.h"
#include <cstring>
using namespace juce;
namespace {
void value(FactorySound&s,int p,int v){auto d=sm_parameter_info(s.engineIndex,p);s.values[(size_t)p]=d.maximum>d.minimum?(float)(jlimit(d.minimum,d.maximum,v)-d.minimum)/(d.maximum-d.minimum):0;}
int find(const std::vector<FactorySound>&sounds,int engine,const String&name){for(size_t i=0;i<449;i++)if(sounds[i].engineIndex==engine&&sounds[i].name.containsIgnoreCase(name))return (int)i;for(size_t i=0;i<449;i++)if(sounds[i].engineIndex==engine)return (int)i;return 0;}
}
void describeFactorySound(FactorySound&s){String n=s.name.toUpperCase();s.collection="Factory";s.character="Classic";s.source="SynthMaven firmware factory";
 if(n.contains("BASS")||n.contains("808")||n.contains("SUB "))s.soundType="Bass";
 else if(n.contains("DRUM")||n.contains("TOM")||n.contains("CONGA")||n.contains("TABLA")||n.contains("KICK")||n.contains("RIM")||n.contains("SNARE")||n.contains("HIHAT")||n.contains("HAT "))s.soundType="Drums & Percussion";
 else if(n.contains("BRASS")||n.contains("HORN")||n.contains("TRUMP"))s.soundType="Brass";
 else if(n.contains("FLUTE")||n.contains("PIPE")||n.contains("REED")||n.contains("SAX")||n.contains("HARMON")||n.contains("CLAR")||n.contains("WHISL"))s.soundType="Woodwinds & Reeds";
 else if(n.contains("PIANO")||n.contains("PNO")||n.contains("RHODES")||n.contains("CLAV")||n.contains("KEY")||n.contains("HARPSICH"))s.soundType="Keys & Piano";
 else if(n.contains("ORGAN")||s.engineIndex==7)s.soundType="Organ";
 else if(n.contains("CHOIR")||n.contains("VOX")||n.contains("VOICE")||s.engineIndex==5)s.soundType="Voice & Choir";
 else if(n.contains("STR")||n.contains("VIOL")||n.contains("CELLO")||n.contains("ENS")||s.engineIndex==15)s.soundType="Strings";
 else if(n.contains("BELL")||n.contains("MALLET")||n.contains("CHIME")||n.contains("VIBRA")||n.contains("MARIM")||n.contains("GAMELAN")||n.contains("MBIRA"))s.soundType="Bells & Mallets";
 else if(n.contains("PLUCK")||n.contains("GTR")||n.contains("GUIT")||n.contains("HARP")||n.contains("SITAR")||n.contains("KOTO")||n.contains("BANJO")||n.contains("DULC"))s.soundType="Plucked & Guitar";
 else if(n.contains("PAD")||n.contains("SLOW")||n.contains("AIR "))s.soundType="Pads";
 else if(n.contains("SEQ")||n.contains("ARP")||s.engineIndex==20)s.soundType="Arps & Sequences";
 else if(n.contains("LEAD")||n.contains(" LD")||n.contains("SOLO"))s.soundType="Leads";
 else if(s.engineIndex==11||s.engineIndex==8||s.engineIndex==18||n.contains("FX")||n.contains("TRAIN")||n.contains("TAKE OFF"))s.soundType="Textures & FX";
 else if(s.engineIndex==10)s.soundType="Plucked & Guitar";else s.soundType="Synth";
 if(s.soundType=="Plucked & Guitar"||s.soundType=="Woodwinds & Reeds"||s.soundType=="Strings"||s.soundType=="Drums & Percussion")s.character+=" Acoustic-inspired";
 if(n.contains("DARK"))s.character+=" Dark";if(n.contains("BRIGHT"))s.character+=" Bright";if(n.contains("WARM"))s.character+=" Warm";
}
void appendSynthMavenLibrary(std::vector<FactorySound>&sounds){
 // Six deliberate performance articulations per original program. These are
 // labelled variations, not claims of thousands of unrelated source patches.
 const int originals=(int)sounds.size();const char*styles[]={"Studio","Warm","Wide","Pluck","Pad","Motion"};
 for(int base=0;base<originals;base++)for(int style=0;style<6;style++){
  FactorySound s=sounds[(size_t)base];s.name+=" / "+String(styles[style]);s.collection="Performance Variations";s.source="Original SynthMaven articulation recipe from "+sounds[(size_t)base].name;
  switch(style){
   case 0:value(s,33,0);value(s,34,8);value(s,35,10);value(s,36,18);value(s,89,0);value(s,91,12);s.character="Clean Dry";break;
   case 1:value(s,96,-22);value(s,50,-12);value(s,153,10);value(s,34,18);value(s,36,28);s.character="Warm Dark";break;
   case 2:value(s,34,78);value(s,36,48);value(s,91,76);value(s,44,20);value(s,99,52);s.character="Wide Spacious";break;
   case 3:value(s,1,0);value(s,2,62);value(s,3,24);value(s,4,34);value(s,5,20);value(s,34,12);value(s,35,26);value(s,36,24);s.soundType="Plucked & Guitar";s.character="Short Bright";break;
   case 4:value(s,1,68);value(s,2,95);value(s,3,110);value(s,4,88);value(s,34,56);value(s,36,64);value(s,91,58);s.soundType="Pads";s.character="Soft Spacious";break;
   case 5:value(s,9,46);value(s,10,0);value(s,14,22);value(s,15,12);value(s,16,10);value(s,35,42);value(s,36,38);value(s,98,60);s.character="Animated Rhythmic";break;
  }sounds.push_back(std::move(s));
 }
 struct World{const char*name;int engine;const char*base;const char*type;int e[8];int attack,decay,sustain,release;bool lead;};
 const World world[]={
  {"Oud Amber",10,"NYLON","Plucked & Guitar",{1,34,64,72,34,96,0,18},0,94,127,48,false},
  {"Oud Silk",10,"STR PLUCK","Plucked & Guitar",{1,45,48,80,44,82,0,9},0,96,127,58,false},
  {"Electric Oud",10,"STEEL","Plucked & Guitar",{1,24,94,90,22,110,0,28},0,90,127,66,false},
  {"Qanun Pearl",10,"DULCIMER","Plucked & Guitar",{1,64,110,100,12,108,0,18},0,100,127,82,false},
  {"Qanun Felt",10,"DULCIMER","Plucked & Guitar",{1,59,60,88,28,90,0,14},0,96,127,65,false},
  {"Baglama Wire",10,"SITAR","Plucked & Guitar",{3,32,96,92,60,108,72,12},0,100,127,62,false},
  {"Saz Long Neck",10,"SITAR","Plucked & Guitar",{3,16,100,97,48,112,100,18},0,100,127,74,false},
  {"Buzuk Copper",10,"SITAR","Plucked & Guitar",{3,50,104,86,52,112,84,18},0,96,127,54,false},
  {"Tanbur Cedar",10,"TANPURA","Plucked & Guitar",{3,0,70,116,74,92,80,8},0,100,127,94,false},
  {"Santur Glass",10,"DULCIMER","Bells & Mallets",{1,82,112,108,10,105,0,10},0,100,127,88,false},
  {"Semsemeya Lyre",10,"PHYS HARP","Plucked & Guitar",{3,8,85,80,42,100,14,26},0,96,127,56,false},
  {"Levant Lyre",10,"PHYS HARP","Plucked & Guitar",{3,24,66,94,64,90,0,12},0,100,127,78,false},
  {"Dutar Wood",10,"NYLON","Plucked & Guitar",{1,24,74,83,36,94,0,20},0,100,127,52,false},
  {"Silk Road Koto",10,"KOTO","Plucked & Guitar",{1,17,88,92,18,108,0,12},0,100,127,76,false},
  {"Shamisen Pick",10,"BANJO","Plucked & Guitar",{1,18,110,70,12,120,0,36},0,90,127,42,false},
  {"Pipa Bamboo",10,"STR PLUCK","Plucked & Guitar",{1,30,96,76,28,116,0,24},0,96,127,48,false},
  {"Ney Dawn",9,"FLUTE 1","Woodwinds & Reeds",{-1},14,82,110,38,true},
  {"Kawala Breath",9,"FLUTE 1","Woodwinds & Reeds",{-1},10,78,108,32,true},
  {"Duduk Velvet",5,"HARMONICA","Woodwinds & Reeds",{96,108,18,-3,76,38,52,8},16,88,112,46,true},
  {"Mey Ember",5,"HARMONICA","Woodwinds & Reeds",{76,92,12,-2,104,26,62,6},9,84,115,32,true},
  {"Balaban Warm",5,"HARMONICA","Woodwinds & Reeds",{84,110,16,-4,88,32,64,5},12,86,114,42,true},
  {"Mizmar Fire",5,"HARMONICA","Woodwinds & Reeds",{48,72,14,2,127,12,108,4},2,78,118,24,true},
  {"Zurna Festival",5,"TALKBOX","Woodwinds & Reeds",{38,62,9,2,127,16,112,7},1,72,120,22,true},
  {"Shehnai Gold",5,"HARMONICA","Woodwinds & Reeds",{60,82,22,1,118,24,88,6},8,80,116,36,true},
  {"Kaval Air",9,"FLUTE 1","Woodwinds & Reeds",{-1},17,84,112,44,true},
  {"Arab Clarinet",9,"PIPES 1","Woodwinds & Reeds",{-1},7,84,112,34,true},
  {"Accordion Reed",5,"HARMONICA","Woodwinds & Reeds",{82,24,20,0,115,12,78,2},5,88,120,35,false},
  {"Shawm Copper",5,"HARMONICA","Woodwinds & Reeds",{44,72,12,2,124,10,100,8},3,80,118,28,true},
  {"Arab Strings",15,"VIOLINS","Strings",{20,127,96,102,96,55,14,16},30,100,120,66,false},
  {"Turk Ensemble",15,"ENSEMBLE","Strings",{46,124,104,116,102,64,25,18},34,100,122,74,false},
  {"Persian Strings",15,"CELLOS","Strings",{90,120,54,108,84,46,32,20},46,100,120,82,false},
  {"Kamanche Bow",10,"VIOLIN BOW","Strings",{0,48,70,114,14,84,96,4},34,90,127,62,true},
  {"Rebab Desert",10,"VIOLIN BOW","Strings",{0,36,62,116,18,82,104,10},28,90,127,68,true},
  {"Erhu Silk",10,"VIOLIN BOW","Strings",{0,44,54,116,12,76,94,4},40,92,127,76,true},
  {"Silk Ensemble",15,"SLOW STR","Strings",{50,124,72,112,82,40,22,16},76,100,124,96,false},
  {"Levant Cloud",22,"CHOIR VS","Pads",{-1},78,100,120,98,false},
  {"Desert Ribbon",0,"SYN LEAD","Leads",{-1},2,76,112,35,true},
  {"Dabke Pulse",0,"FUNK","Leads",{-1},0,70,118,24,true},
  {"Shabi Saw",21,"LEAD","Leads",{-1},1,76,116,30,true},
  {"Rai Brass",9,"BRASS 1","Brass",{-1},4,80,108,34,true},
  {"Balkan Pulse",14,"ACID","Leads",{-1},1,72,116,26,true},
  {"Turk Solo",17,"LEAD","Leads",{-1},3,78,114,38,true},
  {"Golden Reed",19,"BRASS","Leads",{-1},5,84,116,38,true},
  {"Oriental Vox",5,"AAH LEAD","Voice & Choir",{8,58,34,0,102,16,86,6},4,80,114,42,true},
  {"Darbuka Doum",10,"HAND DRUM","Drums & Percussion",{2,98,60,72,28,116,34,28},0,94,127,38,false},
  {"Darbuka Tek",10,"HAND DRUM","Drums & Percussion",{2,72,114,46,112,120,8,65},0,82,127,22,false},
  {"Frame Drum",10,"HAND DRUM","Drums & Percussion",{2,32,66,78,28,108,12,32},0,94,127,46,false},
  {"Riq Rim",10,"WOOD BLOCK","Drums & Percussion",{0,100,122,38,104,118,0,82},0,80,127,22,false},
  {"Daf Bronze",10,"HAND DRUM","Drums & Percussion",{2,46,100,92,66,118,14,54},0,100,127,60,false},
  {"Bendir Low",10,"HAND DRUM","Drums & Percussion",{2,24,50,88,32,112,20,28},0,100,127,58,false},
  {"Tabla Hand",10,"TABLA","Drums & Percussion",{2,118,84,76,62,118,24,32},0,98,127,44,false},
  {"Doira Skin",10,"HAND DRUM","Drums & Percussion",{2,76,94,74,82,116,16,48},0,94,127,36,false},
 };
 const char*articulations[]={"Dry","Live","Expressive","Hall"};
 for(const auto&w:world)for(int variation=0;variation<4;variation++){
  FactorySound s=sounds[(size_t)find(sounds,w.engine,w.base)];s.name="OR "+String(w.name)+" / "+articulations[variation];s.soundType=w.type;s.collection="Oriental & Middle Eastern";s.character="Oriental Acoustic-inspired";s.source="Original SynthMaven modeled-instrument recipe; no commercial sample or preset data";
  for(int p=0;p<8;p++)if(w.e[0]>=0)value(s,118+p,w.e[p]);value(s,1,w.attack);value(s,2,w.decay);value(s,3,w.sustain);value(s,4,w.release);value(s,0,98);value(s,33,0);value(s,34,variation==2?32:10);value(s,35,variation==0?0:variation==3?38:18);value(s,36,variation==0?8:variation==3?70:34);value(s,37,w.lead?2:0);value(s,38,w.lead?(variation==2?54:30):0);value(s,41,1);value(s,28,0);value(s,9,78);value(s,13,w.lead?(variation==2?4:2):0);value(s,12,w.lead?45:0);value(s,47,2);value(s,48,3);value(s,96,variation==0?-6:variation==1?4:variation==2?12:-12);value(s,99,variation==3?62:20);if(variation==0)s.character+=" Dry";if(variation==1)s.character+=" Bright";if(variation==2)s.character+=" Expressive";if(variation==3)s.character+=" Spacious";sounds.push_back(std::move(s));
 }
 // The YSE repository explicitly authors and licenses this seed bank CC0-1.0.
 int length=0;const char*bank=nullptr;for(int i=0;i<BinaryData::namedResourceListSize;i++)if(String(BinaryData::originalFilenames[i]).contains("YSE-Originals"))bank=BinaryData::getNamedResource(BinaryData::namedResourceList[i],length);
 if(bank&&length==4104){auto*b=(const uint8_t*)bank;int base=find(sounds,9,"");for(int i=0;i<32;i++){FactorySound s=sounds[(size_t)base];s.name=String::fromUTF8(bank+6+i*128+118,10).trim()+" #"+String(i+1);s.collection="YSE CC0 FM";s.source="YSE original 32-voice bank / CC0-1.0";s.character="Digital Classic";s.soundType=i%4==0?"Synth":i%4==1?"Keys & Piano":i%4==2?"Bells & Mallets":"Brass";s.customKind=9;std::memcpy(s.customPatch.data(),b+6+i*128,128);value(s,1,0);value(s,3,110);value(s,33,0);value(s,34,0);value(s,35,0);value(s,36,24);sounds.push_back(std::move(s));}}
}
