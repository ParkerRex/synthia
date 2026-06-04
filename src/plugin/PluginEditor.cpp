#include "PluginEditor.h"

#include "PluginProcessor.h"

SynthAudioProcessorEditor::SynthAudioProcessorEditor(SynthAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    juce::ignoreUnused(audioProcessor);

    titleLabel.setText("Synth", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setFont(juce::Font(juce::FontOptions(24.0f, juce::Font::bold)));
    addAndMakeVisible(titleLabel);

    statusLabel.setText("Foundation build: " + juce::String(static_cast<int>(synth::getParameterSpecs().size()))
                            + " parameters, silent engine",
                        juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    statusLabel.setFont(juce::Font(juce::FontOptions(14.0f)));
    addAndMakeVisible(statusLabel);

    setSize(720, 420);
}

void SynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(18, 20, 22));
    g.setColour(juce::Colour::fromRGB(65, 70, 76));
    g.drawRect(getLocalBounds(), 1);
}

void SynthAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(24);
    titleLabel.setBounds(area.removeFromTop(40));
    statusLabel.setBounds(area.removeFromTop(28));
}
