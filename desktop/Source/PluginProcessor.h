/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "SynthMavenDSP.h"
#include <array>
#include <atomic>
struct FactorySound { juce::String name,engine; int engineIndex=0; std::vector<float> values; };
class SynthMavenEditor;
class SynthMavenProcessor final : public juce::AudioProcessor, private juce::Timer {
public:
 SynthMavenProcessor();~SynthMavenProcessor() override;
 void prepareToPlay(double,int) override;void releaseResources() override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 bool isBusesLayoutSupported(const BusesLayout&)const override;
 juce::AudioProcessorEditor* createEditor() override;bool hasEditor()const override{return true;}
 const juce::String getName()const override{return "SynthMaven";}
 bool acceptsMidi()const override{return true;}bool producesMidi()const override{return false;}
 double getTailLengthSeconds()const override{return 30.;}
 int getNumPrograms()override;int getCurrentProgram()override{return currentProgram.load();}
 void setCurrentProgram(int)override;const juce::String getProgramName(int)override;void changeProgramName(int,const juce::String&)override{}
 void getStateInformation(juce::MemoryBlock&)override;void setStateInformation(const void*,int)override;
 static const std::vector<FactorySound>& factory();
 int selectedEngine()const{return factory()[(size_t)juce::jlimit(0,getNumFactory()-1,currentProgram.load())].engineIndex;}
 static int getNumFactory(){return (int)factory().size();}
 static bool visibleParameter(int);
 juce::String parameterLabel(int)const;juce::String parameterText(int,float)const;
 void savePreset(const juce::File&);bool loadPreset(const juce::File&,juce::String&,int voice=0);
 static juce::StringArray dx7VoiceNames(const juce::File&,juce::String&);
 juce::String displayName()const;
 void requestPanic(){panic.store(true);}
 juce::AudioProcessorValueTreeState parameters;
 juce::MidiKeyboardState keyboard;
 std::array<std::atomic<float>,512> scope{};std::atomic<unsigned> scopeWrite{0};std::atomic<float> peak{0};
 std::atomic<int> currentProgram{0};
private:
 static juce::AudioProcessorValueTreeState::ParameterLayout layout();
 sm_context *dsp=nullptr;std::vector<std::atomic<float>*> values;std::vector<juce::RangedAudioParameter*> hostParameters;std::vector<float> applied;
 std::atomic<int> pendingProgram{-1};std::atomic<bool> panic{false};std::atomic<unsigned> programGeneration{0};
 double rate=44100.,phase=0.;float history[4][2]{};bool primed=false;
 int midiBank[16]{};std::atomic<int> patchKind{0},patchVersion{0};std::array<std::atomic<uint8_t>,128> patchBytes{};int appliedPatch=0;
 std::atomic<int> midiProgram{-1};
 void timerCallback() override;
 void handleMidi(const juce::MidiMessage&);void pullSample(float*);void queuePatch(int,const uint8_t*);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthMavenProcessor)
};
