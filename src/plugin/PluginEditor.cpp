#include "PluginEditor.h"

#include <algorithm>
#include <cctype>
#include <cmath>

namespace
{
// ---- Palette ---------------------------------------------------------------
const auto background   = juce::Colour::fromRGB(18, 20, 24);
const auto headerBg     = juce::Colour::fromRGB(11, 13, 16);
const auto panelBg      = juce::Colour::fromRGB(28, 32, 37);
const auto panelHeader  = juce::Colour::fromRGB(33, 38, 44);
const auto fieldBg      = juce::Colour::fromRGB(20, 23, 27);
const auto stroke       = juce::Colour::fromRGB(52, 59, 66);
const auto strokeSoft   = juce::Colour::fromRGB(38, 43, 49);
const auto text         = juce::Colour::fromRGB(236, 239, 242);
const auto mutedText    = juce::Colour::fromRGB(150, 159, 167);
const auto accent       = juce::Colour::fromRGB(88, 196, 176);
const auto live         = juce::Colour::fromRGB(108, 206, 138);
const auto staged       = juce::Colour::fromRGB(220, 158, 86);
const auto warn         = juce::Colour::fromRGB(228, 120, 96);
const auto knobFill     = juce::Colour::fromRGB(40, 45, 51);
const auto knobStroke   = juce::Colour::fromRGB(62, 70, 78);

constexpr float rotaryStart = juce::MathConstants<float>::pi * 1.25f;
constexpr float rotaryEnd   = juce::MathConstants<float>::pi * 2.75f;

// Fixed shell chrome heights (editor body math reads these by name).
constexpr int headerHeight   = 66;
constexpr int layerBarHeight  = 92;
constexpr int tabBarHeight    = 40;
constexpr int footerHeight    = 26;

juce::Font uiFont(float size = 13.0f, bool bold = false)
{
    return juce::Font(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain));
}

juce::String formatValue(const synth::ParameterSpec& spec, double value)
{
    if (std::abs(value) < 1.0e-3)
        value = 0.0; // clean sub-interval denormals (e.g. normalized-default round-trip) to a true zero

    const auto& unit = spec.unit;
    if (unit == "dB")
        return juce::String(value, 1) + " dB";
    if (unit == "Hz")
        return juce::String(value, value < 10.0 ? 2 : 1) + " Hz";
    if (unit == "milliseconds")
        return value >= 1000.0 ? juce::String(value / 1000.0, 2) + " s"
                               : juce::String(value, value < 10.0 ? 1 : 0) + " ms";
    if (unit == "semitones")
        return juce::String(value, 1) + " st";
    if (unit == "cents")
        return juce::String(juce::roundToInt(value)) + " ct";
    if (unit == "percent")
        return juce::String(juce::roundToInt(value * 100.0)) + " %";
    if (unit == "degrees")
        return juce::String(juce::roundToInt(value)) + juce::String::fromUTF8("\xc2\xb0");
    if (unit == "octaves")
    {
        const auto octaves = static_cast<int>(std::round(value));
        return (octaves > 0 ? "+" : "") + juce::String(octaves);
    }
    if (unit == "voices")
        return juce::String(static_cast<int>(std::round(value)));
    return juce::String(value, 2);
}

void styleFlatButton(juce::Button& button, juce::Colour fill, juce::Colour textColour = text)
{
    button.setColour(juce::TextButton::buttonColourId, fill);
    button.setColour(juce::TextButton::buttonOnColourId, accent.darker(0.1f));
    button.setColour(juce::TextButton::textColourOffId, textColour);
    button.setColour(juce::TextButton::textColourOnId, juce::Colour::fromRGB(12, 14, 16));
}

void styleChipCaption(juce::Label& label)
{
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, mutedText);
    label.setFont(uiFont(10.5f));
}

void styleChipValue(juce::Label& label)
{
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, text);
    label.setFont(uiFont(15.0f, true));
}
} // namespace

