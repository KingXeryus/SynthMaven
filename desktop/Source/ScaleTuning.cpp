/* SPDX-License-Identifier: GPL-3.0-only */
#include "ScaleTuning.h"
#include <cmath>
#include <cstdlib>
using namespace juce;
bool ScalaScale::parse(const String&text,ScalaScale&out,String&error){
 if(text.length()>262144){error="Scale file exceeds 256 KB.";return false;}
 StringArray lines;for(auto line:StringArray::fromLines(text))if(!line.trimStart().startsWithChar('!'))lines.add(line.trim());
 if(lines.size()<2){error="Missing Scala description or note count.";return false;}
 auto countText=lines[1].upToFirstOccurrenceOf("!",false,false).trim();if(countText.isEmpty()||!countText.containsOnly("0123456789")){error="Invalid note count.";return false;}
 int count=countText.getIntValue();if(count<1||count>4096){error="SynthMaven supports 1–4096 degrees per period.";return false;}
 ScalaScale scale;scale.description=lines[0].isEmpty()?"Untitled Scala scale":lines[0];scale.degrees.push_back(0);
 int read=0;for(int i=2;i<lines.size()&&read<count;i++){auto line=lines[i];if(line.isEmpty())continue;auto token=line.upToFirstOccurrenceOf("!",false,false).trim().upToFirstOccurrenceOf(" ",false,false).upToFirstOccurrenceOf("\t",false,false);auto s=token.toStdString();char*end=nullptr;double value=std::strtod(s.c_str(),&end);bool ok=end!=s.c_str()&&std::isfinite(value);
 if(token.containsChar('.')){ok=ok&&*end=='\0';}else{if(end&&*end=='/'){char*last=nullptr;double denominator=std::strtod(end+1,&last);ok=ok&&last!=end+1&&*last=='\0'&&denominator>0&&std::isfinite(denominator);value/=denominator;}else ok=ok&&*end=='\0';ok=ok&&value>0;value=1200*std::log2(value);}
 if(!ok||!std::isfinite(value)){error="Invalid pitch on degree "+String(read+1)+".";return false;}scale.degrees.push_back(value);read++;}
 if(read!=count){error="Scala degree count does not match its data.";return false;}
 scale.period=scale.degrees.back();scale.degrees.pop_back();if(scale.period<=0||scale.period>12000){error="Scale period must be above 0 and at most 12000 cents.";return false;}
 for(int note=-12;note<=127;note++)if(std::abs(scale.offset(note))>10000){error="This scale exceeds the engine's ±10000-cent tuning range.";return false;}
 out=std::move(scale);error.clear();return true;
}
double ScalaScale::offset(int midiNote)const{int n=(int)degrees.size(),d=midiNote-60,q=d/n,r=d%n;if(r<0){r+=n;--q;}return q*period+degrees[(size_t)r]-d*100.;}
