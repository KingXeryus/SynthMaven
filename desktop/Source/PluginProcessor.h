/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include "OscillatorRack.h"
#include "MaqamPresets.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "SynthMavenDSP.h"
#include "ScaleTuning.h"
#include "SamplerEngine.h"
#include <array>
#include <atomic>
struct FactorySound { juce::String name,engine,soundType,collection,character,source; int engineIndex=0,catalogIndex=0,baseProgram=0,customKind=0;std::array<uint8_t,128>customPatch{}; std::vector<float> values; };
class SynthMavenEditor;
class SynthMavenProcessor final : public juce::AudioProcessor, private juce::Timer {
public:
 static constexpr int layerCount=8,catalogCount=24;
 SynthMavenProcessor();~SynthMavenProcessor() override;
 void prepareToPlay(double,int) override;void releaseResources() override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 bool isBusesLayoutSupported(const BusesLayout&)const override;
 juce::AudioProcessorEditor* createEditor() override;bool hasEditor()const override{return true;}
 const juce::String getName()const override{return "SynthMaven";}
 bool acceptsMidi()const override{return true;}bool producesMidi()const override{return false;}
 double getTailLengthSeconds()const override{return 30.;}
 int getNumPrograms()override;int getCurrentProgram()override{return layerProgram(selectedLayer.load());}
 void setCurrentProgram(int)override;const juce::String getProgramName(int)override;void changeProgramName(int,const juce::String&)override{}
 void getStateInformation(juce::MemoryBlock&)override;void setStateInformation(const void*,int)override;
 static const std::vector<FactorySound>& factory();static juce::String catalogName(int);static int catalogSize(int);
 int selectedEngine(int layer=-1)const;int layerProgram(int)const;void setLayerProgram(int layer,int program);void selectLayer(int);
 static int getNumFactory(){return (int)factory().size();}static bool visibleParameter(int);
 juce::String parameterId(int,int layer=-1)const;static juce::String layerId(int,const juce::String&);
 juce::String parameterLabel(int,int layer=-1)const;juce::String parameterText(int,float,int layer=-1)const;
 void savePreset(const juce::File&);bool loadPreset(const juce::File&,juce::String&,int voice=0);
 static juce::StringArray dx7VoiceNames(const juce::File&,juce::String&);juce::String displayName(int layer=-1)const;
 bool loadSample(int,const juce::File&,juce::String&);void clearSample(int);void activateSample(int);
 std::shared_ptr<const MavenSample> sampleForLayer(int)const;
 bool sampleEnabled(int)const;juce::String layerInstrument(int)const;
 static juce::String sampleId(int,const juce::String&);static const char*const sampleKeys[19];
 std::array<std::atomic<float>,9> meters{};
 void requestPanic(){panic.store(true);}void beginLearn(const juce::String&);void cancelLearn();void removeMapping(const juce::String&);
 bool assignMapping(int channel,int cc,const juce::String& target);juce::String mappingDescription(const juce::String&)const;
 juce::String learningDescription()const;juce::String controllerStatus()const;void clearMappings();
 static juce::String tuningId(int,const juce::String&);
 bool loadScala(int,const juce::String&,juce::String&);juce::String scaleDescription(int)const;
 void useScaleLibrary(const juce::File&);int scaleDegree(int,int)const;unsigned scaleMask(int)const;void setScaleMask(int,unsigned);void applyMaqam(int,int,bool descending=false);juce::String tuningName(int)const;bool storeUserScale(int,int,const juce::String&);void recallUserScale(int,int);juce::String userScaleName(int,int)const;
 void resetTuning(int);void tuningPreset(int,int);void storeTuning(int,int);void recallTuning(int,int);
 double tuningOffset(int,int)const;double appliedTuningOffset(int layer,int note)const{return sm_tuning_offset(layers[(size_t)layer].dsp,note);}
 std::atomic<int> selectedLayer{0};std::atomic<unsigned> uiRevision{0};
 juce::AudioProcessorValueTreeState parameters;juce::MidiKeyboardState keyboard;
 std::array<std::atomic<float>,512> scope{};std::atomic<unsigned> scopeWrite{0};std::atomic<float> peak{0};
private:
 struct Layer {
  sm_context*dsp=nullptr;std::atomic<int>program{0},pending{-1},generation{0},midiProgram{-1};
  std::vector<std::atomic<float>*> values;std::vector<juce::RangedAudioParameter*> params;std::vector<float>applied;
  std::atomic<int>patchKind{0},patchVersion{0};std::array<std::atomic<uint8_t>,128>patchBytes{};int appliedPatch=-1;
  std::atomic<float>*enabled=nullptr,*gain=nullptr,*pan=nullptr,*transpose=nullptr,*channel=nullptr,*portamento=nullptr,*mute=nullptr,*solo=nullptr,*split=nullptr,*splitPoint=nullptr,*splitSide=nullptr,*sampleOn=nullptr;
  std::array<std::atomic<float>*,19> sampleValues{};MavenSampler sampler;std::unique_ptr<OscillatorRack>rack;std::array<std::atomic<float>*,24>rackValues{};std::shared_ptr<const MavenSample> sampleData;float blockPeak=0;int activeSample=-1,activeSplit=-1,activePoint=-1,activeSide=-1;
  int tuningTarget=-1;unsigned tuningRevision=~0u;int portamentoIndex=-1;bool wasEnabled=false;int activeTranspose=0,activeChannel=0;float leftGain=0,rightGain=0;
 };
 std::array<Layer,layerCount> layers;
 std::atomic<float>*masterGain=nullptr,*masterPan=nullptr,*masterMute=nullptr;float masterL=1,masterR=1;bool anySolo=false;
 juce::CriticalSection sampleLock;std::vector<std::shared_ptr<const MavenSample>>retiredSamples;
 void publishSample(int,std::shared_ptr<const MavenSample>);static std::shared_ptr<const MavenSample> decodeSample(juce::AudioFormatReader&,const juce::String&,juce::String&);

