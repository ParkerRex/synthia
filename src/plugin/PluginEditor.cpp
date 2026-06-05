#include "PluginEditor.h"

#include <algorithm>
#include <cctype>

namespace
{
const auto background = juce::Colour::fromRGB(18, 20, 22);
const auto panel = juce::Colour::fromRGB(28, 32, 36);
const auto panelStroke = juce::Colour::fromRGB(62, 70, 76);
const auto text = juce::Colour::fromRGB(238, 240, 242);
const auto mutedText = juce::Colour::fromRGB(165, 172, 178);
const auto accent = juce::Colour::fromRGB(88, 184, 160);
const auto warning = juce::Colour::fromRGB(232, 150, 78);

juce::Font labelFont(float size = 13.0f)
{
    return juce::Font(juce::FontOptions(size));
}

juce::Font boldFont(float size = 14.0f)
{
    return juce::Font(juce::FontOptions(size, juce::Font::bold));
}

juce::String fileSafeName(juce::String name)
{
    name = name.trim();
    if (name.isEmpty())
        name = "User Preset";

    std::string safe;
    auto pendingDash = false;
    for (const auto character : name.toStdString())
    {
        const auto lower = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        if ((lower >= 'a' && lower <= 'z') || (lower >= '0' && lower <= '9'))
        {
            if (pendingDash && !safe.empty())
                safe.push_back('-');
            safe.push_back(lower);
            pendingDash = false;
        }
        else
        {
            pendingDash = true;
        }
    }

    return safe.empty() ? "user-preset" : juce::String(safe);
}

juce::File presetJsonFile(juce::File file)
{
    if (file == juce::File() || file.getFileExtension().equalsIgnoreCase(".json"))
        return file;

    return file.withFileExtension(".json");
}

void styleButton(juce::Button& button, juce::Colour fill)
{
    button.setColour(juce::TextButton::buttonColourId, fill);
    button.setColour(juce::TextButton::buttonOnColourId, accent.darker(0.2f));
    button.setColour(juce::TextButton::textColourOffId, text);
    button.setColour(juce::TextButton::textColourOnId, text);
}

void styleCombo(juce::ComboBox& combo)
{
    combo.setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(35, 40, 45));
    combo.setColour(juce::ComboBox::outlineColourId, panelStroke);
    combo.setColour(juce::ComboBox::textColourId, text);
    combo.setColour(juce::ComboBox::arrowColourId, accent);
}

void styleTextEditor(juce::TextEditor& editor)
{
    editor.setColour(juce::TextEditor::backgroundColourId, juce::Colour::fromRGB(35, 40, 45));
    editor.setColour(juce::TextEditor::outlineColourId, panelStroke);
    editor.setColour(juce::TextEditor::focusedOutlineColourId, accent);
    editor.setColour(juce::TextEditor::textColourId, text);
}
} // namespace

