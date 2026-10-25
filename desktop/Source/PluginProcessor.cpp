/* SPDX-License-Identifier: GPL-3.0-only */
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
using namespace juce;
const std::vector<FactorySound>& SynthMavenProcessor::factory(){
 static const auto sounds=[] {std::vector<FactorySound> result;sm_context *c=sm_create();if(!c)return result;
  for(int i=0;i<sm_program_count();++i){sm_select_program(c,i);FactorySound x;x.name=sm_program_name(i);x.engineIndex=sm_program_engine(i);x.engine=sm_engine_name(x.engineIndex);for(int p=0;p<sm_parameter_count();++p)x.values.push_back(sm_get_parameter(c,p));result.push_back(std::move(x));}sm_destroy(c);return result;}();return sounds;
}
bool SynthMavenProcessor::visibleParameter(int i){return i<138||(i>=150&&i!=157);}
void SynthMavenProcessor::timerCallback(){int p=midiProgram.exchange(-1);if(p>=0)setCurrentProgram(p);}
AudioProcessorValueTreeState::ParameterLayout SynthMavenProcessor::layout(){
 AudioProcessorValueTreeState::ParameterLayout out;
 for(int i=0;i<sm_parameter_count();++i){if(!visibleParameter(i))continue;auto d=sm_parameter_info(0,i);String name=String(d.name?d.name:"")+" "+String(i+1);out.add(std::make_unique<AudioParameterFloat>(ParameterID("p"+String(i),1),name,NormalisableRange<float>(0,1),0));}return out;
}
SynthMavenProcessor::SynthMavenProcessor():AudioProcessor(BusesProperties().withOutput("Stereo",AudioChannelSet::stereo(),true)),parameters(*this,nullptr,"SynthMavenState",layout()){
 dsp=sm_create();for(int i=0;i<sm_parameter_count();++i){values.push_back(parameters.getRawParameterValue("p"+String(i)));hostParameters.push_back(parameters.getParameter("p"+String(i)));}applied.assign(values.size(),-100.f);
 for(auto &s:scope)s.store(0);int initial=0;for(size_t i=0;i<factory().size();++i)if(factory()[i].engine=="HYBRID"){initial=(int)i;break;}setCurrentProgram(initial);startTimerHz(30);
}
SynthMavenProcessor::~SynthMavenProcessor(){stopTimer();sm_destroy(dsp);}
bool SynthMavenProcessor::isBusesLayoutSupported(const BusesLayout& b)const{return b.getMainInputChannelSet().isDisabled()&&(b.getMainOutputChannelSet()==AudioChannelSet::stereo()||b.getMainOutputChannelSet()==AudioChannelSet::mono());}
int SynthMavenProcessor::getNumPrograms(){return (int)factory().size();}
const String SynthMavenProcessor::getProgramName(int i){return isPositiveAndBelow(i,getNumPrograms())?factory()[(size_t)i].name:String();}
void SynthMavenProcessor::setCurrentProgram(int i){if(!isPositiveAndBelow(i,getNumPrograms()))return;programGeneration.fetch_add(1);currentProgram.store(i);const auto&f=factory()[(size_t)i];for(size_t p=0;p<f.values.size();++p)if(hostParameters[p])hostParameters[p]->setValueNotifyingHost(jlimit(0.f,1.f,f.values[p]));patchKind.store(0);pendingProgram.store(i);programGeneration.fetch_add(1);}
void SynthMavenProcessor::prepareToPlay(double sr,int){rate=sr>0?sr:44100.;phase=0;primed=false;std::fill(applied.begin(),applied.end(),-100.f);pendingProgram.store(currentProgram.load());setLatencySamples((int)std::ceil((sm_control_period()+2)*rate/44100.));}
void SynthMavenProcessor::releaseResources(){if(dsp)sm_panic(dsp);primed=false;}
void SynthMavenProcessor::handleMidi(const MidiMessage&m){
 int ch=jlimit(0,15,m.getChannel()-1);if(m.isController()&&m.getControllerNumber()==0)midiBank[ch]=m.getControllerValue();
 if(m.isProgramChange()){int i=midiBank[ch]*128+m.getProgramChangeNumber();if(i<getNumPrograms())midiProgram.store(i);return;}
 if(m.getRawDataSize()<=3)sm_midi(dsp,m.getRawData(),m.getRawDataSize());
}
void SynthMavenProcessor::pullSample(float* s){sm_render(dsp,s,1);}
void SynthMavenProcessor::processBlock(AudioBuffer<float>&buffer,MidiBuffer&midi){
 ScopedNoDenormals guard;buffer.clear();if(!dsp)return;keyboard.processNextMidiBuffer(midi,0,buffer.getNumSamples(),true);
 if(panic.exchange(false))sm_panic(dsp);
 unsigned generation=programGeneration.load();if(!(generation&1u)){
  int next=pendingProgram.exchange(-1);if(next>=0){sm_select_program(dsp,next);std::fill(applied.begin(),applied.end(),-100.f);primed=false;phase=0;appliedPatch=-1;}
  for(size_t i=0;i<values.size();++i){if(!values[i])continue;float v=values[i]->load();if(v!=applied[i]){sm_set_parameter(dsp,(int)i,v);applied[i]=v;}}
 }
 int pv=patchVersion.load();if(!(pv&1)&&pv!=appliedPatch){uint8_t bytes[128];for(int i=0;i<128;i++)bytes[i]=patchBytes[(size_t)i].load();if(pv==patchVersion.load()){int kind=patchKind.load();if(kind)sm_custom_patch(dsp,kind,bytes,128);appliedPatch=pv;}}
 if(auto *playhead=getPlayHead())if(auto p=playhead->getPosition())if(auto bpm=p->getBpm())sm_tempo(dsp,*bpm);
 MidiBuffer::Iterator it(midi);MidiMessage message;int position=0;bool has=it.getNextEvent(message,position);float maxLevel=0;
 for(int i=0;i<buffer.getNumSamples();++i){while(has&&position<=i){handleMidi(message);has=it.getNextEvent(message,position);}
  if(!primed){for(auto &s:history)pullSample(s);primed=true;}
  while(phase>=1.){for(int k=0;k<3;k++)for(int ch=0;ch<2;ch++)history[k][ch]=history[k+1][ch];pullSample(history[3]);phase-=1.;}
  float s[2];float t=(float)phase;for(int c=0;c<2;c++){float a=history[0][c],b=history[1][c],d=history[2][c],e=history[3][c];s[c]=jlimit(-1.f,1.f,b+.5f*t*(d-a+t*(2*a-5*b+4*d-e+t*(3*(b-d)+e-a))));maxLevel=jmax(maxLevel,std::abs(s[c]));}
  if(buffer.getNumChannels()==1)buffer.setSample(0,i,.5f*(s[0]+s[1]));else{buffer.setSample(0,i,s[0]);buffer.setSample(1,i,s[1]);}
  scope[scopeWrite.fetch_add(1)%512].store(.5f*(s[0]+s[1]));phase+=44100./rate;
 }
 peak.store(maxLevel);midi.clear();
}
String SynthMavenProcessor::parameterLabel(int p)const{auto d=sm_parameter_info(selectedEngine(),p);return String(d.name?d.name:"");}
String SynthMavenProcessor::parameterText(int p,float v)const{auto d=sm_parameter_info(selectedEngine(),p);int n=d.minimum+roundToInt(v*(d.maximum-d.minimum));auto s=sm_parameter_choice(selectedEngine(),p,n);return s&&*s?String(s):String(n);}
void SynthMavenProcessor::getStateInformation(MemoryBlock& b){auto tree=parameters.copyState();tree.setProperty("program",currentProgram.load(),nullptr);int kind=patchKind.load();if(kind){uint8_t bytes[128];for(int i=0;i<128;i++)bytes[i]=patchBytes[(size_t)i].load();MemoryBlock patch(bytes,128);tree.setProperty("patchKind",kind,nullptr);tree.setProperty("patch",patch.toBase64Encoding(),nullptr);}auto xml=tree.createXml();copyXmlToBinary(*xml,b);}
void SynthMavenProcessor::setStateInformation(const void* data,int n){auto xml=getXmlFromBinary(data,n);if(!xml||!xml->hasTagName("SynthMavenState"))return;auto tree=ValueTree::fromXml(*xml);int p=jlimit(0,getNumPrograms()-1,(int)tree.getProperty("program",0));programGeneration.fetch_add(1);currentProgram.store(p);patchKind.store(0);parameters.replaceState(tree);MemoryBlock patch;if(patch.fromBase64Encoding(tree.getProperty("patch").toString())&&patch.getSize()==128)queuePatch((int)tree.getProperty("patchKind",9),(const uint8_t*)patch.getData());pendingProgram.store(p);programGeneration.fetch_add(1);}
void SynthMavenProcessor::savePreset(const File&f){MemoryBlock b;getStateInformation(b);f.replaceWithData(b.getData(),b.getSize());}
void SynthMavenProcessor::queuePatch(int kind,const uint8_t*b){patchVersion.fetch_add(1);patchKind.store(kind);for(int i=0;i<128;i++)patchBytes[(size_t)i].store(b[i]);patchVersion.fetch_add(1);}
namespace {
int dx7Offset(const MemoryBlock&b,String&error){const auto*s=(const uint8_t*)b.getData();size_t size=b.getSize();for(size_t off=0;off+4104<=size;off++)if(s[off]==0xf0&&s[off+1]==0x43&&s[off+3]==9&&s[off+4]==0x20&&s[off+5]==0&&s[off+4103]==0xf7){unsigned sum=0;for(size_t k=off+6;k<off+4102;k++){if(s[k]>127){error="Invalid DX7 bank data.";return -1;}sum+=s[k];}if(((128-(sum&127))&127)!=s[off+4102]){error="DX7 checksum mismatch.";return -1;}return (int)off;}error="Expected a valid 32-voice DX7 bulk SysEx bank.";return -1;}
}
StringArray SynthMavenProcessor::dx7VoiceNames(const File&f,String&error){MemoryBlock b;StringArray names;if(!f.loadFileAsData(b)){error="Cannot read this file.";return names;}int off=dx7Offset(b,error);if(off<0)return names;auto*s=(const uint8_t*)b.getData();for(int v=0;v<32;v++){String name;for(int i=0;i<10;i++){uint8_t ch=s[off+6+v*128+118+i];name+=String::charToString(ch>=32&&ch<127?(juce_wchar)ch:(juce_wchar)' ');}names.add(name.trim());}return names;}
String SynthMavenProcessor::displayName()const{if(patchKind.load()==9){String name;for(int i=118;i<128;i++){uint8_t ch=patchBytes[(size_t)i].load();name+=String::charToString(ch>=32&&ch<127?(juce_wchar)ch:(juce_wchar)' ');}return name.trim();}return factory()[(size_t)currentProgram.load()].name;}
bool SynthMavenProcessor::loadPreset(const File&f,String&error,int voice){MemoryBlock b;if(!f.loadFileAsData(b)){error="Cannot read this file.";return false;}
 if(f.hasFileExtension("syx")){int off=dx7Offset(b,error);if(off<0)return false;int program=0;for(size_t i=0;i<factory().size();i++)if(factory()[i].engineIndex==9){program=(int)i;break;}setCurrentProgram(program);queuePatch(9,(const uint8_t*)b.getData()+off+6+juce::jlimit(0,31,voice)*128);return true;}
 auto xml=getXmlFromBinary(b.getData(),(int)b.getSize());if(!xml||!xml->hasTagName("SynthMavenState")){error="Not a SynthMaven preset.";return false;}setStateInformation(b.getData(),(int)b.getSize());return true;
}

AudioProcessorEditor* SynthMavenProcessor::createEditor(){return new SynthMavenEditor(*this);}
AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new SynthMavenProcessor();}