 struct Tuning {std::array<std::atomic<float>,140>scala{};std::array<std::atomic<float>*,12>notes{};std::atomic<float>*root=nullptr,*reference=nullptr,*bypass=nullptr,*scalaOn=nullptr;std::atomic<unsigned>version{0};juce::String source,description;std::atomic<unsigned>mask{4095};std::atomic<int>scalaCount{12};juce::String name{"Equal / Custom"};std::array<juce::ValueTree,24>memories;std::array<float,16>cachedValues{};std::array<double,128>cachedOffsets{};unsigned cachedVersion=~0u,revision=0;};
 std::array<Tuning,9>tunings;std::atomic<float>*masterTuning=nullptr;mutable juce::CriticalSection tuningLock;
 juce::File customScaleLibrary;std::array<juce::ValueTree,24>userScales;void loadUserScales();juce::File userScalesFile()const;
 juce::ValueTree tuningState(int)const;void restoreTuning(int,const juce::ValueTree&);void applyTuning();
 struct HostParameter { juce::String id;juce::RangedAudioParameter*param=nullptr;std::atomic<float>*raw=nullptr;int layer=-1,native=-1; };
 std::vector<HostParameter> allParameters;std::unique_ptr<std::atomic<float>[]>mappedUpdates;
 std::array<std::atomic<int>,17*128>mappings{};std::atomic<int>learnTarget{-1},lastCC{-1},lastChannel{0};
 std::atomic<bool>panic{false};double rate=44100.,phase=0.;float history[4][2]{};bool primed=false;int midiBank[16]{};
 static juce::AudioProcessorValueTreeState::ParameterLayout layout();void timerCallback()override;
 void handleMidi(const juce::MidiMessage&);void pullSample(float*);void queuePatch(int layer,int,const uint8_t*);
 void applyLayers();void flushMappedUpdates();int parameterIndex(const juce::String&)const;void applyMapped(int,float);int defaultCCParameter(int layer,int cc)const;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthMavenProcessor)
};