// ============================================================================
// Look and feel: modern rotary knobs, switch toggles, flat combos and buttons.
// ============================================================================
class SynthAudioProcessorEditor::SynthLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    SynthLookAndFeel()
    {
        setColour(juce::Slider::textBoxTextColourId, text);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

        setColour(juce::ComboBox::backgroundColourId, fieldBg);
        setColour(juce::ComboBox::outlineColourId, stroke);
        setColour(juce::ComboBox::textColourId, text);
        setColour(juce::ComboBox::arrowColourId, accent);

        setColour(juce::PopupMenu::backgroundColourId, juce::Colour::fromRGB(24, 28, 32));
        setColour(juce::PopupMenu::textColourId, text);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, accent.darker(0.2f));
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour::fromRGB(12, 14, 16));

        setColour(juce::TextEditor::backgroundColourId, fieldBg);
        setColour(juce::TextEditor::outlineColourId, stroke);
        setColour(juce::TextEditor::focusedOutlineColourId, accent);
        setColour(juce::TextEditor::textColourId, text);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider&) override
    {
        const auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(3.0f);
        const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centreX = bounds.getCentreX();
        const auto centreY = bounds.getCentreY();
        const auto toAngle = startAngle + sliderPos * (endAngle - startAngle);
        const auto lineWidth = 3.0f;
        const auto arcRadius = radius - lineWidth * 0.5f - 1.0f;

        juce::Path backTrack;
        backTrack.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f,
                                startAngle, endAngle, true);
        g.setColour(juce::Colour::fromRGB(46, 52, 59));
        g.strokePath(backTrack, juce::PathStrokeType(lineWidth, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));

        if (sliderPos > 0.0f)
        {
            juce::Path valueArc;
            valueArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f,
                                   startAngle, toAngle, true);
            g.setColour(accent);
            g.strokePath(valueArc, juce::PathStrokeType(lineWidth, juce::PathStrokeType::curved,
                                                        juce::PathStrokeType::rounded));
        }

        const auto bodyRadius = arcRadius - lineWidth * 0.5f - 3.0f;
        g.setColour(knobFill);
        g.fillEllipse(centreX - bodyRadius, centreY - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f);
        g.setColour(knobStroke);
        g.drawEllipse(centreX - bodyRadius, centreY - bodyRadius, bodyRadius * 2.0f, bodyRadius * 2.0f, 1.0f);

        const juce::Point<float> tip(centreX + (bodyRadius - 2.0f) * std::sin(toAngle),
                                     centreY - (bodyRadius - 2.0f) * std::cos(toAngle));
        const juce::Point<float> root(centreX + (bodyRadius * 0.32f) * std::sin(toAngle),
                                      centreY - (bodyRadius * 0.32f) * std::cos(toAngle));
        g.setColour(text);
        g.drawLine({ root, tip }, 2.0f);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        const auto on = button.getToggleState();
        const auto trackWidth = juce::jmin(40.0f, bounds.getWidth());
        auto track = juce::Rectangle<float>(trackWidth, 20.0f).withCentre(bounds.getCentre());

        g.setColour(on ? accent.withAlpha(0.85f) : juce::Colour::fromRGB(36, 41, 47));
        g.fillRoundedRectangle(track, 10.0f);
        g.setColour(on ? accent : (shouldDrawButtonAsHighlighted ? mutedText : stroke));
        g.drawRoundedRectangle(track, 10.0f, 1.0f);

        const auto knobDiameter = 14.0f;
        const auto knobX = on ? track.getRight() - knobDiameter - 3.0f : track.getX() + 3.0f;
        g.setColour(on ? juce::Colours::white : mutedText);
        g.fillEllipse(knobX, track.getCentreY() - knobDiameter * 0.5f, knobDiameter, knobDiameter);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool,
                      int, int, int, int, juce::ComboBox&) override
    {
        const auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(0.5f);
        g.setColour(fieldBg);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(stroke);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        juce::Path arrow;
        const auto arrowArea = juce::Rectangle<float>(static_cast<float>(width) - 18.0f, 0.0f, 14.0f,
                                                      static_cast<float>(height)).reduced(0.0f, static_cast<float>(height) * 0.42f);
        arrow.startNewSubPath(arrowArea.getX(), arrowArea.getY());
        arrow.lineTo(arrowArea.getCentreX(), arrowArea.getBottom());
        arrow.lineTo(arrowArea.getRight(), arrowArea.getY());
        g.setColour(accent);
        g.strokePath(arrow, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
    }

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds(8, 1, box.getWidth() - 26, box.getHeight() - 2);
        label.setFont(uiFont(12.5f));
    }

    juce::Font getComboBoxFont(juce::ComboBox&) override { return uiFont(12.5f); }
    juce::Font getPopupMenuFont() override { return uiFont(13.0f); }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        auto fill = backgroundColour;
        if (shouldDrawButtonAsDown)
            fill = fill.darker(0.2f);
        else if (shouldDrawButtonAsHighlighted)
            fill = fill.brighter(0.08f);

        g.setColour(fill);
        g.fillRoundedRectangle(bounds, 5.0f);
        g.setColour(stroke.withAlpha(0.7f));
        g.drawRoundedRectangle(bounds, 5.0f, 1.0f);
    }

    juce::Font getTextButtonFont(juce::TextButton&, int) override { return uiFont(13.0f, true); }
};

// ============================================================================
// ParameterControl: one APVTS-bound control inside a fixed cell.
// ============================================================================
class SynthAudioProcessorEditor::ParameterControl final : public juce::Component
{
public:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    ParameterControl(juce::AudioProcessorValueTreeState& state,
                     const synth::ParameterSpec& parameterSpec,
                     const juce::String& stripPrefix)
        : spec(parameterSpec)
    {
        auto displayName = juce::String(spec.name);
        if (stripPrefix.isNotEmpty() && displayName.startsWith(stripPrefix))
            displayName = displayName.substring(stripPrefix.length());
        if (displayName.endsWith(" Direct")) // redundant inside the Direct Modulation panels
            displayName = displayName.dropLastCharacters(7);

        nameLabel.setText(displayName, juce::dontSendNotification);
        nameLabel.setJustificationType(juce::Justification::centred);
        nameLabel.setColour(juce::Label::textColourId, mutedText);
        nameLabel.setFont(uiFont(11.0f));
        nameLabel.setMinimumHorizontalScale(0.6f);
        addAndMakeVisible(nameLabel);

        if (spec.kind == synth::ParameterKind::Choice)
        {
            wide = true;
            for (int i = 0; i < static_cast<int>(spec.choices.size()); ++i)
                combo.addItem(juce::String(spec.choices[static_cast<std::size_t>(i)]), i + 1);

            comboAttachment = std::make_unique<ComboBoxAttachment>(state, spec.id, combo);
            addAndMakeVisible(combo);
        }
        else if (spec.kind == synth::ParameterKind::Bool)
        {
            toggle.setClickingTogglesState(true);
            buttonAttachment = std::make_unique<ButtonAttachment>(state, spec.id, toggle);
            addAndMakeVisible(toggle);
        }
        else
        {
            slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            slider.setRotaryParameters(rotaryStart, rotaryEnd, true);
            slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 15);
            slider.setColour(juce::Slider::textBoxTextColourId, text);
            slider.setNumDecimalPlacesToDisplay(2); // fallback so text never renders in scientific form

            // The attachment must exist first: SliderParameterAttachment overwrites the
            // text<->value functions in its constructor, so our formatter is applied after.
            sliderAttachment = std::make_unique<SliderAttachment>(state, spec.id, slider);
            slider.textFromValueFunction = [this](double value) { return formatValue(spec, value); };
            slider.valueFromTextFunction = [](const juce::String& textValue) {
                return textValue.retainCharacters("-0123456789.").getDoubleValue();
            };
            slider.updateText();
            addAndMakeVisible(slider);
        }
    }

    bool isWide() const noexcept { return wide; }

    void resized() override
    {
        auto bounds = getLocalBounds();
        nameLabel.setBounds(bounds.removeFromTop(14));

        if (spec.kind == synth::ParameterKind::Choice)
            combo.setBounds(bounds.withSizeKeepingCentre(bounds.getWidth() - 4, 28));
        else if (spec.kind == synth::ParameterKind::Bool)
            toggle.setBounds(bounds.withSizeKeepingCentre(bounds.getWidth(), 30));
        else
            slider.setBounds(bounds);
    }

