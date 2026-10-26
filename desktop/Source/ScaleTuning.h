/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <juce_core/juce_core.h>
#include <array>
struct ScalaScale {
 juce::String description;std::vector<double> degrees;double period=1200;
 static bool parse(const juce::String&,ScalaScale&,juce::String&);
 double offset(int midiNote)const;
};
