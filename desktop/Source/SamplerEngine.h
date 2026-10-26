/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <array>
struct MavenSample {
 juce::String name; juce::AudioBuffer<float> audio; double sampleRate=44100.;
 juce::MemoryBlock encoded; // Portable 24-bit WAV, embedded in DAW/preset state.
};
struct MavenSampleSettings {
 float start=0,end=1,loopStart=0,loopEnd=1,root=60.f/127.f,fine=.5f;
 float attack=.035f,decay=.2f,sustain=1,release=.1f,cutoff=1,resonance=0,velocity=1;
 float lfoRate=.2f,lfoPitch=0,lfoFilter=0;
 float mode=0,reverse=0,crossfade=.1f,chorus=0,delay=0,reverb=0;
};
class MavenSampler {
public:
 MavenSampler();
 void setSample(std::shared_ptr<const MavenSample>);
 void setSettings(const MavenSampleSettings&,const std::array<double,128>&);
 void midi(const juce::MidiMessage&); void render(float* stereo);void panic();
 int activeVoices()const;
private:
 struct Voice {bool active=false,held=false;int note=0,channel=0;double pos=0,increment=1;float velocity=1;juce::ADSR envelope;float ic1[2]{},ic2[2]{};};
 std::array<Voice,16> voices;std::array<bool,16>sustain{};std::array<float,16>bend{},expression{};
 std::array<double,128>tuning{};std::shared_ptr<const MavenSample>sample;MavenSampleSettings settings;
 juce::Reverb reverb;std::array<std::vector<float>,2>delayBuffer;int delayWrite=0;double chorusPhase=0,lfoPhase=0;int filterTick=0;void filterCoefficients(float);
 float a1=1,a2=0,a3=0,k=2;int nextVoice=0;
 double increment(const Voice&)const;void updatePitch();
};
