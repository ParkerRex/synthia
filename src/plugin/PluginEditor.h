#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"

#include <array>
#include <memory>
#include <optional>
#include <vector>

// Modern Sylenth-style control surface for sylenth-ai.
//
// The editor is a dense, grouped-module shell rather than a long scrolling form:
//   * a header with preset navigation, an output meter, live diagnostics, and panic;
//   * a persistent Layer A/B selector that exposes the live layer.* mix state;
//   * a SOUND / MOD / FX tabbed workspace whose panels bind to real APVTS parameters.
//
// Each module panel carries a functional-zone header tick (source / shaping / performance /
// modulation / utility) so the surface reads as a rack of grouped modules. The SOUND page
// leads with the synthesis hero (oscillator slots, tone source, filter/envelopes/LFO,
// performance modules, arp/step/chord) and keeps the preset browser and MIDI utility panels
// at the bottom, matching the Sylenth everything-visible grid with minimal scrolling.
//
// Honesty boundary: the legacy flat osc.* path remains the Layer A Osc 1 compatibility
// source. It is titled "Osc A1 Tone" and badged LIVE so its A1 relationship is explicit.
// Additional layer.N.osc.M.* slots render through the same oscillator stack foundation,
// while deeper Sylenth parity such as per-layer filters, copy/paste, previews, and full A1
// field migration is still future work.
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
    class LayoutSection;
    class ParameterControl;
    class Panel;
    class OscillatorPanel;
    class FilterPanel;
    class EnvelopePanel;
    class LcdDisplay;
    class MixerPanel;
    class PresetWorkflowPanel;
    class PresetMetadataPanel;
    class SequencerPanel;
    class PresetBrowserPanel;
    class MidiControllerPanel;
    class ModulationOverviewPanel;
    class Meter;

    enum class Page
    {
        Sound,
        Mod,
        Fx,
        Browser
    };

    struct RowItem
    {
        LayoutSection* section = nullptr;
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
    void loadPresetAtIndex(int itemIndex);
    void togglePresetFavoriteAtIndex(int itemIndex);
    void loadSelectedPreset();
    void stepPreset(int direction);
    void savePresetAs();
    void duplicatePreset();
    void initializePreset();
    void resetPreset();
    void randomizePreset();
    void captureCompareSlot(int slotIndex);
    void recallCompareSlot(int slotIndex);
    void refreshPresetWorkflow();
    void updateLcdPreset();
    const SynthAudioProcessor::PresetListItem* findCurrentPresetItem();
    void syncPresetMetadataPanel();
    void savePresetWithMetadata(synth::PresetWriteMode mode);

    void updateStatus(const juce::String& message);
    void updateDiagnostics();
    void timerCallback() override;

    Panel* addPanel(juce::Component& page,
                    std::vector<std::unique_ptr<Panel>>& store,
                    juce::String title,
                    std::vector<std::string> ids,
                    juce::String badge = {},
                    juce::Colour badgeColour = {},
                    juce::String stripPrefix = {},
                    juce::String enabledParamId = {});

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
    juce::Label dirtyStatePill;
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
    juce::TextButton browserTab { "Browser" };
    juce::Viewport pageViewport;
    juce::Component soundPage;
    juce::Component modPage;
    juce::Component fxPage;
    juce::Component browserPage;
    std::vector<std::unique_ptr<Panel>> soundPanels;
    std::array<std::unique_ptr<OscillatorPanel>, synth::oscillatorSlotsPerLayer> slotPanels;
    std::unique_ptr<LcdDisplay> lcdDisplay;
    std::unique_ptr<MixerPanel> mixerPanel;
    std::unique_ptr<PresetWorkflowPanel> presetWorkflowPanel;
    std::unique_ptr<PresetMetadataPanel> presetMetadataPanel;
    std::unique_ptr<PresetBrowserPanel> presetBrowserPanel;
    std::unique_ptr<MidiControllerPanel> midiControllerPanel;
    std::unique_ptr<SequencerPanel> sequencerPanel;
    std::unique_ptr<ModulationOverviewPanel> modulationOverviewPanel;
    std::vector<std::unique_ptr<Panel>> modPanels;
    std::vector<std::unique_ptr<Panel>> fxPanels;

    // Named handles into the panel stores so page layout reads clearly.
    Panel* voicePanel = nullptr;
    Panel* coreOscPanel = nullptr;
    std::unique_ptr<FilterPanel> filterPanel;
    std::unique_ptr<EnvelopePanel> ampEnvPanel;
    std::unique_ptr<EnvelopePanel> modEnvPanel;
    Panel* lfoPanel = nullptr;
    Panel* rampPanel = nullptr;
    Panel* ampPanel = nullptr;
    Panel* macroPanel = nullptr;
    Panel* oscPitchModPanel = nullptr;
    Panel* pulseWidthModPanel = nullptr;
    Panel* filterCutoffModPanel = nullptr;
    std::array<Panel*, synth::transModSlotCount> transModPanels {};
    Panel* saturationPanel = nullptr;
    Panel* phaserPanel = nullptr;
    Panel* chorusPanel = nullptr;
    Panel* eqPanel = nullptr;
    Panel* delayPanel = nullptr;
    Panel* reverbPanel = nullptr;
    Panel* compressorPanel = nullptr;
    Panel* masterFxPanel = nullptr;

    Page currentPage = Page::Sound;
    int selectedLayer = 0;

    std::vector<SynthAudioProcessor::PresetListItem> presetItems;
    std::optional<SynthAudioProcessor::PresetListItem> currentPresetMetadataItem;
    std::unique_ptr<juce::FileChooser> fileChooser;
    int lastInvalidSamples = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthAudioProcessorEditor)
};
