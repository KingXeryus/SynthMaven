/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include "PluginProcessor.h"
// Parameter diagrams use normalized values. Filter curves are illustrative,
// not transfer-function measurements; the output trace is actual mixed audio.
struct ParameterGraphic:juce::Component {
 enum Kind{Envelope,Lfo,Filter,Levels,Scope};
 SynthMavenProcessor&p;Kind kind;int layer,base;int cutoff=-1,resonance=-1,drag=-1;
 ParameterGraphic(SynthMavenProcessor&proc,Kind k,int b=1):p(proc),kind(k),layer(proc.selectedLayer.load()),base(b){
  for(int i=118;i<126;i++){auto name=p.parameterLabel(i,layer);if(name=="CUT")cutoff=i;if(name=="RES")resonance=i;}
  setMouseCursor(juce::MouseCursor::CrosshairCursor);
 }
 float value(int i)const{auto*v=p.parameters.getRawParameterValue(p.parameterId(i,layer));return v?v->load():0;}
 void write(int i,float v){if(auto*param=p.parameters.getParameter(p.parameterId(i,layer))){param->setValueNotifyingHost(juce::jlimit(0.f,1.f,v));}}
 juce::Rectangle<float>plot()const{return getLocalBounds().toFloat().reduced(16,28).withTrimmedBottom(4);}
 juce::Point<float>point(float x,float y)const{auto r=plot();return {r.getX()+x*r.getWidth(),r.getBottom()-y*r.getHeight()};}
 std::array<juce::Point<float>,3>handles()const{float a=value(base),d=value(base+1),s=value(base+2),r=value(base+3);return {point(.04f+.25f*a,1),point(.09f+.25f*a+.2f*d,s),point(.75f+.24f*r,0)};}
 void paint(juce::Graphics&g)override{
  using namespace juce;auto bounds=getLocalBounds().toFloat();g.setColour(Colour(0xff101923));g.fillRoundedRectangle(bounds,6);auto r=plot();g.setColour(Colour(0xff243442));for(int i=0;i<=8;i++)g.drawVerticalLine(roundToInt(r.getX()+r.getWidth()*i/8),r.getY(),r.getBottom());for(int i=0;i<=4;i++)g.drawHorizontalLine(roundToInt(r.getY()+r.getHeight()*i/4),r.getX(),r.getRight());
  String title=kind==Envelope?"ENVELOPE / NORMALIZED SHAPE":kind==Lfo?"LFO / WAVEFORM PREVIEW":kind==Filter?"FILTER / ILLUSTRATIVE RESPONSE":kind==Levels?"EFFECT SENDS":"LIVE OUTPUT / ALL LAYERS";
  g.setColour(Colour(0xff8394a7));g.setFont(10);g.drawText(title,12,5,getWidth()-24,16,Justification::centredLeft);Path path;
  if(kind==Envelope){auto h=handles();path.startNewSubPath(point(0,0));path.lineTo(h[0]);path.quadraticTo(h[0].x,h[1].y,h[1].x,h[1].y);path.lineTo(point(.72f,value(base+2)));path.lineTo(h[2]);for(int i=0;i<3;i++){g.setColour(Colour(0xff56e5c7));g.fillEllipse(h[i].x-4,h[i].y-4,8,8);}g.setColour(Colour(0xff8394a7));g.drawText("Drag attack, decay/sustain and release handles",12,getHeight()-22,getWidth()-24,16,Justification::centredLeft);}
  else if(kind==Lfo){auto info=sm_parameter_info(p.selectedEngine(layer),base+1);int choice=info.minimum+roundToInt(value(base+1)*(info.maximum-info.minimum));String wave=sm_parameter_choice(p.selectedEngine(layer),base+1,choice);float cycles=1+value(base)*3;for(int n=0;n<=400;n++){float x=n/400.f,t=std::fmod(x*cycles,1.f),y=std::sin(t*MathConstants<float>::twoPi);if(wave.containsIgnoreCase("TRI"))y=1-4*std::abs(t-.5f);else if(wave.containsIgnoreCase("SQR")||wave.containsIgnoreCase("SQU"))y=t<.5f?1:-1;else if(wave.containsIgnoreCase("SAW"))y=t*2-1;else if(wave.containsIgnoreCase("RND")||wave.containsIgnoreCase("S&H"))y=std::sin(std::floor(x*cycles*8)*73.23f);auto q=point(x,.5f+y*.44f);if(n==0)path.startNewSubPath(q);else path.lineTo(q);}g.setColour(Colour(0xff8394a7));g.drawText(wave+" / rate "+p.parameterText(base,value(base),layer),12,getHeight()-22,getWidth()-24,16,Justification::centredLeft);}
  else if(kind==Filter){if(cutoff<0){g.setColour(Colour(0xff8394a7));g.drawText("This engine has no exposed cutoff / resonance pair",r,Justification::centred);return;}String model=p.parameterText(46,value(46),layer);float c=value(cutoff),res=resonance<0?0:value(resonance);for(int n=0;n<=400;n++){float x=n/400.f,y=.78f/(1+std::pow(10.f,(x-c)*6))+.18f*res*std::exp(-std::pow((x-c)*20,2));if(model=="HP")y=.78f-y;else if(model=="BP")y=.8f*std::exp(-std::pow((x-c)*(5+res*12),2));else if(model.containsIgnoreCase("NOT"))y=.8f-.75f*std::exp(-std::pow((x-c)*(5+res*12),2));auto q=point(x,juce::jlimit(0.f,1.f,y));if(n==0)path.startNewSubPath(q);else path.lineTo(q);}g.setColour(Colour(0xff8394a7));g.drawText("Drag: cutoff left/right, resonance up/down / low → high frequency",12,getHeight()-22,getWidth()-24,16,Justification::centredLeft);}
  else if(kind==Levels){const char*names[]={"CHORUS","DELAY","REVERB"};for(int i=0;i<3;i++){auto bar=r.withX(r.getX()+i*r.getWidth()/3+8).withWidth(r.getWidth()/3-16);g.setColour(Colour(0xff263b42));g.fillRect(bar);g.setColour(Colour(0xff56e5c7));g.fillRect(bar.removeFromBottom(bar.getHeight()*value(34+i)));g.setColour(Colours::white);g.drawText(names[i],r.getX()+i*r.getWidth()/3,r.getY(),r.getWidth()/3,18,Justification::centred);}return;}
  else {unsigned head=p.scopeWrite.load();for(unsigned n=0;n<512;n++){auto q=point(n/511.f,.5f+juce::jlimit(-1.f,1.f,p.scope[(head+n)%512].load())*.46f);if(n==0)path.startNewSubPath(q);else path.lineTo(q);}}
  g.setColour(Colour(0xff56e5c7));g.strokePath(path,PathStrokeType(2));
 }
 void mouseDown(const juce::MouseEvent&e)override{drag=-1;if(kind==Envelope){auto h=handles();float distance=30;for(int i=0;i<3;i++){float d=h[i].getDistanceFrom(e.position);if(d<distance){distance=d;drag=i;}}}else if(kind==Filter&&cutoff>=0)drag=0;else if(kind==Levels)drag=juce::jlimit(0,2,int((e.x-plot().getX())/(plot().getWidth()/3)));for(int i:editing())if(auto*q=p.parameters.getParameter(p.parameterId(i,layer)))q->beginChangeGesture();mouseDrag(e);}
 std::vector<int>editing()const{if(drag<0)return {};if(kind==Envelope)return drag==0?std::vector<int>{base}:drag==1?std::vector<int>{base+1,base+2}:std::vector<int>{base+3};if(kind==Filter)return resonance>=0?std::vector<int>{cutoff,resonance}:std::vector<int>{cutoff};if(kind==Levels)return {34+drag};return {};}
 void mouseDrag(const juce::MouseEvent&e)override{if(drag<0)return;auto r=plot();float x=(e.x-r.getX())/r.getWidth(),y=1-(e.y-r.getY())/r.getHeight();if(kind==Envelope){if(drag==0)write(base,(x-.04f)/.25f);if(drag==1){write(base+1,(x-.09f-.25f*value(base))/.2f);write(base+2,y);}if(drag==2)write(base+3,(x-.75f)/.24f);}else if(kind==Filter){write(cutoff,x);if(resonance>=0)write(resonance,y);}else if(kind==Levels)write(34+drag,y);repaint();}
 void mouseUp(const juce::MouseEvent&)override{for(int i:editing())if(auto*q=p.parameters.getParameter(p.parameterId(i,layer)))q->endChangeGesture();drag=-1;}
};