private:
    synth::ParameterSpec spec;
    bool wide = false;
    juce::Label nameLabel;
    juce::Slider slider;
    juce::ComboBox combo;
    juce::ToggleButton toggle;
    std::unique_ptr<SliderAttachment> sliderAttachment;
    std::unique_ptr<ComboBoxAttachment> comboAttachment;
    std::unique_ptr<ButtonAttachment> buttonAttachment;
};

// ============================================================================
// Meter: peak output meter with clip latch.
// ============================================================================
class SynthAudioProcessorEditor::Meter final : public juce::Component
{
public:
    void setLevel(float peakLinear, bool flagClip)
    {
        peak = std::isfinite(peakLinear) ? peakLinear : 0.0f;
        if (flagClip || peak >= 0.98f)
            clipHold = 12; // hold the clip indicator for a few timer ticks
        else if (clipHold > 0)
            --clipHold;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(fieldBg);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(strokeSoft);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        auto track = bounds.reduced(3.0f);
        auto clipZone = track.removeFromRight(8.0f);
        track.removeFromRight(3.0f);

        const auto db = peak > 0.0001f ? juce::Decibels::gainToDecibels(peak) : -100.0f;
        const auto norm = juce::jlimit(0.0f, 1.0f, (db + 60.0f) / 66.0f);

        auto filled = track.withWidth(track.getWidth() * norm);
        juce::Colour barColour = db > 0.0f ? warn : (db > -6.0f ? staged : live);
        g.setColour(barColour.withAlpha(0.92f));
        g.fillRoundedRectangle(filled, 2.0f);

        g.setColour(clipHold > 0 ? warn : juce::Colour::fromRGB(40, 45, 51));
        g.fillRoundedRectangle(clipZone, 2.0f);

        g.setColour(peak > 0.0001f ? text : mutedText);
        g.setFont(uiFont(10.5f));
        const auto label = peak > 0.0001f ? juce::String(db, 1) + " dB" : "-inf";
        g.drawText(label, track.toNearestInt(), juce::Justification::centredLeft, false);
    }

private:
    float peak = 0.0f;
    int clipHold = 0;
};

// ============================================================================
// Panel: titled frame that grid-packs controls into stable rows.
// ============================================================================
class SynthAudioProcessorEditor::Panel final : public juce::Component
{
public:
    Panel(juce::AudioProcessorValueTreeState& state,
          juce::String panelTitle,
          const std::vector<std::string>& ids,
          juce::String badgeText,
          juce::Colour badgeFill,
          const juce::String& stripPrefix)
        : title(std::move(panelTitle)),
          badge(std::move(badgeText)),
          badgeColour(badgeFill)
    {
        for (const auto& id : ids)
        {
            if (const auto* found = synth::findParameterSpec(id))
            {
                auto control = std::make_unique<ParameterControl>(state, *found, stripPrefix);
                addAndMakeVisible(*control);
                controls.push_back(std::move(control));
            }
        }
    }

    int preferredHeight(int width) const
    {
        return computeLayout(width, false);
    }

    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced(0.5f);
        g.setColour(panelBg);
        g.fillRoundedRectangle(bounds, 6.0f);
        g.setColour(strokeSoft);
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

        auto headerArea = getLocalBounds().removeFromTop(headerHeight);
        g.setColour(panelHeader);
        g.fillRoundedRectangle(headerArea.toFloat().reduced(0.5f), 6.0f);
        g.fillRect(headerArea.removeFromBottom(8)); // square off the lower header corners

        auto titleArea = getLocalBounds().removeFromTop(headerHeight).reduced(12, 0);
        g.setColour(text);
        g.setFont(uiFont(12.0f, true));
        g.drawText(title.toUpperCase(), titleArea, juce::Justification::centredLeft, true);

        if (badge.isNotEmpty())
        {
            const auto badgeWidth = badge.length() * 8 + 16;
            auto badgeArea = titleArea.removeFromRight(badgeWidth).withSizeKeepingCentre(badgeWidth, 16);
            g.setColour(badgeColour.withAlpha(0.18f));
            g.fillRoundedRectangle(badgeArea.toFloat(), 8.0f);
            g.setColour(badgeColour);
            g.setFont(uiFont(10.0f, true));
            g.drawText(badge, badgeArea, juce::Justification::centred, false);
        }
    }

    void resized() override
    {
        computeLayout(getWidth(), true);
    }

