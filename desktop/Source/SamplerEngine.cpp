/* SPDX-License-Identifier: GPL-3.0-only */
#include "SamplerEngine.h"
#include <cmath>
using namespace juce;
MavenSampler::MavenSampler(){for(auto&b:delayBuffer)b.resize(44100);reverb.setSampleRate(44100);panic();}
void MavenSampler::panic(){reverb.reset();for(auto&b:delayBuffer)std::fill(b.begin(),b.end(),0);delayWrite=0;for(auto&v:voices){v.active=false;v.envelope.reset();}sustain.fill(false);bend.fill(0);expression.fill(1);}
void MavenSampler::setSample(std::shared_ptr<const MavenSample>s){if(s.get()!=sample.get()){panic();sample=std::move(s);}}
void MavenSampler::setSettings(const MavenSampleSettings&s,const std::array<double,128>&t){
 bool geometry=s.start!=settings.start||s.end!=settings.end||s.reverse!=settings.reverse||s.mode!=settings.mode;
 settings=s;tuning=t;if(geometry)panic();
 ADSR::Parameters e(.001f+s.attack*s.attack*4,.001f+s.decay*s.decay*4,s.sustain,.005f+s.release*s.release*8);
 for(auto&v:voices){v.envelope.setSampleRate(44100);v.envelope.setParameters(e);}
 filterCoefficients(s.cutoff);
 Reverb::Parameters fx;fx.roomSize=.65f;fx.damping=.5f;fx.wetLevel=s.reverb*.35f;fx.dryLevel=1;fx.width=1;reverb.setParameters(fx);
 updatePitch();
}
void MavenSampler::filterCoefficients(float position){float cutoff=jlimit(20.f,19845.f,20.f*std::pow(1000.f,jlimit(0.f,1.f,position)));float g=std::tan(MathConstants<float>::pi*cutoff/44100.f);k=2.f-1.95f*settings.resonance;a1=1.f/(1.f+g*(g+k));a2=g*a1;a3=g*a2;}
double MavenSampler::increment(const Voice&v)const {int root=roundToInt(settings.root*127);double semitones=v.note-root+(settings.fine-.5)*2+bend[(size_t)v.channel]*2+tuning[(size_t)v.note]/100.;return std::pow(2.,semitones/12.)*(sample?sample->sampleRate/44100.:1.);}
void MavenSampler::updatePitch(){for(auto&v:voices)if(v.active)v.increment=increment(v);}
void MavenSampler::midi(const MidiMessage&m){int ch=jlimit(0,15,m.getChannel()-1);if(m.isAllSoundOff()){panic();return;}if(m.isPitchWheel()){bend[(size_t)ch]=(m.getPitchWheelValue()-8192)/8192.f;updatePitch();return;}
 if(m.isController()){int cc=m.getControllerNumber(),value=m.getControllerValue();if(cc==11)expression[(size_t)ch]=value/127.f;if(cc==121){bend[(size_t)ch]=0;expression[(size_t)ch]=1;sustain[(size_t)ch]=false;for(auto&v:voices)if(v.channel==ch&&!v.held)v.envelope.noteOff();updatePitch();}if(cc==64){sustain[(size_t)ch]=value>=64;if(value<64)for(auto&v:voices)if(v.active&&v.channel==ch&&!v.held)v.envelope.noteOff();}if(cc==123)for(auto&v:voices)if(v.channel==ch){v.held=false;v.envelope.noteOff();}return;}
 if(m.isNoteOn()&&sample&&sample->audio.getNumSamples()>1){Voice*v=nullptr;for(auto&candidate:voices)if(!candidate.active){v=&candidate;break;}if(!v)v=&voices[(size_t)(nextVoice++%16)];v->envelope.reset();v->active=v->held=true;v->note=m.getNoteNumber();v->channel=ch;v->velocity=1-settings.velocity+settings.velocity*m.getFloatVelocity();v->increment=increment(*v);int length=sample->audio.getNumSamples();double start=jlimit(0.,(double)length-2,settings.start*(length-1.)),end=jlimit(start+1.,(double)length,settings.end*(double)length);v->pos=settings.reverse>=.5f?end-1:start;for(int c=0;c<2;c++)v->ic1[c]=v->ic2[c]=0;v->envelope.noteOn();}
 else if(m.isNoteOff()&&roundToInt(settings.mode*2)!=1)for(auto&v:voices)if(v.active&&v.note==m.getNoteNumber()&&v.channel==ch){v.held=false;if(!sustain[(size_t)ch])v.envelope.noteOff();}
}
int MavenSampler::activeVoices()const{int n=0;for(auto&v:voices)n+=v.active;return n;}
void MavenSampler::render(float*out){out[0]=out[1]=0;if(!sample)return;int length=sample->audio.getNumSamples();if(length<2)return;
 double start=jlimit(0.,(double)length-2,settings.start*(length-1.)),end=jlimit(start+1.,(double)length,settings.end*(double)length);
 double lo=jlimit(start,end-1,settings.loopStart*(double)length),hi=jlimit(lo+1.,end,settings.loopEnd*(double)length);bool loop=roundToInt(settings.mode*2)==2,reverse=settings.reverse>=.5f;
 double fade=jmin((hi-lo)*.5,settings.crossfade*.05*sample->sampleRate);
 auto read=[&](int channel,double position){double x=jlimit(0.,(double)length-1,position);int i=(int)x,j=jmin(length-1,i+1);float f=(float)(x-i);auto*data=sample->audio.getReadPointer(jmin(channel,sample->audio.getNumChannels()-1));return data[i]+f*(data[j]-data[i]);};
 float lfo=(float)std::sin(lfoPhase);lfoPhase+=2*MathConstants<double>::pi*(.05+settings.lfoRate*settings.lfoRate*20)/44100.;if(lfoPhase>2*MathConstants<double>::pi)lfoPhase-=2*MathConstants<double>::pi;double pitchFactor=settings.lfoPitch>0?std::pow(2.,lfo*settings.lfoPitch*2/12.):1.;if(settings.lfoFilter>0&&((filterTick++&31)==0))filterCoefficients(settings.cutoff+lfo*settings.lfoFilter*.5f);
 for(auto&v:voices){if(!v.active)continue;if(v.pos<start||v.pos>=end){v.active=false;continue;}float env=v.envelope.getNextSample()*v.velocity*expression[(size_t)v.channel];if(!v.envelope.isActive()){v.active=false;continue;}
  if(!loop)env*=(float)jlimit(0.,1.,(reverse?(v.pos-start+1):(end-v.pos))/(64.*v.increment));
  for(int c=0;c<2;c++){float x=read(c,v.pos);if(loop&&fade>1){if(!reverse&&v.pos>=hi-fade&&v.pos<hi){float f=(float)((v.pos-hi+fade)/fade);x=x*(1-f)+read(c,lo+v.pos-hi+fade)*f;}else if(reverse&&v.pos<=lo+fade&&v.pos>=lo){float f=(float)((lo+fade-v.pos)/fade);x=x*(1-f)+read(c,hi-(lo+fade-v.pos))*f;}}
   float v3=x-v.ic2[c],v1=a1*v.ic1[c]+a2*v3,v2=v.ic2[c]+a2*v.ic1[c]+a3*v3;v.ic1[c]=2*v1-v.ic1[c];v.ic2[c]=2*v2-v.ic2[c];out[c]+=v2*env*.7f;}
  v.pos+=(reverse?-v.increment:v.increment)*pitchFactor;if(loop){if(!reverse&&v.pos>=hi)v.pos=lo+fade+std::fmod(v.pos-hi,hi-lo-fade);if(reverse&&v.pos<lo)v.pos=hi-fade-1.e-6-std::fmod(lo-v.pos,hi-lo-fade);}
 }
 if(settings.chorus>0||settings.delay>0){float modulation=(float)std::sin(chorusPhase);chorusPhase+=2*MathConstants<double>::pi*.35/44100.;if(chorusPhase>2*MathConstants<double>::pi)chorusPhase-=2*MathConstants<double>::pi;for(int c=0;c<2;c++){auto&buffer=delayBuffer[(size_t)c];float echo=buffer[(size_t)((delayWrite+44100-11025)%44100)];int chorusOffset=660+(int)(130*modulation*(c==0?1:-1));float chorus=buffer[(size_t)((delayWrite+44100-chorusOffset)%44100)];buffer[(size_t)delayWrite]=out[c]+echo*.3f;out[c]+=echo*settings.delay*.5f+chorus*settings.chorus*.35f;}delayWrite=(delayWrite+1)%44100;}
 if(settings.reverb>0)reverb.processStereo(&out[0],&out[1],1);

}
