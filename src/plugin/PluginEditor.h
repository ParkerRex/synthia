#pragma once

#include <JuceHeader.h>

class SynthAudioProcessor;

class SynthAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit SynthAudioProcessorEditor(SynthAudioProcessor&);
    ~SynthAudioProcessorEditor() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    SynthAudioProcessor& audioProcessor;
    juce::Label titleLabel;
    juce::Label statusLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthAudioProcessorEditor)
};