private:
    int computeLayout(int width, bool apply) const
    {
        if (width <= 0)
            return headerHeight + padTop + cellHeight + padBottom;

        const auto innerX = padX;
        const auto innerWidth = width - 2 * padX;
        const auto columns = std::max(1, (innerWidth + gap) / (unitWidth + gap));
        const auto columnWidth = static_cast<float>(innerWidth - (columns - 1) * gap) / static_cast<float>(columns);
        const auto topOffset = headerHeight + padTop;

        int column = 0;
        int row = 0;
        for (const auto& control : controls)
        {
            const auto span = std::min(control->isWide() ? 2 : 1, columns);
            if (column + span > columns)
            {
                column = 0;
                ++row;
            }

            if (apply)
            {
                const auto cellX = innerX + static_cast<int>(std::round(static_cast<float>(column) * (columnWidth + static_cast<float>(gap))));
                const auto cellW = static_cast<int>(std::round(static_cast<float>(span) * columnWidth + static_cast<float>((span - 1) * gap)));
                const auto cellY = topOffset + row * (cellHeight + rowGap);
                control->setBounds(cellX, cellY, cellW, cellHeight);
            }

            column += span;
        }

        const auto rowCount = controls.empty() ? 0 : row + 1;
        return topOffset + rowCount * cellHeight + std::max(0, rowCount - 1) * rowGap + padBottom;
    }

    static constexpr int headerHeight = 26;
    static constexpr int unitWidth = 66;
    static constexpr int cellHeight = 64;
    static constexpr int gap = 8;
    static constexpr int rowGap = 4;
    static constexpr int padX = 12;
    static constexpr int padTop = 6;
    static constexpr int padBottom = 10;

    juce::String title;
    juce::String badge;
    juce::Colour badgeColour;
    std::vector<std::unique_ptr<ParameterControl>> controls;
};

// ============================================================================
// Local helpers shared by the editor implementation.
// ============================================================================
namespace
{
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
} // namespace

// ============================================================================
// Editor
// ============================================================================
SynthAudioProcessorEditor::SynthAudioProcessorEditor(SynthAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    lookAndFeel = std::make_unique<SynthLookAndFeel>();
    setLookAndFeel(lookAndFeel.get());

    meter = std::make_unique<Meter>();

    buildHeader();
    buildLayerBar();
    buildPages();

    refreshPresetMenu();
    setSelectedLayer(0);
    setPage(Page::Sound);
    updateDiagnostics();
    startTimerHz(15);

    setResizable(true, true);
    setResizeLimits(1080, 760, 1800, 1320);
    setSize(1320, 940);
}

SynthAudioProcessorEditor::~SynthAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void SynthAudioProcessorEditor::buildHeader()
{
    titleLabel.setText("SYNTH", juce::dontSendNotification);
    titleLabel.setColour(juce::Label::textColourId, text);
    titleLabel.setFont(uiFont(22.0f, true));
    addAndMakeVisible(titleLabel);

    engineTag.setText("CORE OSC ENGINE", juce::dontSendNotification);
    engineTag.setColour(juce::Label::textColourId, live);
    engineTag.setFont(uiFont(10.5f, true));
    addAndMakeVisible(engineTag);

    styleFlatButton(prevPresetButton, juce::Colour::fromRGB(38, 43, 49));
    styleFlatButton(nextPresetButton, juce::Colour::fromRGB(38, 43, 49));
    styleFlatButton(loadButton, accent.darker(0.32f));
    styleFlatButton(saveButton, juce::Colour::fromRGB(58, 84, 108));
    styleFlatButton(duplicateButton, juce::Colour::fromRGB(58, 84, 108));
    styleFlatButton(panicButton, warn.darker(0.1f));
    addAndMakeVisible(prevPresetButton);
    addAndMakeVisible(nextPresetButton);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(saveButton);
    addAndMakeVisible(duplicateButton);
    addAndMakeVisible(panicButton);

    addAndMakeVisible(presetCombo);
    presetCombo.setTextWhenNothingSelected("Select preset");

    nameEditor.setText(audioProcessor.getCurrentPresetName());
    nameEditor.setFont(uiFont(13.0f));
    addAndMakeVisible(nameEditor);

    addAndMakeVisible(*meter);

    voicesCaption.setText("VOICES", juce::dontSendNotification);
    styleChipCaption(voicesCaption);
    styleChipValue(voicesValue);
    addAndMakeVisible(voicesCaption);
    addAndMakeVisible(voicesValue);

    costCaption.setText("LOAD EST.", juce::dontSendNotification);
    styleChipCaption(costCaption);
    styleChipValue(costValue);
    addAndMakeVisible(costCaption);
    addAndMakeVisible(costValue);

    prevPresetButton.onClick = [this] { stepPreset(-1); };
    nextPresetButton.onClick = [this] { stepPreset(1); };
    loadButton.onClick = [this] { loadSelectedPreset(); };
    saveButton.onClick = [this] { savePresetAs(); };
    duplicateButton.onClick = [this] { duplicatePreset(); };
    panicButton.onClick = [this] {
        audioProcessor.requestPanic();
        updateStatus("Panic sent: voices and FX cleared");
    };

    statusLabel.setColour(juce::Label::textColourId, mutedText);
    statusLabel.setFont(uiFont(11.5f));
    statusLabel.setText("Ready", juce::dontSendNotification);
    addAndMakeVisible(statusLabel);

    diagnosticsLabel.setJustificationType(juce::Justification::centredRight);
    diagnosticsLabel.setColour(juce::Label::textColourId, mutedText);
    diagnosticsLabel.setFont(uiFont(11.5f));
    addAndMakeVisible(diagnosticsLabel);
}

void SynthAudioProcessorEditor::buildLayerBar()
{
    layerCaption.setText("LAYER", juce::dontSendNotification);
    layerCaption.setColour(juce::Label::textColourId, mutedText);
    layerCaption.setFont(uiFont(11.0f, true));
    layerCaption.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(layerCaption);

    auto styleLayerButton = [this](juce::TextButton& button) {
        styleFlatButton(button, juce::Colour::fromRGB(38, 43, 49));
        button.setClickingTogglesState(false);
        addAndMakeVisible(button);
    };
    styleLayerButton(layerAButton);
    styleLayerButton(layerBButton);
    layerAButton.onClick = [this] { setSelectedLayer(0); };
    layerBButton.onClick = [this] { setSelectedLayer(1); };

    layerStatusPill.setJustificationType(juce::Justification::centred);
    layerStatusPill.setFont(uiFont(11.0f, true));
    addAndMakeVisible(layerStatusPill);

    addAndMakeVisible(layerControlsHost);

    soundTab.setClickingTogglesState(false);
    modTab.setClickingTogglesState(false);
    fxTab.setClickingTogglesState(false);
    soundTab.onClick = [this] { setPage(Page::Sound); };
    modTab.onClick = [this] { setPage(Page::Mod); };
    fxTab.onClick = [this] { setPage(Page::Fx); };
    addAndMakeVisible(soundTab);
    addAndMakeVisible(modTab);
    addAndMakeVisible(fxTab);

    pageViewport.setScrollBarsShown(true, false);
    pageViewport.setViewedComponent(&soundPage, false);
    addAndMakeVisible(pageViewport);
}