class SynthAudioProcessorEditor::ParameterControl final : public juce::Component
{
public:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    ParameterControl(juce::AudioProcessorValueTreeState& state, const synth::ParameterSpec& parameterSpec)
        : spec(parameterSpec)
    {
        nameLabel.setText(spec.name, juce::dontSendNotification);
        nameLabel.setJustificationType(juce::Justification::centredLeft);
        nameLabel.setColour(juce::Label::textColourId, text);
        nameLabel.setFont(labelFont());
        addAndMakeVisible(nameLabel);

        if (spec.kind == synth::ParameterKind::Choice)
        {
            for (int i = 0; i < static_cast<int>(spec.choices.size()); ++i)
                combo.addItem(juce::String(spec.choices[static_cast<std::size_t>(i)]), i + 1);

            styleCombo(combo);
            comboAttachment = std::make_unique<ComboBoxAttachment>(state, spec.id, combo);
            addAndMakeVisible(combo);
        }
        else if (spec.kind == synth::ParameterKind::Bool)
        {
            toggle.setButtonText({});
            toggle.setColour(juce::ToggleButton::tickColourId, accent);
            toggle.setColour(juce::ToggleButton::tickDisabledColourId, panelStroke);
            buttonAttachment = std::make_unique<ButtonAttachment>(state, spec.id, toggle);
            addAndMakeVisible(toggle);
        }
        else
        {
            slider.setSliderStyle(juce::Slider::LinearHorizontal);
            slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 68, 20);
            slider.setColour(juce::Slider::trackColourId, accent);
            slider.setColour(juce::Slider::backgroundColourId, juce::Colour::fromRGB(42, 46, 50));
            slider.setColour(juce::Slider::thumbColourId, text);
            slider.setColour(juce::Slider::textBoxTextColourId, text);
            slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB(25, 28, 31));
            slider.setColour(juce::Slider::textBoxOutlineColourId, panelStroke);
            sliderAttachment = std::make_unique<SliderAttachment>(state, spec.id, slider);
            addAndMakeVisible(slider);
        }
    }

    void resized() override
    {
        auto area = getLocalBounds();
        auto labelArea = area.removeFromLeft(142);
        nameLabel.setBounds(labelArea.reduced(0, 3));

        if (spec.kind == synth::ParameterKind::Choice)
            combo.setBounds(area.reduced(0, 4));
        else if (spec.kind == synth::ParameterKind::Bool)
            toggle.setBounds(area.removeFromLeft(32).reduced(3));
        else
            slider.setBounds(area.reduced(0, 3));
    }

private:
    synth::ParameterSpec spec;
    juce::Label nameLabel;
    juce::Slider slider;
    juce::ComboBox combo;
    juce::ToggleButton toggle;
    std::unique_ptr<SliderAttachment> sliderAttachment;
    std::unique_ptr<ComboBoxAttachment> comboAttachment;
    std::unique_ptr<ButtonAttachment> buttonAttachment;
};

class SynthAudioProcessorEditor::SectionComponent final : public juce::Component
{
public:
    SectionComponent(juce::AudioProcessorValueTreeState& state,
                     juce::String sectionTitle,
                     std::initializer_list<const char*> parameterIds)
        : title(std::move(sectionTitle))
    {
        for (const auto* id : parameterIds)
        {
            if (const auto* spec = synth::findParameterSpec(id))
            {
                auto control = std::make_unique<ParameterControl>(state, *spec);
                addAndMakeVisible(*control);
                controls.push_back(std::move(control));
            }
        }
    }

    int preferredHeight() const
    {
        const auto rows = static_cast<int>((controls.size() + 1) / 2);
        return 40 + rows * 34 + 12;
    }

    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced(0.5f);
        g.setColour(panel);
        g.fillRoundedRectangle(bounds, 6.0f);
        g.setColour(panelStroke);
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
        g.setColour(text);
        g.setFont(boldFont());
        auto titleArea = getLocalBounds().reduced(14, 8).removeFromTop(24);
        g.drawText(title, titleArea, juce::Justification::centredLeft, true);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(14, 10);
        area.removeFromTop(30);
        const auto gap = 12;
        const auto rowHeight = 32;
        const auto columnWidth = (area.getWidth() - gap) / 2;

        for (int i = 0; i < static_cast<int>(controls.size()); ++i)
        {
            const auto column = i % 2;
            const auto row = i / 2;
            auto cell = juce::Rectangle<int>(area.getX() + column * (columnWidth + gap),
                                             area.getY() + row * rowHeight,
                                             columnWidth,
                                             rowHeight);
            controls[static_cast<std::size_t>(i)]->setBounds(cell);
        }
    }

private:
    juce::String title;
    std::vector<std::unique_ptr<ParameterControl>> controls;
};

