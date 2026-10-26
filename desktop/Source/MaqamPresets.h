/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <juce_core/juce_core.h>
// Independently encoded scale sketches from traditional note spellings.
// Reference: https://maqamworld.com/en/maqam.php . No images/audio bundled.
// Cents are editable approximations, not complete maqam sayr or fixed intonation.
struct MaqamPreset {
 const char*name;const char*family;const char*aliases;std::vector<float>ascending,descending;const char*note;
 static const std::vector<MaqamPreset>&all(){static const std::vector<MaqamPreset>data={
 {"Ajam","Ajam","Ajam Ushayran",{0,200,400,500,700,900,1100},{0,200,400,500,700,900,1000},"Major-type ascending; lowered seventh descending."},
 {"Athar Kurd","Kurd","",{0,100,300,600,700,800,1100},{},"Quarter-tone sketch; flexible traditional intonation."},
 {"Awj Iraq","Sikah","",{0,150,350,450,750,850,1150},{},"Octave-led descending melodic behavior is not encoded by a static scale."},
 {"Bastanikar","Sikah","Bastah Nikar",{0,150,350,500,650,750,1050,1150},{},"Extended/non-octave behavior reduced to an editable octave sketch."},
 {"Bayati","Bayati","Muhayyar; Bayat",{0,150,300,500,700,800,1000},{0,150,300,500,700,850,1000},"Upper-jins alternatives selected manually; no automatic melodic direction detection."},
 {"Bayati Shuri","Bayati","Shuri",{0,150,300,500,600,900,1000},{},"Bayati lower jins with Hijaz upper jins."},
 {"Dalanshin","Rast","",{0,200,350,500,700,900,1050},{},"Rast foundation; special upper-register motion is not encoded."},
 {"Hijaz","Hijaz","",{0,100,400,500,700,800,1000},{0,100,400,500,700,850,1000},"Upper-jins alternatives; editable equal/quarter-tone approximation."},
 {"Hijazkar","Hijaz","Shadd Araban; Shahnaz; Suzidil",{0,100,400,500,700,800,1100},{},"Aliases grouped here; their melodic behavior may differ."},
 {"Hijazkar Kurd","Kurd","",{0,100,300,500,700,800,1000},{},"Kurd first octave; Hijazkar upper-octave behavior is not encoded."},
 {"Husayni","Bayati","Husseini",{0,150,300,500,700,850,1000},{0,150,300,500,700,800,1000},"Upper sixth has alternatives; the melodic emphasis is not modeled."},
 {"Huzam","Sikah","Rahat al-Arwah; Houzam",{0,150,350,450,750,850,1050},{},"Relative intervals anchored to the selected equal-tempered root."},
 {"Iraq","Sikah","",{0,150,350,500,650,850,1050},{},"Relative intervals anchored to the selected root."},
 {"Jiharkah","Independent","Chahargah",{0,200,386,484,700,900,1050},{},"Approximate flexible third/fourth; adjust by ear for performance tradition."},
 {"Kirdan","Rast","Sazkar",{0,200,350,500,700,900,1050},{},"Shares Rast pitch material; upper-register sayr and ornaments differ."},
 {"Kurd","Kurd","Kurdī",{0,100,300,500,700,800,1000},{},"Phrygian-type octave sketch."},
 {"Lami","Independent","",{0,100,300,500,600,800,1000},{},"Editable octave sketch."},
 {"Mahur","Rast","",{0,200,350,500,700,900,1100},{},"Rast lower jins with natural seventh."},
 {"Mustaar","Sikah","Mustar",{0,250,350,550,650,850,1050},{},"Relative intervals anchored to the selected root."},
 {"Nahawand","Nahawand","Farahfaza; Buselik",{0,200,300,500,700,800,1100},{0,200,300,500,700,800,1000},"Harmonic-type ascending; natural-minor-type descending."},
 {"Nahawand Murassaa","Nahawand","Nahawand Murassa",{0,200,300,500,600,900,1000},{},"Altered upper jins; editable octave sketch."},
 {"Nairuz","Rast","Yakah; Nayruz",{0,200,350,500,700,850,1000},{},"Rast lower jins with Bayati-type upper pitch material."},
 {"Nawa Athar","Nikriz","Nawa Ather",{0,200,300,600,700,800,1100},{},"Editable octave sketch."},
 {"Nikriz","Nikriz","Nakriz; Neveser",{0,200,300,600,700,900,1000},{},"Editable octave sketch."},
 {"Rast","Rast","",{0,200,350,500,700,900,1050},{0,200,350,500,700,900,1000},"Half-flat third and seventh; descending upper-jins alternative."},
 {"Saba","Independent","Sabah",{0,150,300,400,700,800,1000},{},"Non-octave upper-register behavior is reduced to a repeating octave sketch."},
 {"Saba Zamzam","Independent","Saba Kurdi",{0,100,300,400,700,800,1000},{},"Non-octave upper-register behavior is reduced to a repeating octave sketch."},
 {"Shawq Afza","Ajam","Shawq Afzaa",{0,200,400,500,700,800,1100},{},"Ajam lower jins with Hijaz upper jins."},
 {"Sikah","Sikah","Segah; Segāh",{0,150,350,550,700,850,1050},{},"Relative intervals; the chosen root is an equal-tempered anchor."},
 {"Sikah Baladi","Independent","",{0,200,350,550,700,850,1050},{},"Highly flexible intonation: rough editable sketch only, not an exact reproduction."},
 {"Suzdalara","Rast","Suzidilara",{0,200,350,500,700,900,1000},{},"Rast lower jins; Nahawand upper jins."},
 {"Suznak","Rast","Suzinak",{0,200,350,500,700,800,1100},{},"Rast lower jins; Hijaz upper jins."},
 {"Ushaq Masri","Nahawand","Ushshaq Masri",{0,200,300,500,700,850,1000},{},"Nahawand lower jins with Bayati upper jins."},
 {"Zanjaran","Hijaz","Zangiran",{0,100,400,500,700,900,1000},{},"Hijaz lower jins; Ajam upper jins."}
 };return data;}
 // Choose unique, ascending MIDI key slots with minimal cent error. This handles
 // dense quarter-tone steps without overwriting the root or another degree.
 static std::pair<unsigned,std::array<float,12>>mapping(const std::vector<float>&degrees){
  std::array<float,12>offsets{};unsigned mask=0;int count=(int)degrees.size();if(count<1||count>12)return {4095,offsets};
  double cost[13][13];int prev[13][13];for(auto&r:cost)for(auto&v:r)v=1e30;for(auto&r:prev)for(auto&v:r)v=-1;cost[0][0]=0;
  for(int d=0;d<count;d++)for(int last=0;last<=12;last++)if(cost[d][last]<1e29)for(int key=last;key<=12-count+d;key++){if(d==0&&key!=0)continue;double delta=degrees[(size_t)d]-key*100;double candidate=cost[d][last]+delta*delta-key*.0001;if(candidate<cost[d+1][key+1]){cost[d+1][key+1]=candidate;prev[d+1][key+1]=last;}}
  int end=1;for(int k=2;k<=12;k++)if(cost[count][k]<cost[count][end])end=k;for(int d=count;d>0;d--){int key=end-1;mask|=1u<<key;offsets[(size_t)key]=degrees[(size_t)d-1]-key*100;end=prev[d][end];}return {mask,offsets};
 }
};
