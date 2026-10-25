/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include "PluginProcessor.h"
class SynthMavenEditor final : public juce::AudioProcessorEditor,private juce::Timer,private juce::ListBoxModel {
public:
 explicit SynthMavenEditor(SynthMavenProcessor&);~SynthMavenEditor()override;
 void paint(juce::Graphics&)override;void resized()override;
private:
 struct Style;struct Control;struct Panel;
 SynthMavenProcessor& processor;std::unique_ptr<Style> style;
 juce::TextEditor search;juce::ComboBox engineFilter;juce::ListBox browser;
 juce::TextButton previous{"<"},next{">"},save{"SAVE"},import{"IMPORT"},panic{"PANIC"};
 juce::TextButton soundTab{"SOUND"},modTab{"MODULATION"},fxTab{"EFFECTS"},allTab{"ALL PARAMETERS"};
 juce::Viewport viewport;std::unique_ptr<Panel> panel;
 juce::MidiKeyboardComponent keyboard;
 std::unique_ptr<juce::Drawable> logo;std::unique_ptr<juce::FileChooser> chooser;
 std::vector<int> filtered;int tab=0,lastProgram=-1;juce::String lastQuery;
 juce::Rectangle<int> scopeBounds,headerBounds;
 int getNumRows()override;void paintListBoxItem(int,juce::Graphics&,int,int,bool)override;
 void selectedRowsChanged(int)override;void timerCallback()override;
 void filter();void rebuild();void selectRelative(int);void openPreset(bool);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthMavenEditor)
};