class SynthAudioProcessorEditor::TransModSlotComponent final : public juce::Component
{
public:
    TransModSlotComponent(juce::AudioProcessorValueTreeState& state, int slotNumber)
        : slot(slotNumber)
    {
        title.setText("Slot " + juce::String(slot), juce::dontSendNotification);
        title.setColour(juce::Label::textColourId, text);
        title.setFont(boldFont(13.0f));
        addAndMakeVisible(title);

        const auto prefix = "transmod." + std::to_string(slot) + ".";
        addControl(state, prefix + "enabled");
        addControl(state, prefix + "source");
        addControl(state, prefix + "scaler");
        addControl(state, prefix + "filter_cutoff_semitones");
        addControl(state, prefix + "osc_pitch_semitones");
        addControl(state, prefix + "pulse_width");
        addControl(state, prefix + "amp_level_db");
        addControl(state, prefix + "pan");
    }

    int preferredHeight() const
    {
        return 32 + static_cast<int>(controls.size()) * 30 + 10;
    }

    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced(0.5f);
        g.setColour(juce::Colour::fromRGB(23, 27, 31));
        g.fillRoundedRectangle(bounds, 5.0f);
        g.setColour(panelStroke);
        g.drawRoundedRectangle(bounds, 5.0f, 1.0f);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(10, 8);
        title.setBounds(area.removeFromTop(24));
        for (auto& control : controls)
            control->setBounds(area.removeFromTop(30));
    }

private:
    void addControl(juce::AudioProcessorValueTreeState& state, const std::string& id)
    {
        if (const auto* spec = synth::findParameterSpec(id))
        {
            auto control = std::make_unique<ParameterControl>(state, *spec);
            addAndMakeVisible(*control);
            controls.push_back(std::move(control));
        }
    }

    int slot = 1;
    juce::Label title;
    std::vector<std::unique_ptr<ParameterControl>> controls;
};

class SynthAudioProcessorEditor::TransModSectionComponent final : public juce::Component
{
public:
    explicit TransModSectionComponent(juce::AudioProcessorValueTreeState& state)
    {
        for (int slot = 1; slot <= synth::transModSlotCount; ++slot)
        {
            auto slotComponent = std::make_unique<TransModSlotComponent>(state, slot);
            addAndMakeVisible(*slotComponent);
            slots.push_back(std::move(slotComponent));
        }
    }

    int preferredHeight() const
    {
        return 42 + 2 * 274 + 20;
    }

    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced(0.5f);
        g.setColour(panel);
        g.fillRoundedRectangle(bounds, 6.0f);
        g.setColour(panelStroke);
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
        g.setColour(text);
        g.setFont(boldFont());
        auto titleArea = getLocalBounds().reduced(14, 8).removeFromTop(24);
        g.drawText("TransMod", titleArea, juce::Justification::centredLeft, true);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(14, 10);
        area.removeFromTop(32);
        const auto gap = 10;
        const auto columnWidth = (area.getWidth() - 3 * gap) / 4;
        const auto rowHeight = 274;

        for (int i = 0; i < static_cast<int>(slots.size()); ++i)
        {
            const auto column = i % 4;
            const auto row = i / 4;
            const auto x = area.getX() + column * (columnWidth + gap);
            const auto y = area.getY() + row * (rowHeight + gap);
            slots[static_cast<std::size_t>(i)]->setBounds(x, y, columnWidth, rowHeight);
        }
    }

private:
    std::vector<std::unique_ptr<TransModSlotComponent>> slots;
};

