#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"

#include <memory>
#include <vector>

class SynthAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    explicit SynthAudioProcessorEditor(SynthAudioProcessor&);
    ~SynthAudioProcessorEditor() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    class ParameterControl;
    class SectionComponent;
    class TransModSlotComponent;
    class TransModSectionComponent;

    void buildSections();
    void refreshPresetMenu();
    void loadSelectedPreset();
    void savePresetAs();
    void duplicatePreset();
    void updateStatus(const juce::String& message);
    void updateDiagnostics();
    void timerCallback() override;

    SynthAudioProcessor& audioProcessor;
    juce::Label titleLabel;
    juce::Label statusLabel;
    juce::Label presetNameLabel;
    juce::ComboBox presetCombo;
    juce::TextEditor presetNameEditor;
    juce::TextButton loadPresetButton { "Load" };
    juce::TextButton savePresetButton { "Save As" };
    juce::TextButton duplicatePresetButton { "Duplicate" };
    juce::TextButton panicButton { "Panic" };
    juce::Label diagnosticsLabel;
    juce::Viewport viewport;
    juce::Component content;
    std::vector<SynthAudioProcessor::PresetListItem> presetItems;
    std::vector<std::unique_ptr<SectionComponent>> sections;
    std::unique_ptr<TransModSectionComponent> transModSection;
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthAudioProcessorEditor)
};