SynthAudioProcessorEditor::Panel* SynthAudioProcessorEditor::addPanel(
    juce::Component& page,
    std::vector<std::unique_ptr<Panel>>& store,
    juce::String title,
    std::vector<std::string> ids,
    juce::String badge,
    juce::Colour badgeColour,
    juce::String stripPrefix)
{
    auto panel = std::make_unique<Panel>(audioProcessor.getValueTreeState(), std::move(title),
                                         ids, std::move(badge), badgeColour, stripPrefix);
    auto* raw = panel.get();
    page.addAndMakeVisible(*raw);
    store.push_back(std::move(panel));
    return raw;
}

void SynthAudioProcessorEditor::buildPages()
{
    // ---- SOUND: the live core sound-design surface ------------------------
    coreOscPanel = addPanel(soundPage, soundPanels, "Oscillator", {
        "osc.pitch_semitones", "osc.fine_cents", "osc.stack_count", "osc.stack_detune",
        "osc.saw_level", "osc.pulse_level", "osc.pulse_width", "osc.noise_level",
        "osc.sub_wave", "osc.sub_octave", "osc.sub_level", "osc.sub_pulse_width",
        "osc.sync_amount", "osc.phase_reset"
    }, "LIVE", live);

    filterPanel = addPanel(soundPage, soundPanels, "Filter", {
        "filter.enabled", "filter.mode", "filter.cutoff_semitones", "filter.resonance",
        "filter.drive", "filter.keytrack", "filter.oversampling"
    });

    ampEnvPanel = addPanel(soundPage, soundPanels, "Amp Envelope", {
        "amp_env.attack_ms", "amp_env.decay_ms", "amp_env.sustain", "amp_env.release_ms"
    });

    modEnvPanel = addPanel(soundPage, soundPanels, "Mod Envelope", {
        "mod_env.attack_ms", "mod_env.decay_ms", "mod_env.sustain", "mod_env.release_ms"
    });

    lfoPanel = addPanel(soundPage, soundPanels, "LFO", {
        "lfo.shape", "lfo.rate_mode", "lfo.rate_hz", "lfo.sync_division",
        "lfo.phase_degrees", "lfo.gate_mode", "lfo.mono", "lfo.swing"
    });

    voicePanel = addPanel(soundPage, soundPanels, "Voice", {
        "voice.mode", "voice.polyphony", "voice.unison_count", "voice.retrigger",
        "voice.glide_ms", "voice.velocity_glide_ms"
    });

    ampPanel = addPanel(soundPage, soundPanels, "Amp / Stereo", {
        "amp.drive", "amp.level_db", "amp.pan", "amp.pan_spread",
        "amp.unison_spread", "amp.analog"
    });

    rampPanel = addPanel(soundPage, soundPanels, "Ramp", {
        "ramp.enabled", "ramp.mode", "ramp.delay_ms", "ramp.rise_ms", "ramp.curve"
    });

    macroPanel = addPanel(soundPage, soundPanels, "Macros", {
        "macro.motion", "macro.width", "macro.drive", "macro.space"
    });

    // ---- MOD: direct routes grouped by destination + the eight TransMod slots
    oscPitchModPanel = addPanel(modPage, modPanels, "Osc Pitch Mod", {
        "direct.osc_keytrack_semitones", "direct.osc_lfo_semitones", "direct.osc_mod_env_semitones"
    }, {}, {}, "Osc ");
    pulseWidthModPanel = addPanel(modPage, modPanels, "Pulse Width Mod", {
        "direct.pulse_keytrack", "direct.pulse_lfo", "direct.pulse_mod_env"
    }, {}, {}, "Pulse ");
    filterCutoffModPanel = addPanel(modPage, modPanels, "Filter Cutoff Mod", {
        "direct.filter_keytrack", "direct.filter_lfo_semitones", "direct.filter_mod_env_semitones"
    }, {}, {}, "Filter ");

    for (int slot = 1; slot <= synth::transModSlotCount; ++slot)
    {
        const auto prefix = "transmod." + std::to_string(slot) + ".";
        const auto title = "TransMod " + juce::String(slot);
        transModPanels[static_cast<std::size_t>(slot - 1)] = addPanel(
            modPage, modPanels, title, {
                prefix + "enabled", prefix + "source", prefix + "scaler",
                prefix + "osc_pitch_semitones", prefix + "pulse_width",
                prefix + "filter_cutoff_semitones", prefix + "amp_level_db", prefix + "pan"
            }, {}, {}, title + " ");
    }

    // ---- FX: a post-voice rack grouped by module, plus master/quality -----
    saturationPanel = addPanel(fxPage, fxPanels, "Saturation", {
        "fx.saturation_enabled", "fx.saturation_mix", "fx.saturation_drive"
    }, {}, {}, "Saturation ");
    chorusPanel = addPanel(fxPage, fxPanels, "Chorus", {
        "fx.chorus_enabled", "fx.chorus_mix", "fx.chorus_rate_hz", "fx.chorus_depth_ms"
    }, {}, {}, "Chorus ");
    delayPanel = addPanel(fxPage, fxPanels, "Delay", {
        "fx.delay_enabled", "fx.delay_mix", "fx.delay_sync_division", "fx.delay_feedback"
    }, {}, {}, "Delay ");
    reverbPanel = addPanel(fxPage, fxPanels, "Reverb", {
        "fx.reverb_enabled", "fx.reverb_mix", "fx.reverb_decay"
    }, {}, {}, "Reverb ");
    masterFxPanel = addPanel(fxPage, fxPanels, "Master / Quality", {
        "fx.enabled", "quality.realtime_mode", "quality.offline_mode"
    });
}