SynthAudioProcessorEditor::SynthAudioProcessorEditor(SynthAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    titleLabel.setText("Synth", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, text);
    titleLabel.setFont(juce::Font(juce::FontOptions(25.0f, juce::Font::bold)));
    addAndMakeVisible(titleLabel);

    statusLabel.setText("Dry core ready", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setColour(juce::Label::textColourId, mutedText);
    statusLabel.setFont(labelFont());
    addAndMakeVisible(statusLabel);

    presetNameLabel.setText("Name", juce::dontSendNotification);
    presetNameLabel.setColour(juce::Label::textColourId, mutedText);
    presetNameLabel.setFont(labelFont(12.0f));
    addAndMakeVisible(presetNameLabel);

    styleCombo(presetCombo);
    addAndMakeVisible(presetCombo);

    presetNameEditor.setText(audioProcessor.getCurrentPresetName());
    styleTextEditor(presetNameEditor);
    addAndMakeVisible(presetNameEditor);

    styleButton(loadPresetButton, accent.darker(0.35f));
    styleButton(savePresetButton, juce::Colour::fromRGB(63, 91, 116));
    styleButton(duplicatePresetButton, juce::Colour::fromRGB(63, 91, 116));
    styleButton(panicButton, warning.darker(0.25f));
    addAndMakeVisible(loadPresetButton);
    addAndMakeVisible(savePresetButton);
    addAndMakeVisible(duplicatePresetButton);
    addAndMakeVisible(panicButton);

    diagnosticsLabel.setJustificationType(juce::Justification::centredLeft);
    diagnosticsLabel.setColour(juce::Label::textColourId, mutedText);
    diagnosticsLabel.setFont(labelFont(12.0f));
    addAndMakeVisible(diagnosticsLabel);

    viewport.setViewedComponent(&content, false);
    viewport.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport);

    loadPresetButton.onClick = [this] { loadSelectedPreset(); };
    savePresetButton.onClick = [this] { savePresetAs(); };
    duplicatePresetButton.onClick = [this] { duplicatePreset(); };
    panicButton.onClick = [this] {
        audioProcessor.requestPanic();
        updateStatus("Panic requested");
    };

    buildSections();
    refreshPresetMenu();
    updateDiagnostics();
    startTimerHz(8);

    setResizable(true, true);
    setResizeLimits(900, 640, 1500, 1100);
    setSize(1180, 820);
}

void SynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(background);
    g.setColour(juce::Colour::fromRGB(10, 12, 14));
    g.fillRect(getLocalBounds().removeFromTop(92));
    g.setColour(panelStroke);
    g.drawRect(getLocalBounds(), 1);
}

void SynthAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    auto header = area.removeFromTop(92).reduced(18, 12);
    auto leftHeader = header.removeFromLeft(340);
    titleLabel.setBounds(leftHeader.removeFromTop(34));
    statusLabel.setBounds(leftHeader.removeFromTop(26));

    auto presetArea = header.removeFromLeft(520);
    presetCombo.setBounds(presetArea.removeFromTop(28));
    presetArea.removeFromTop(6);
    presetNameLabel.setBounds(presetArea.removeFromLeft(42).reduced(0, 2));
    presetNameEditor.setBounds(presetArea.removeFromLeft(210));
    presetArea.removeFromLeft(8);
    loadPresetButton.setBounds(presetArea.removeFromLeft(66));
    presetArea.removeFromLeft(6);
    savePresetButton.setBounds(presetArea.removeFromLeft(82));
    presetArea.removeFromLeft(6);
    duplicatePresetButton.setBounds(presetArea.removeFromLeft(90));

    auto rightHeader = header;
    panicButton.setBounds(rightHeader.removeFromRight(74).removeFromTop(28));
    diagnosticsLabel.setBounds(rightHeader.reduced(12, 0));

    viewport.setBounds(area);
    const auto contentWidth = std::max(900, viewport.getWidth() - 18);
    auto y = 14;
    const auto gap = 12;
    for (auto& section : sections)
    {
        const auto height = section->preferredHeight();
        section->setBounds(14, y, contentWidth - 28, height);
        y += height + gap;
    }

    if (transModSection != nullptr)
    {
        const auto height = transModSection->preferredHeight();
        transModSection->setBounds(14, y, contentWidth - 28, height);
        y += height + gap;
    }

    content.setSize(contentWidth, y + 14);
}

