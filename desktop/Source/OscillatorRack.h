/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
// Two additional band-limited oscillators accompany the original engine source.
struct OscillatorRack {
 inline static const char*keys[]={"osc1Level","osc2On","osc2Wave","osc2Octave","osc2Fine","osc2Level","osc2Pan","osc3On","osc3Wave","osc3Octave","osc3Fine","osc3Level","osc3Pan","oscAttack","oscDecay","oscSustain","oscRelease","chorusMix","chorusRate","chorusDepth","flangerMix","flangerRate","flangerDepth","flangerFeedback"};
 inline static const float defaults[]={1,0,0,.5f,.5f,.3f,.5f,0,0,.5f,.5f,.3f,.5f,.02f,.2f,.8f,.2f,0,.2f,.5f,0,.2f,.5f,.5f};
 std::array<float,24>settings{};std::array<double,128>tuning{};
 struct Voice{bool active=false,held=false;int note=60,channel=1,stage=0;float velocity=0,envelope=0;std::array<double,2>phase{};};std::array<Voice,16>voices{};std::array<bool,16>sustain{};std::array<float,16>bend{},expression{};
 struct ModDelay {std::array<std::vector<float>,2>line;int head=0;double phase=0;ModDelay(){for(auto&l:line)l.resize(4096);}void clear(){for(auto&l:line)std::fill(l.begin(),l.end(),0);head=0;phase=0;}
  void process(float*b,float mix,float rate,float depth,float feedback,bool flange){for(int c=0;c<2;c++){double mod=std::sin(phase+c*1.4),delay=flange?44.1+(1+mod)*depth*220:660+(1+mod)*depth*220;double pos=head-delay;while(pos<0)pos+=4096;int a=(int)pos;float t=(float)(pos-a),wet=line[c][a]*(1-t)+line[c][(a+1)%4096]*t;line[c][head]=juce::jlimit(-4.f,4.f,b[c]+wet*feedback);b[c]=b[c]*(1-mix*.5f)+wet*mix*.5f;}head=(head+1)%4096;phase+=juce::MathConstants<double>::twoPi*(.05+rate*rate*4)/44100;if(phase>juce::MathConstants<double>::twoPi)phase-=juce::MathConstants<double>::twoPi;}
 }chorus,flanger;
 OscillatorRack(){std::copy(std::begin(defaults),std::end(defaults),settings.begin());panic();}
 void panic(){for(auto&v:voices)v={};sustain.fill(false);bend.fill(0);expression.fill(1);chorus.clear();flanger.clear();}
 void midi(const juce::MidiMessage&m){int ch=juce::jlimit(0,15,m.getChannel()-1);if(m.isNoteOn()){Voice*v=nullptr;for(auto&x:voices)if(!x.active){v=&x;break;}if(!v)v=&*std::min_element(voices.begin(),voices.end(),[](auto&a,auto&b){return a.envelope<b.envelope;});*v={};v->active=v->held=true;v->note=m.getNoteNumber();v->channel=ch+1;v->velocity=m.getFloatVelocity();}
  else if(m.isNoteOff()){for(auto&v:voices)if(v.active&&v.note==m.getNoteNumber()&&v.channel==ch+1){v.held=false;if(!sustain[ch])v.stage=3;}}
  else if(m.isPitchWheel())bend[ch]=(m.getPitchWheelValue()-8192)/8192.f*2;
  else if(m.isController()){int cc=m.getControllerNumber();if(cc==64){sustain[ch]=m.getControllerValue()>=64;if(!sustain[ch])for(auto&v:voices)if(v.channel==ch+1&&!v.held)v.stage=3;}if(cc==11)expression[ch]=m.getControllerValue()/127.f;if(cc==120||cc==123)for(auto&v:voices)if(v.channel==ch+1){if(cc==120)v.active=false;else {v.held=false;v.stage=3;}}if(cc==121){sustain[ch]=false;bend[ch]=0;expression[ch]=1;for(auto&v:voices)if(v.channel==ch+1&&!v.held)v.stage=3;}}
 }
 static double blep(double t,double dt){if(t<dt){t/=dt;return t+t-t*t-1;}if(t>1-dt){t=(t-1)/dt;return t*t+t+t+1;}return 0;}
 static float wave(double phase,double dt,int shape){double sine=std::sin(phase*juce::MathConstants<double>::twoPi);if(shape==0)return (float)sine;if(shape==1)return (float)(2*phase-1-blep(phase,dt));if(shape==2)return (float)((phase<.5?1:-1)+blep(phase,dt)-blep(std::fmod(phase+.5,1.),dt));return (float)(2/juce::MathConstants<double>::pi*std::asin(sine));}
 void render(float*b){b[0]*=settings[0];b[1]*=settings[0];for(auto&v:voices){if(!v.active)continue;float a=.001f+settings[13]*settings[13]*4,d=.005f+settings[14]*settings[14]*4,r=.005f+settings[16]*settings[16]*6;if(v.stage==0){v.envelope+=1/(a*44100);if(v.envelope>=1){v.envelope=1;v.stage=1;}}else if(v.stage==1){v.envelope-=1/(d*44100);if(v.envelope<=settings[15]){v.envelope=settings[15];v.stage=2;}}else if(v.stage==2)v.envelope=settings[15];else{v.envelope-=1/(r*44100);if(v.envelope<=0){v.active=false;continue;}}
   for(int osc=0;osc<2;osc++){int k=1+osc*6;if(settings[k]<.5f)continue;double pitch=v.note+(juce::roundToInt(settings[k+2]*6)-3)*12+(settings[k+3]-.5)*2+bend[v.channel-1]+tuning[v.note]/100;double frequency=440*std::pow(2.,(pitch-69)/12.);double dt=juce::jlimit(.000001,.45,frequency/44100);float sample=frequency<19845?wave(v.phase[osc],dt,juce::roundToInt(settings[k+1]*3)):0;v.phase[osc]=std::fmod(v.phase[osc]+dt,1.);if(settings[k]<.5f)continue;sample*=v.envelope*v.velocity*expression[v.channel-1]*settings[k+4]*.2f;float pan=settings[k+5];b[0]+=sample*juce::jmin(1.f,2*(1-pan));b[1]+=sample*juce::jmin(1.f,2*pan);}
  }if(settings[17]>.00001f)chorus.process(b,settings[17],settings[18],settings[19],0,false);if(settings[20]>.00001f)flanger.process(b,settings[20],settings[21],settings[22],(settings[23]-.5f)*1.6f,true);
 }
};