void SynthAudioProcessorEditor::setSelectedLayer(int layerIndex)
{
    selectedLayer = juce::jlimit(0, synth::layerCount - 1, layerIndex);
    const auto layerNumber = selectedLayer + 1;
    const auto layerLetter = juce::String::charToString(static_cast<juce::juce_wchar>('A' + selectedLayer));
    const auto layerPrefix = "layer." + juce::String(layerNumber) + ".";
    const auto stripPrefix = "Layer " + layerLetter + " ";

    auto activeColour = accent;
    auto inactiveColour = juce::Colour::fromRGB(38, 43, 49);
    styleFlatButton(layerAButton, selectedLayer == 0 ? activeColour : inactiveColour);
    styleFlatButton(layerBButton, selectedLayer == 1 ? activeColour : inactiveColour);

    // Render-boundary truth: Layer A maps to the live core path; Layer B is staged state.
    const auto live_ = selectedLayer == 0;
    layerStatusPill.setText(live_ ? "LIVE - core path" : "STAGED - not yet rendered",
                            juce::dontSendNotification);
    layerStatusPill.setColour(juce::Label::textColourId, live_ ? live : staged);

    // Rebuild per-layer mix controls for the selected layer.
    layerControls.clear();
    auto& state = audioProcessor.getValueTreeState();
    for (const auto* suffix : { "enabled", "level_db", "pan", "solo", "mute" })
    {
        const auto id = (layerPrefix + suffix).toStdString();
        if (const auto* spec = synth::findParameterSpec(id))
        {
            auto control = std::make_unique<ParameterControl>(state, *spec, stripPrefix);
            layerControlsHost.addAndMakeVisible(*control);
            layerControls.push_back(std::move(control));
        }
    }

    // Rebuild the two oscillator-slot panels for the selected layer. These bind to real
    // layer.N.osc.M.* state but are badged STATE because per-slot rendering is not live.
    for (int slot = 0; slot < synth::oscillatorSlotsPerLayer; ++slot)
    {
        if (slotPanels[static_cast<std::size_t>(slot)] != nullptr)
            soundPage.removeChildComponent(slotPanels[static_cast<std::size_t>(slot)].get());

        const auto slotNumber = slot + 1;
        const auto oscPrefix = (layerPrefix + "osc." + juce::String(slotNumber) + ".").toStdString();
        const auto slotTitle = "Osc " + layerLetter + juce::String(slotNumber);
        const auto slotStrip = "Layer " + layerLetter + " Osc " + juce::String(slotNumber) + " ";

        auto panel = std::make_unique<Panel>(state, slotTitle, std::vector<std::string>{
            oscPrefix + "enabled", oscPrefix + "waveform", oscPrefix + "voices",
            oscPrefix + "octave", oscPrefix + "note", oscPrefix + "fine_cents",
            oscPrefix + "level", oscPrefix + "phase_degrees", oscPrefix + "detune",
            oscPrefix + "stereo", oscPrefix + "pan", oscPrefix + "retrigger",
            oscPrefix + "invert"
        }, "STATE", staged, slotStrip);
        soundPage.addAndMakeVisible(*panel);
        slotPanels[static_cast<std::size_t>(slot)] = std::move(panel);
    }

    resized();
}

void SynthAudioProcessorEditor::setPage(Page page)
{
    currentPage = page;

    juce::Component* viewed = &soundPage;
    if (page == Page::Mod)
        viewed = &modPage;
    else if (page == Page::Fx)
        viewed = &fxPage;
    pageViewport.setViewedComponent(viewed, false);

    auto styleTab = [](juce::TextButton& tab, bool active) {
        styleFlatButton(tab, active ? accent.darker(0.18f) : juce::Colour::fromRGB(30, 34, 39),
                        active ? juce::Colour::fromRGB(12, 14, 16) : mutedText);
    };
    styleTab(soundTab, page == Page::Sound);
    styleTab(modTab, page == Page::Mod);
    styleTab(fxTab, page == Page::Fx);

    layoutActivePage();
}

void SynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(background);

    auto header = getLocalBounds().removeFromTop(headerHeight);
    g.setColour(headerBg);
    g.fillRect(header);

    auto layerBar = getLocalBounds().withTop(headerHeight).removeFromTop(layerBarHeight);
    g.setColour(juce::Colour::fromRGB(22, 25, 29));
    g.fillRect(layerBar);
    g.setColour(strokeSoft);
    g.drawHorizontalLine(headerHeight, 0.0f, static_cast<float>(getWidth()));
    g.drawHorizontalLine(headerHeight + layerBarHeight, 0.0f, static_cast<float>(getWidth()));

    auto footer = getLocalBounds().removeFromBottom(footerHeight);
    g.setColour(headerBg);
    g.fillRect(footer);
    g.setColour(strokeSoft);
    g.drawHorizontalLine(getHeight() - footerHeight, 0.0f, static_cast<float>(getWidth()));
}

void SynthAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    layoutHeader(area.removeFromTop(headerHeight));
    layoutLayerBar(area.removeFromTop(layerBarHeight));

    auto footer = area.removeFromBottom(footerHeight).reduced(16, 0);
    diagnosticsLabel.setBounds(footer.removeFromRight(640));
    statusLabel.setBounds(footer);

    auto tabs = area.removeFromTop(tabBarHeight).reduced(16, 6);
    const auto tabWidth = 116;
    soundTab.setBounds(tabs.removeFromLeft(tabWidth));
    tabs.removeFromLeft(6);
    modTab.setBounds(tabs.removeFromLeft(tabWidth));
    tabs.removeFromLeft(6);
    fxTab.setBounds(tabs.removeFromLeft(tabWidth));

    pageViewport.setBounds(area.reduced(12, 8));
    layoutActivePage();
}

void SynthAudioProcessorEditor::layoutHeader(juce::Rectangle<int> area)
{
    area = area.reduced(16, 9);

    auto left = area.removeFromLeft(154);
    titleLabel.setBounds(left.removeFromTop(28));
    engineTag.setBounds(left.removeFromTop(16));

    auto centreInHeight = [&area](juce::Rectangle<int> slot, int height) {
        return slot.withSizeKeepingCentre(slot.getWidth(), height).translated(0, 0)
            .withY(area.getY() + (area.getHeight() - height) / 2);
    };

    // Right cluster: panic, meter, chips.
    panicButton.setBounds(centreInHeight(area.removeFromRight(74), 30));
    area.removeFromRight(10);
    meter->setBounds(centreInHeight(area.removeFromRight(170), 26));
    area.removeFromRight(12);

    auto costChip = area.removeFromRight(60);
    costCaption.setBounds(costChip.removeFromTop(costChip.getHeight() / 2));
    costValue.setBounds(costChip);
    area.removeFromRight(6);
    auto voicesChip = area.removeFromRight(60);
    voicesCaption.setBounds(voicesChip.removeFromTop(voicesChip.getHeight() / 2));
    voicesValue.setBounds(voicesChip);
    area.removeFromRight(14);

    // Middle cluster: preset nav with a flexible combo.
    auto nav = area;
    prevPresetButton.setBounds(centreInHeight(nav.removeFromLeft(30), 30));
    nav.removeFromLeft(4);
    duplicateButton.setBounds(centreInHeight(nav.removeFromRight(88), 30));
    nav.removeFromRight(6);
    saveButton.setBounds(centreInHeight(nav.removeFromRight(74), 30));
    nav.removeFromRight(6);
    loadButton.setBounds(centreInHeight(nav.removeFromRight(58), 30));
    nav.removeFromRight(10);
    nameEditor.setBounds(centreInHeight(nav.removeFromRight(150), 28));
    nav.removeFromRight(8);
    nextPresetButton.setBounds(centreInHeight(nav.removeFromRight(30), 30));
    nav.removeFromRight(6);
    presetCombo.setBounds(centreInHeight(nav, 30));
}

void SynthAudioProcessorEditor::layoutLayerBar(juce::Rectangle<int> area)
{
    area = area.reduced(16, 8);

    auto verticalCentre = [&area](juce::Rectangle<int> slot, int height) {
        return slot.withY(area.getY() + (area.getHeight() - height) / 2).withHeight(height);
    };

    layerCaption.setBounds(verticalCentre(area.removeFromLeft(48), 20));
    layerAButton.setBounds(verticalCentre(area.removeFromLeft(40), 30));
    area.removeFromLeft(6);
    layerBButton.setBounds(verticalCentre(area.removeFromLeft(40), 30));
    area.removeFromLeft(14);
    layerStatusPill.setBounds(verticalCentre(area.removeFromLeft(184), 22));
    area.removeFromLeft(8);

    layerControlsHost.setBounds(area);
    for (int i = 0; i < static_cast<int>(layerControls.size()); ++i)
        layerControls[static_cast<std::size_t>(i)]->setBounds(i * (72 + 6), 0, 72, layerControlsHost.getHeight());
}

int SynthAudioProcessorEditor::layoutRows(juce::Component& page,
                                          const std::vector<std::vector<RowItem>>& rows,
                                          int width)
{
    const auto outerPad = 4;
    const auto rowGap = 12;
    const auto columnGap = 12;
    const auto usableWidth = width - 2 * outerPad;

    int y = outerPad;
    for (const auto& row : rows)
    {
        const auto totalGap = columnGap * std::max(0, static_cast<int>(row.size()) - 1);
        const auto contentWidth = usableWidth - totalGap;

        int rowHeight = 0;
        std::vector<int> widths;
        widths.reserve(row.size());
        for (const auto& item : row)
        {
            const auto itemWidth = static_cast<int>(std::round(item.widthFraction * static_cast<float>(contentWidth)));
            widths.push_back(itemWidth);
            rowHeight = std::max(rowHeight, item.panel->preferredHeight(itemWidth));
        }

        int x = outerPad;
        for (std::size_t i = 0; i < row.size(); ++i)
        {
            row[i].panel->setBounds(x, y, widths[i], rowHeight);
            x += widths[i] + columnGap;
        }

        y += rowHeight + rowGap;
    }

    page.setSize(width, y - rowGap + outerPad);
    return page.getHeight();
}