void SynthAudioProcessorEditor::buildSections()
{
    auto& state = audioProcessor.getValueTreeState();

    auto addSection = [this, &state](juce::String title, std::initializer_list<const char*> ids) {
        auto section = std::make_unique<SectionComponent>(state, std::move(title), ids);
        content.addAndMakeVisible(*section);
        sections.push_back(std::move(section));
    };

    addSection("Voice", {
        "voice.mode", "voice.polyphony", "voice.unison_count", "voice.retrigger",
        "voice.glide_ms", "voice.velocity_glide_ms"
    });

    addSection("Oscillator", {
        "osc.pitch_semitones", "osc.fine_cents", "osc.stack_count", "osc.stack_detune",
        "osc.saw_level", "osc.pulse_level", "osc.pulse_width", "osc.noise_level",
        "osc.sub_wave", "osc.sub_octave", "osc.sub_level", "osc.sync_amount",
        "osc.phase_reset"
    });

    addSection("Filter", {
        "filter.enabled", "filter.mode", "filter.cutoff_semitones", "filter.resonance",
        "filter.drive", "filter.keytrack", "filter.oversampling",
        "direct.filter_lfo_semitones", "direct.filter_mod_env_semitones"
    });

    addSection("Envelopes", {
        "amp_env.attack_ms", "amp_env.decay_ms", "amp_env.sustain", "amp_env.release_ms",
        "mod_env.attack_ms", "mod_env.decay_ms", "mod_env.sustain", "mod_env.release_ms"
    });

    addSection("LFO", {
        "lfo.shape", "lfo.rate_mode", "lfo.rate_hz", "lfo.sync_division",
        "lfo.phase_degrees", "lfo.gate_mode", "lfo.mono", "lfo.swing"
    });

    addSection("Ramp", {
        "ramp.enabled", "ramp.mode", "ramp.delay_ms", "ramp.rise_ms", "ramp.curve"
    });

    addSection("Direct Mod", {
        "direct.filter_keytrack", "direct.osc_keytrack_semitones", "direct.osc_lfo_semitones",
        "direct.osc_mod_env_semitones", "direct.pulse_keytrack", "direct.pulse_lfo",
        "direct.pulse_mod_env"
    });

    addSection("Amp and Stereo", {
        "amp.drive", "amp.level_db", "amp.pan", "amp.pan_spread",
        "amp.unison_spread", "amp.analog"
    });

    addSection("Macros", {
        "macro.motion", "macro.width", "macro.drive", "macro.space"
    });

    addSection("FX", {
        "fx.enabled", "fx.saturation_enabled", "fx.saturation_mix", "fx.saturation_drive",
        "fx.delay_enabled", "fx.delay_mix", "fx.delay_sync_division", "fx.delay_feedback",
        "fx.reverb_enabled", "fx.reverb_mix", "fx.reverb_decay",
        "fx.chorus_enabled", "fx.chorus_mix", "fx.chorus_rate_hz", "fx.chorus_depth_ms"
    });

    addSection("Quality", {
        "quality.realtime_mode", "quality.offline_mode"
    });

    transModSection = std::make_unique<TransModSectionComponent>(state);
    content.addAndMakeVisible(*transModSection);
}

void SynthAudioProcessorEditor::refreshPresetMenu()
{
    const auto previousText = presetCombo.getText();
    presetItems = audioProcessor.getPresetList();
    presetCombo.clear(juce::dontSendNotification);

    for (int i = 0; i < static_cast<int>(presetItems.size()); ++i)
    {
        const auto& item = presetItems[static_cast<std::size_t>(i)];
        presetCombo.addItem((item.factory ? "Factory: " : "User: ") + item.displayName, i + 1);
    }

    if (presetItems.empty())
    {
        presetCombo.setText("No presets found", juce::dontSendNotification);
        return;
    }

    const auto currentName = audioProcessor.getCurrentPresetName();
    for (int i = 0; i < static_cast<int>(presetItems.size()); ++i)
    {
        if (presetItems[static_cast<std::size_t>(i)].displayName == currentName)
        {
            presetCombo.setSelectedItemIndex(i, juce::dontSendNotification);
            return;
        }
    }

    if (previousText.isNotEmpty())
        presetCombo.setText(previousText, juce::dontSendNotification);
    else
        presetCombo.setSelectedItemIndex(0, juce::dontSendNotification);
}

