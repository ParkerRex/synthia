#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"

#include <array>
#include <memory>
#include <vector>

// Modern Sylenth-style control surface for sylenth-ai.
//
// The editor is a fixed dense shell rather than a long scrolling form:
//   * a header with preset navigation, an output meter, live diagnostics, and panic;
//   * a persistent Layer A/B selector that exposes the live layer.* mix state;
//   * a SOUND / MOD / FX tabbed workspace whose panels bind to real APVTS parameters.
//
// Honesty boundary: the legacy flat osc.* path remains the Layer A Osc 1 compatibility
// source and is badged LIVE. Additional layer.N.osc.M.* slots render through the same
// oscillator stack foundation, while deeper Sylenth parity such as per-layer filters,
// copy/paste, previews, and full A1 field migration is still future work.
class SynthAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    explicit SynthAudioProcessorEditor(SynthAudioProcessor&);
    ~SynthAudioProcessorEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    class SynthLookAndFeel;
    class ParameterControl;
    class Panel;
    class Meter;

    enum class Page
    {
        Sound,
        Mod,
        Fx
    };

    struct RowItem
    {
        Panel* panel = nullptr;
        float widthFraction = 1.0f;
    };

    void buildHeader();
    void buildLayerBar();
    void buildPages();
    void setSelectedLayer(int layerIndex);
    void setPage(Page page);

    void layoutHeader(juce::Rectangle<int> area);
    void layoutLayerBar(juce::Rectangle<int> area);
    int layoutRows(juce::Component& page, const std::vector<std::vector<RowItem>>& rows, int width);
    void layoutActivePage();

    void refreshPresetMenu();
    void loadSelectedPreset();
    void stepPreset(int direction);
    void savePresetAs();
    void duplicatePreset();

    void updateStatus(const juce::String& message);
    void updateDiagnostics();
    void timerCallback() override;

    Panel* addPanel(juce::Component& page,
                    std::vector<std::unique_ptr<Panel>>& store,
                    juce::String title,
                    std::vector<std::string> ids,
                    juce::String badge = {},
                    juce::Colour badgeColour = {},
                    juce::String stripPrefix = {});

    SynthAudioProcessor& audioProcessor;
    std::unique_ptr<SynthLookAndFeel> lookAndFeel;

    // Header
    juce::Label titleLabel;
    juce::Label engineTag;
    juce::Label statusLabel;
    juce::ComboBox presetCombo;
    juce::TextEditor nameEditor;
    juce::TextButton prevPresetButton { "<" };
    juce::TextButton nextPresetButton { ">" };
    juce::TextButton loadButton { "Load" };
    juce::TextButton saveButton { "Save As" };
    juce::TextButton duplicateButton { "Duplicate" };
    juce::TextButton panicButton { "Panic" };
    std::unique_ptr<Meter> meter;
    juce::Label voicesValue;
    juce::Label voicesCaption;
    juce::Label costValue;
    juce::Label costCaption;
    juce::Label diagnosticsLabel;

    // Layer bar
    juce::Label layerCaption;
    juce::TextButton layerAButton { "A" };
    juce::TextButton layerBButton { "B" };
    juce::Label layerStatusPill;
    juce::Component layerControlsHost;
    std::vector<std::unique_ptr<ParameterControl>> layerControls;

    // Workspace
    juce::TextButton soundTab { "Sound" };
    juce::TextButton modTab { "Modulation" };
    juce::TextButton fxTab { "Effects" };
    juce::Viewport pageViewport;
    juce::Component soundPage;
    juce::Component modPage;
    juce::Component fxPage;
    std::vector<std::unique_ptr<Panel>> soundPanels;
    std::array<std::unique_ptr<Panel>, synth::oscillatorSlotsPerLayer> slotPanels;
    std::vector<std::unique_ptr<Panel>> modPanels;
    std::vector<std::unique_ptr<Panel>> fxPanels;

    // Named handles into the panel stores so page layout reads clearly.
    Panel* voicePanel = nullptr;
    Panel* coreOscPanel = nullptr;
    Panel* filterPanel = nullptr;
    Panel* ampEnvPanel = nullptr;
    Panel* modEnvPanel = nullptr;
    Panel* lfoPanel = nullptr;
    Panel* rampPanel = nullptr;
    Panel* arpPanel = nullptr;
    Panel* chordPanel = nullptr;
    Panel* ampPanel = nullptr;
    Panel* macroPanel = nullptr;
    Panel* oscPitchModPanel = nullptr;
    Panel* pulseWidthModPanel = nullptr;
    Panel* filterCutoffModPanel = nullptr;
    std::array<Panel*, synth::transModSlotCount> transModPanels {};
    Panel* saturationPanel = nullptr;
    Panel* chorusPanel = nullptr;
    Panel* delayPanel = nullptr;
    Panel* reverbPanel = nullptr;
    Panel* masterFxPanel = nullptr;

    Page currentPage = Page::Sound;
    int selectedLayer = 0;

    std::vector<SynthAudioProcessor::PresetListItem> presetItems;
    std::unique_ptr<juce::FileChooser> fileChooser;
    int lastInvalidSamples = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthAudioProcessorEditor)
};
