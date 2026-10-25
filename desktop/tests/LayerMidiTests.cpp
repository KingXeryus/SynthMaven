/* SPDX-License-Identifier: GPL-3.0-only */
#include "PluginProcessor.h"
#include <cmath>
#include <cstdio>
using namespace juce;
static void set(SynthMavenProcessor&p,const String&id,float v){auto*x=p.parameters.getParameter(id);if(!x)std::abort();x->setValueNotifyingHost(v);}
static float get(SynthMavenProcessor&p,const String&id){return p.parameters.getRawParameterValue(id)->load();}
static double play(SynthMavenProcessor&p,int channel=1){p.prepareToPlay(48000,256);AudioBuffer<float>b(2,256);MidiBuffer m;double total=0;for(int block=0;block<40;block++){if(block==0)m.addEvent(MidiMessage::noteOn(channel,60,(uint8)100),0);p.processBlock(b,m);for(int i=0;i<256;i++)for(int c=0;c<2;c++){float v=b.getSample(c,i);if(!std::isfinite(v)||std::abs(v)>1.001f)std::abort();total+=std::abs(v);}}p.releaseResources();return total;}
static void cc(SynthMavenProcessor&p,int channel,int controller,int value){AudioBuffer<float>b(2,256);MidiBuffer m;m.addEvent(MidiMessage::controllerEvent(channel,controller,value),0);p.processBlock(b,m);}
int main(){ScopedJuceInitialiser_GUI gui;SynthMavenProcessor p;
 int total=0;for(int bank=0;bank<24;bank++){int n=p.catalogSize(bank);if(n<=0)return 1;printf("%s: %d\n",p.catalogName(bank).toRawUTF8(),n);total+=n;}if(total!=p.getNumPrograms()||p.catalogSize(23)<9)return 2;
 int programs[8]{};for(int l=0;l<8;l++){int engine=l==7?23:l;for(size_t i=0;i<p.factory().size();i++)if(p.factory()[i].catalogIndex==engine){programs[l]=(int)i;break;}p.setLayerProgram(l,programs[l]);set(p,p.layerId(l,"portamento"),0.f);set(p,p.layerId(l,"gain"),.65f);set(p,p.layerId(l,"pan"),.1f+l*.1f);set(p,p.layerId(l,"transpose"),.5f);set(p,p.layerId(l,"channel"),0);}
 for(int l=0;l<8;l++){for(int i=0;i<8;i++)set(p,p.layerId(i,"enabled"),i==l?1.f:0.f);if(play(p)<=0){printf("Silent layer %d\n",l);return 3;}}
 for(int l=0;l<8;l++)set(p,p.layerId(l,"enabled"),1);if(play(p)<=0)return 4;
 for(int l=0;l<8;l++)set(p,p.layerId(l,"channel"),l==3?2.f/16.f:1.f/16.f);if(play(p,2)<=0)return 5;
 p.prepareToPlay(48000,256);if(!p.assignMapping(2,21,p.parameterId(96,3)))return 6;cc(p,1,21,110);if(std::abs(get(p,p.parameterId(96,3))-110.f/127.f)<.001)return 7;cc(p,2,21,110);if(std::abs(get(p,p.parameterId(96,3))-110.f/127.f)>.001)return 8;
 p.beginLearn(p.layerId(7,"gain"));cc(p,6,22,64);if(std::abs(get(p,p.layerId(7,"gain"))-64.f/127.f)>.001||!p.mappingDescription(p.layerId(7,"gain")).contains("22"))return 9;
 // Portamento controllers must affect only layers accepting the incoming channel.
 cc(p,2,65,127);if(get(p,p.layerId(3,"portamento"))<.5f||get(p,p.layerId(0,"portamento"))>.5f)return 10;cc(p,2,5,90);if(std::abs(get(p,p.parameterId(38,3))-90.f/127.f)>.001)return 11;
 MemoryBlock saved;p.selectLayer(7);p.getStateInformation(saved);SynthMavenProcessor restored;restored.setStateInformation(saved.getData(),(int)saved.getSize());for(int l=0;l<8;l++){if(restored.layerProgram(l)!=programs[l]||get(restored,p.layerId(l,"enabled"))<.5f)return 12;}if(restored.selectedLayer.load()!=7||restored.mappingDescription(restored.layerId(7,"gain"))!=p.mappingDescription(p.layerId(7,"gain")))return 13;
 if(play(restored,2)<=0)return 14;restored.prepareToPlay(44100,128);cc(restored,6,22,100);if(std::abs(get(restored,restored.layerId(7,"gain"))-100.f/127.f)>.001)return 15;
 // A 0.1 state remains loadable and resets added layers to disabled.
 ValueTree legacy("SynthMavenState");legacy.setProperty("program",269,nullptr);ValueTree value("PARAM");value.setProperty("id","p96",nullptr);value.setProperty("value",.3f,nullptr);legacy.addChild(value,-1,nullptr);auto xml=legacy.createXml();MemoryBlock old;AudioProcessor::copyXmlToBinary(*xml,old);restored.setStateInformation(old.getData(),(int)old.getSize());if(restored.layerProgram(0)!=269||get(restored,restored.layerId(1,"enabled"))>.5f)return 16;
 puts("PASS: 24 populated banks / 449 presets; eight distinct-engine layers; MIDI channels; CC mapping/Learn; portamento CCs; complete layered recall; legacy preset migration.");return 0;}