void SynthAudioProcessorEditor::loadSelectedPreset()
{
    const auto index = presetCombo.getSelectedItemIndex();
    if (index < 0 || index >= static_cast<int>(presetItems.size()))
    {
        updateStatus("Choose a preset first");
        return;
    }

    juce::String message;
    const auto& item = presetItems[static_cast<std::size_t>(index)];
    if (audioProcessor.loadPresetFile(item.file, message))
    {
        presetNameEditor.setText(audioProcessor.getCurrentPresetName(), juce::dontSendNotification);
        refreshPresetMenu();
    }

    updateStatus(message);
}

void SynthAudioProcessorEditor::savePresetAs()
{
    const auto presetName = presetNameEditor.getText().trim().isNotEmpty()
        ? presetNameEditor.getText().trim()
        : audioProcessor.getCurrentPresetName();
    const auto suggestedFile = audioProcessor.getUserPresetDirectory()
        .getChildFile(fileSafeName(presetName) + ".json");

    fileChooser = std::make_unique<juce::FileChooser>("Save Synth preset",
                                                       suggestedFile,
                                                       "*.json");
    juce::Component::SafePointer<SynthAudioProcessorEditor> safeEditor { this };
    fileChooser->launchAsync(juce::FileBrowserComponent::saveMode
                             | juce::FileBrowserComponent::canSelectFiles
                             | juce::FileBrowserComponent::warnAboutOverwriting,
                             [safeEditor, presetName](const juce::FileChooser& chooser) {
                                 if (safeEditor == nullptr)
                                     return;

                                 const auto file = presetJsonFile(chooser.getResult());
                                 if (file == juce::File())
                                     return;

                                 juce::String message;
                                 if (safeEditor->audioProcessor.savePresetFile(file, presetName, message))
                                     safeEditor->refreshPresetMenu();

                                 safeEditor->updateStatus(message);
                             });
}

void SynthAudioProcessorEditor::duplicatePreset()
{
    auto presetName = presetNameEditor.getText().trim();
    if (presetName.isEmpty())
        presetName = audioProcessor.getCurrentPresetName();
    if (!presetName.endsWithIgnoreCase(" Copy"))
        presetName << " Copy";

    auto directory = audioProcessor.getUserPresetDirectory();
    auto file = directory.getChildFile(fileSafeName(presetName) + ".json");
    auto suffix = 2;
    while (file.existsAsFile())
        file = directory.getChildFile(fileSafeName(presetName) + "-" + juce::String(suffix++) + ".json");

    juce::String message;
    if (audioProcessor.savePresetFile(file, presetName, message))
    {
        presetNameEditor.setText(presetName, juce::dontSendNotification);
        refreshPresetMenu();
    }

    updateStatus(message);
}

void SynthAudioProcessorEditor::updateStatus(const juce::String& message)
{
    statusLabel.setText(message, juce::dontSendNotification);
}

void SynthAudioProcessorEditor::updateDiagnostics()
{
    const auto snapshot = audioProcessor.getDiagnosticsSnapshot();
    const auto sampleRate = snapshot.sampleRate > 0.0
        ? juce::String(snapshot.sampleRate, 0) + " Hz"
        : "No audio";
    const auto peakDb = snapshot.peak > 0.0f
        ? juce::String(juce::Decibels::gainToDecibels(snapshot.peak), 1) + " dB"
        : "-inf dB";

    diagnosticsLabel.setText("SR " + sampleRate
                             + " | Block " + juce::String(snapshot.blockSize)
                             + " | Voices " + juce::String(snapshot.activeVoices)
                             + " | Peak " + peakDb
                             + " | MIDI " + juce::String(snapshot.midiEvents)
                             + " | Invalid " + juce::String(snapshot.invalidSamples)
                             + " | " + snapshot.architecture,
                             juce::dontSendNotification);
}

void SynthAudioProcessorEditor::timerCallback()
{
    updateDiagnostics();
}