void SynthAudioProcessorEditor::layoutActivePage()
{
    const auto viewWidth = std::max(900, pageViewport.getMaximumVisibleWidth());

    if (currentPage == Page::Sound)
    {
        if (slotPanels[0] == nullptr || coreOscPanel == nullptr)
            return;
        std::vector<std::vector<RowItem>> rows = {
            { { slotPanels[0].get(), 0.5f }, { slotPanels[1].get(), 0.5f } },
            { { coreOscPanel, 1.0f } },
            { { filterPanel, 0.32f }, { ampEnvPanel, 0.14f }, { modEnvPanel, 0.14f }, { lfoPanel, 0.40f } },
            { { voicePanel, 0.26f }, { ampPanel, 0.24f }, { rampPanel, 0.26f }, { macroPanel, 0.24f } },
        };
        layoutRows(soundPage, rows, viewWidth);
    }
    else if (currentPage == Page::Mod)
    {
        std::vector<std::vector<RowItem>> rows = {
            { { oscPitchModPanel, 0.34f }, { pulseWidthModPanel, 0.33f }, { filterCutoffModPanel, 0.33f } },
            { { transModPanels[0], 0.25f }, { transModPanels[1], 0.25f },
              { transModPanels[2], 0.25f }, { transModPanels[3], 0.25f } },
            { { transModPanels[4], 0.25f }, { transModPanels[5], 0.25f },
              { transModPanels[6], 0.25f }, { transModPanels[7], 0.25f } },
        };
        layoutRows(modPage, rows, viewWidth);
    }
    else
    {
        std::vector<std::vector<RowItem>> rows = {
            { { saturationPanel, 0.25f }, { chorusPanel, 0.25f },
              { delayPanel, 0.25f }, { reverbPanel, 0.25f } },
            { { masterFxPanel, 0.5f } },
        };
        layoutRows(fxPage, rows, viewWidth);
    }
}

void SynthAudioProcessorEditor::refreshPresetMenu()
{
    presetItems = audioProcessor.getPresetList();
    presetCombo.clear(juce::dontSendNotification);

    for (int i = 0; i < static_cast<int>(presetItems.size()); ++i)
    {
        const auto& item = presetItems[static_cast<std::size_t>(i)];
        presetCombo.addItem((item.factory ? "Factory: " : "User: ") + item.displayName, i + 1);
    }

    if (presetItems.empty())
    {
        presetCombo.setTextWhenNothingSelected("No presets found");
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
        nameEditor.setText(audioProcessor.getCurrentPresetName(), juce::dontSendNotification);
        refreshPresetMenu();
    }

    updateStatus(message);
}

void SynthAudioProcessorEditor::stepPreset(int direction)
{
    if (presetItems.empty())
    {
        updateStatus("No presets found");
        return;
    }

    auto index = presetCombo.getSelectedItemIndex();
    if (index < 0)
        index = direction > 0 ? -1 : 0;

    index = (index + direction + static_cast<int>(presetItems.size()))
            % static_cast<int>(presetItems.size());
    presetCombo.setSelectedItemIndex(index, juce::dontSendNotification);
    loadSelectedPreset();
}

void SynthAudioProcessorEditor::savePresetAs()
{
    const auto presetName = nameEditor.getText().trim().isNotEmpty()
        ? nameEditor.getText().trim()
        : audioProcessor.getCurrentPresetName();
    const auto suggestedFile = audioProcessor.getUserPresetDirectory()
        .getChildFile(fileSafeName(presetName) + ".json");

    fileChooser = std::make_unique<juce::FileChooser>("Save Synth preset", suggestedFile, "*.json");
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
    auto presetName = nameEditor.getText().trim();
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
        nameEditor.setText(presetName, juce::dontSendNotification);
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
    const auto peakDb = snapshot.peak > 0.0001f
        ? juce::String(juce::Decibels::gainToDecibels(snapshot.peak), 1) + " dB"
        : "-inf";

    diagnosticsLabel.setText("SR " + sampleRate
                             + "   Block " + juce::String(snapshot.blockSize)
                             + "   Peak " + peakDb
                             + "   MIDI " + juce::String(snapshot.midiEvents)
                             + "   Invalid " + juce::String(snapshot.invalidSamples)
                             + "   " + snapshot.architecture,
                             juce::dontSendNotification);

    voicesValue.setText(juce::String(snapshot.activeVoices), juce::dontSendNotification);

    // Patch-load estimate derived from the live rendering path (UI-thread only).
    auto raw = [this](const char* id, float fallback) {
        const auto* parameter = audioProcessor.getValueTreeState().getRawParameterValue(id);
        return parameter != nullptr ? parameter->load() : fallback;
    };
    const auto polyphony = raw("voice.polyphony", 8.0f);
    const auto unison = raw("voice.unison_count", 1.0f);
    const auto stack = raw("osc.stack_count", 1.0f);
    const auto filterOn = raw("filter.enabled", 1.0f) >= 0.5f;
    const auto oversample = synth::oversamplingFactor(static_cast<int>(std::round(raw("filter.oversampling", 1.0f))));
    const auto fxOn = raw("fx.enabled", 0.0f) >= 0.5f;
    const auto fxModules = (raw("fx.saturation_enabled", 1.0f) >= 0.5f ? 1 : 0)
                         + (raw("fx.delay_enabled", 1.0f) >= 0.5f ? 1 : 0)
                         + (raw("fx.reverb_enabled", 1.0f) >= 0.5f ? 1 : 0)
                         + (raw("fx.chorus_enabled", 0.0f) >= 0.5f ? 1 : 0);

    const auto oscVoices = polyphony * unison * stack;
    const auto filterMultiplier = filterOn ? (0.5f + 0.5f * static_cast<float>(oversample)) : 1.0f;
    const auto fxMultiplier = fxOn ? (1.0f + 0.06f * static_cast<float>(fxModules)) : 1.0f;
    const auto units = oscVoices * filterMultiplier * fxMultiplier;
    const auto percent = static_cast<int>(std::round(units / 240.0f * 100.0f));

    costValue.setText(percent <= 100 ? juce::String(percent) + "%" : ">100%", juce::dontSendNotification);
    costValue.setColour(juce::Label::textColourId, percent > 90 ? warn : (percent > 60 ? staged : text));

    const auto clip = snapshot.peak >= 0.98f || snapshot.invalidSamples > lastInvalidSamples;
    lastInvalidSamples = snapshot.invalidSamples;
    meter->setLevel(snapshot.peak, clip);
}

void SynthAudioProcessorEditor::timerCallback()
{
    updateDiagnostics();
}
