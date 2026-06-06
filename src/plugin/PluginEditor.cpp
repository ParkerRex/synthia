#include "PluginEditor.h"

#include "../presets/PresetManager.h"

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
const auto info         = juce::Colour::fromRGB(108, 158, 226);
const auto knobFill     = juce::Colour::fromRGB(40, 45, 51);
const auto knobStroke   = juce::Colour::fromRGB(62, 70, 78);

// Functional zone hues drive each module's header identity tick so the grid reads as a
// rack of grouped modules rather than a uniform form. Restrained on purpose: source,
// shaping, performance, and modulation are the only zones; FX keeps per-module badges.
const auto zoneSource   = live;   // oscillators / tone
const auto zoneShape    = accent; // filter / envelopes / LFO
const auto zonePerform  = info;   // voice / amp / ramp / macros
const auto zoneMod      = accent; // modulation routes
const auto zoneUtility  = juce::Colour::fromRGB(96, 104, 112); // browser / MIDI

constexpr float rotaryStart = juce::MathConstants<float>::pi * 1.25f;
constexpr float rotaryEnd   = juce::MathConstants<float>::pi * 2.75f;

// Fixed shell chrome heights (editor body math reads these by name).
constexpr int headerHeight   = 66;
constexpr int layerBarHeight  = 86;
constexpr int tabBarHeight    = 40;
constexpr int footerHeight    = 26;

juce::Font uiFont(float size = 13.0f, bool bold = false)
{
    return juce::Font(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain));
}

juce::String formatValue(const synth::ParameterSpec& spec, double value)
{
    if (!std::isfinite(value))
        value = 0.0;

    auto snapToDisplayedZero = [](double number, double displayedStep) {
        return std::abs(number) < displayedStep * 0.5 ? 0.0 : number;
    };

    const auto& unit = spec.unit;
    if (unit == "dB")
        return juce::String(snapToDisplayedZero(value, 0.1), 1) + " dB";
    if (unit == "Hz")
    {
        const auto decimals = value < 10.0 ? 2 : 1;
        return juce::String(snapToDisplayedZero(value, decimals == 2 ? 0.01 : 0.1), decimals) + " Hz";
    }
    if (unit == "milliseconds")
    {
        value = snapToDisplayedZero(value, value < 10.0 ? 0.1 : 1.0);
        return value >= 1000.0 ? juce::String(value / 1000.0, 2) + " s"
                               : juce::String(value, value < 10.0 ? 1 : 0) + " ms";
    }
    if (unit == "semitones")
        return juce::String(snapToDisplayedZero(value, 0.1), 1) + " st";
    if (unit == "cents")
        return juce::String(juce::roundToInt(snapToDisplayedZero(value, 1.0))) + " ct";
    if (unit == "percent")
        return juce::String(juce::roundToInt(snapToDisplayedZero(value, 0.01) * 100.0)) + " %";
    if (unit == "degrees")
        return juce::String(juce::roundToInt(snapToDisplayedZero(value, 1.0))) + juce::String::fromUTF8("\xc2\xb0");
    if (unit == "octaves")
    {
        const auto octaves = static_cast<int>(std::round(snapToDisplayedZero(value, 1.0)));
        return (octaves > 0 ? "+" : "") + juce::String(octaves);
    }
    if (unit == "voices")
        return juce::String(static_cast<int>(std::round(snapToDisplayedZero(value, 1.0))));
    return juce::String(snapToDisplayedZero(value, 0.01), 2);
}

juce::String formatGridValue(const synth::ParameterSpec& spec, double value)
{
    if (!std::isfinite(value))
        value = 0.0;

    if (spec.unit == "normalized" || spec.unit == "percent")
        return juce::String(juce::roundToInt(value * 100.0)) + "%";

    if (spec.unit == "semitones")
    {
        const auto semitones = juce::roundToInt(value);
        return (semitones > 0 ? "+" : "") + juce::String(semitones);
    }

    if (spec.unit == "voices" || spec.unit == "steps" || spec.unit == "octaves")
        return juce::String(juce::roundToInt(value));

    return formatValue(spec, value);
}

double parseValueText(const synth::ParameterSpec& spec, const juce::String& textValue)
{
    const auto trimmedText = textValue.trim().toLowerCase();
    const auto number = trimmedText.retainCharacters("-0123456789.").getDoubleValue();

    if (spec.unit == "percent" || spec.unit == "normalized")
        return trimmedText.containsChar('%') || std::abs(number) > 1.0 ? number / 100.0 : number;

    if (spec.unit == "milliseconds")
    {
        if (trimmedText.contains("ms"))
            return number;
        if (trimmedText.containsChar('s'))
            return number * 1000.0;
    }

    return number;
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

// Draws the small rounded zone tick at the left of a module header so every panel reads
// as a labelled module in a rack. Dimmed when the module's power toggle is off.
void paintModuleHeaderTick(juce::Graphics& g, juce::Rectangle<int> headerArea,
                           juce::Colour colour, bool enabled = true)
{
    const auto tick = juce::Rectangle<float>(3.0f, 13.0f)
                          .withCentre(headerArea.toFloat().getCentre())
                          .withX(static_cast<float>(headerArea.getX()) + 6.0f);
    g.setColour(enabled ? colour : colour.withAlpha(0.32f));
    g.fillRoundedRectangle(tick, 1.5f);
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

        // Faint min/max ticks just outside the track give the knob a hardware-instrument read.
        const auto tickInner = arcRadius + 1.5f;
        const auto tickOuter = arcRadius + 4.0f;
        g.setColour(juce::Colour::fromRGB(70, 78, 86));
        for (const auto angle : { startAngle, endAngle })
        {
            const auto sinA = std::sin(angle);
            const auto cosA = std::cos(angle);
            g.drawLine(centreX + tickInner * sinA, centreY - tickInner * cosA,
                       centreX + tickOuter * sinA, centreY - tickOuter * cosA, 1.4f);
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
// LayoutSection: common contract for fixed-height editor sections.
// ============================================================================
class SynthAudioProcessorEditor::LayoutSection : public juce::Component
{
public:
    virtual int preferredHeight(int width) const = 0;
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
            slider.valueFromTextFunction = [this](const juce::String& textValue) { return parseValueText(spec, textValue); };
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
class SynthAudioProcessorEditor::Panel final : public LayoutSection
{
public:
    Panel(juce::AudioProcessorValueTreeState& state,
          juce::String panelTitle,
          const std::vector<std::string>& ids,
          juce::String badgeText,
          juce::Colour badgeFill,
          const juce::String& stripPrefix,
          const juce::String& enabledParamId = {})
        : title(std::move(panelTitle)),
          badge(std::move(badgeText)),
          badgeColour(badgeFill)
    {
        if (enabledParamId.isNotEmpty())
        {
            enabledParam = state.getRawParameterValue(enabledParamId);
            lastEnabledState = isModuleEnabled();
        }

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

    int preferredHeight(int width) const override
    {
        return computeLayout(width, false);
    }

    // True when this panel carries a module power toggle that drives its dimmed state.
    bool hasEnabledState() const noexcept { return enabledParam != nullptr; }

    // UI-thread read of the bound enable atomic; modules with no toggle read as on.
    bool isModuleEnabled() const noexcept
    {
        return enabledParam == nullptr || enabledParam->load() >= 0.5f;
    }

    // Called from the editor timer for the visible page only. Repaints the panel when
    // its power toggle flips so the header reflects bypass without per-frame churn.
    void syncEnabledState()
    {
        if (enabledParam == nullptr)
            return;

        const auto enabled = isModuleEnabled();
        if (enabled != lastEnabledState)
        {
            lastEnabledState = enabled;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override
    {
        const auto hasState = hasEnabledState();
        const auto on = isModuleEnabled();

        const auto bounds = getLocalBounds().toFloat().reduced(0.5f);
        g.setColour(panelBg);
        g.fillRoundedRectangle(bounds, 6.0f);
        g.setColour(strokeSoft);
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

        auto headerArea = getLocalBounds().removeFromTop(headerHeight);
        g.setColour(hasState && !on ? panelHeader.darker(0.14f) : panelHeader);
        g.fillRoundedRectangle(headerArea.toFloat().reduced(0.5f), 6.0f);
        g.fillRect(headerArea.removeFromBottom(8)); // square off the lower header corners

        // Zone identity tick: badgeColour doubles as the functional-zone hue.
        const auto tickColour = badgeColour.isTransparent() ? accent.withAlpha(0.65f) : badgeColour;
        paintModuleHeaderTick(g, getLocalBounds().removeFromTop(headerHeight), tickColour, !hasState || on);

        auto titleArea = getLocalBounds().removeFromTop(headerHeight).reduced(16, 0);

        // Power dot: a scan-friendly on/off indicator for modules with an enable toggle.
        if (hasState)
        {
            auto dotArea = titleArea.removeFromLeft(14);
            const auto dot = juce::Rectangle<float>(0.0f, 0.0f, 7.0f, 7.0f)
                                 .withCentre(dotArea.toFloat().getCentre());
            g.setColour(on ? live : juce::Colour::fromRGB(74, 82, 90));
            g.fillEllipse(dot);
        }

        juce::Rectangle<int> badgeArea;
        if (badge.isNotEmpty())
        {
            const auto badgeWidth = badge.length() * 8 + 16;
            badgeArea = titleArea.removeFromRight(badgeWidth).withSizeKeepingCentre(badgeWidth, 16);
            titleArea.removeFromRight(8);
        }

        g.setColour(hasState && !on ? mutedText : text);
        g.setFont(uiFont(12.0f, true));
        g.drawText(title.toUpperCase(), titleArea, juce::Justification::centredLeft, true);

        if (badge.isNotEmpty())
        {
            const auto badgeActive = !hasState || on;
            g.setColour(badgeActive ? badgeColour.withAlpha(0.18f) : strokeSoft.withAlpha(0.35f));
            g.fillRoundedRectangle(badgeArea.toFloat(), 8.0f);
            g.setColour(badgeActive ? badgeColour : mutedText);
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
    static constexpr int unitWidth = 62;
    static constexpr int cellHeight = 60;
    static constexpr int gap = 7;
    static constexpr int rowGap = 4;
    static constexpr int padX = 11;
    static constexpr int padTop = 6;
    static constexpr int padBottom = 9;

    juce::String title;
    juce::String badge;
    juce::Colour badgeColour;
    std::atomic<float>* enabledParam = nullptr;
    bool lastEnabledState = true;
    std::vector<std::unique_ptr<ParameterControl>> controls;
};

// ============================================================================
// PresetBrowserPanel: searchable preset list over the real preset catalog.
// ============================================================================
class SynthAudioProcessorEditor::PresetBrowserPanel final : public LayoutSection,
                                                           private juce::ListBoxModel
{
public:
    explicit PresetBrowserPanel(SynthAudioProcessorEditor& editor)
        : owner(editor)
    {
        searchEditor.setTextToShowWhenEmpty("Search", mutedText);
        searchEditor.setFont(uiFont(12.0f));
        searchEditor.onTextChange = [this] { applyFilters(); };
        addAndMakeVisible(searchEditor);

        categoryBox.setTextWhenNothingSelected("All Categories");
        categoryBox.onChange = [this] {
            if (rebuildingCategoryBox)
                return;

            selectedCategory = categoryBox.getSelectedId() <= 1 ? juce::String{} : categoryBox.getText();
            applyFilters();
        };
        addAndMakeVisible(categoryBox);

        configureFilterButton(factoryButton, "Factory", true);
        configureFilterButton(userButton, "User", true);
        configureFilterButton(legacyButton, "Legacy", true);
        configureFilterButton(favoritesButton, "Fav", false);
        refreshFilterButtons();

        presetList.setModel(this);
        presetList.setRowHeight(rowHeight);
        presetList.setMultipleSelectionEnabled(false);
        presetList.setColour(juce::ListBox::backgroundColourId, fieldBg);
        presetList.setOutlineThickness(0);
        addAndMakeVisible(presetList);
    }

    int preferredHeight(int) const override
    {
        return 226;
    }

    void setItems(std::vector<SynthAudioProcessor::PresetListItem> items,
                  const juce::String& activePresetPath,
                  const juce::String& activePresetName)
    {
        allItems = std::move(items);
        currentPresetPath = activePresetPath;
        currentPresetName = activePresetName;
        rebuildCategoryFilter();
        applyFilters();
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
        g.fillRect(headerArea.removeFromBottom(8));

        paintModuleHeaderTick(g, getLocalBounds().removeFromTop(headerHeight), zoneUtility);
        auto titleArea = getLocalBounds().removeFromTop(headerHeight).reduced(16, 0);
        auto countArea = titleArea.removeFromRight(88).withSizeKeepingCentre(88, 16);
        titleArea.removeFromRight(8);

        g.setColour(text);
        g.setFont(uiFont(12.0f, true));
        g.drawText("PRESET BROWSER", titleArea, juce::Justification::centredLeft, true);

        g.setColour(accent.withAlpha(0.18f));
        g.fillRoundedRectangle(countArea.toFloat(), 8.0f);
        g.setColour(accent);
        g.setFont(uiFont(10.0f, true));
        g.drawText(juce::String(filteredItemIndices.size()) + " / " + juce::String(allItems.size()),
                   countArea, juce::Justification::centred, false);
    }

    void resized() override
    {
        auto content = getLocalBounds();
        content.removeFromTop(headerHeight);
        content = content.reduced(padX, padY);

        auto filters = content.removeFromTop(filterHeight);
        searchEditor.setBounds(filters.removeFromLeft(260));
        filters.removeFromLeft(8);
        categoryBox.setBounds(filters.removeFromLeft(176));
        filters.removeFromLeft(12);

        const auto placeButton = [&filters](juce::TextButton& button, int width) {
            button.setBounds(filters.removeFromLeft(width));
            filters.removeFromLeft(6);
        };

        placeButton(factoryButton, 74);
        placeButton(userButton, 58);
        placeButton(legacyButton, 64);
        placeButton(favoritesButton, 52);

        content.removeFromTop(8);
        presetList.setBounds(content);
    }

private:
    static constexpr int headerHeight = 26;
    static constexpr int padX = 12;
    static constexpr int padY = 8;
    static constexpr int filterHeight = 28;
    static constexpr int rowHeight = 28;
    static constexpr int favoriteHitWidth = 34;

    void configureFilterButton(juce::TextButton& button, const juce::String& label, bool active)
    {
        button.setButtonText(label);
        button.setClickingTogglesState(true);
        button.setToggleState(active, juce::dontSendNotification);
        button.onClick = [this] {
            refreshFilterButtons();
            applyFilters();
        };
        addAndMakeVisible(button);
    }

    void refreshFilterButtons()
    {
        auto refreshButton = [](juce::TextButton& button) {
            const auto active = button.getToggleState();
            styleFlatButton(button,
                            active ? accent.darker(0.18f) : fieldBg,
                            active ? juce::Colour::fromRGB(12, 14, 16) : mutedText);
        };

        refreshButton(factoryButton);
        refreshButton(userButton);
        refreshButton(legacyButton);
        refreshButton(favoritesButton);
    }

    void rebuildCategoryFilter()
    {
        juce::StringArray categories;
        for (const auto& item : allItems)
        {
            const auto category = item.category.trim();
            if (category.isNotEmpty() && !categories.contains(category, true))
                categories.add(category);
        }
        categories.sort(true);

        const auto previousCategory = selectedCategory;
        rebuildingCategoryBox = true;
        categoryBox.clear(juce::dontSendNotification);
        categoryBox.addItem("All Categories", 1);

        selectedCategory = {};
        int selectedId = 1;
        for (int index = 0; index < categories.size(); ++index)
        {
            const auto id = index + 2;
            categoryBox.addItem(categories[index], id);
            if (categories[index] == previousCategory)
            {
                selectedCategory = previousCategory;
                selectedId = id;
            }
        }

        categoryBox.setSelectedId(selectedId, juce::dontSendNotification);
        rebuildingCategoryBox = false;
    }

    bool sourceIncluded(const SynthAudioProcessor::PresetListItem& item) const
    {
        const auto source = item.sourceLabel.trim().toLowerCase();
        if (item.factory)
            return factoryButton.getToggleState();
        if (source == "legacy user")
            return legacyButton.getToggleState();
        return userButton.getToggleState();
    }

    bool matchesSearch(const SynthAudioProcessor::PresetListItem& item, const juce::String& query) const
    {
        if (query.isEmpty())
            return true;

        auto containsQuery = [&query](const juce::String& value) {
            return value.toLowerCase().contains(query);
        };

        if (containsQuery(item.displayName) || containsQuery(item.bank) || containsQuery(item.category)
            || containsQuery(item.sourceLabel) || containsQuery(item.file.getFileNameWithoutExtension()))
            return true;

        for (const auto& tag : item.tags)
        {
            if (containsQuery(tag))
                return true;
        }

        return false;
    }

    void applyFilters()
    {
        filteredItemIndices.clear();
        const auto query = searchEditor.getText().trim().toLowerCase();

        for (int index = 0; index < static_cast<int>(allItems.size()); ++index)
        {
            const auto& item = allItems[static_cast<std::size_t>(index)];
            if (!sourceIncluded(item))
                continue;
            if (favoritesButton.getToggleState() && !item.favorite)
                continue;
            if (selectedCategory.isNotEmpty() && item.category != selectedCategory)
                continue;
            if (!matchesSearch(item, query))
                continue;

            filteredItemIndices.push_back(index);
        }

        presetList.updateContent();
        const auto activeRow = activeFilteredRow();
        if (activeRow >= 0)
            presetList.selectRow(activeRow, false, true);
        else
            presetList.deselectAllRows();
        presetList.repaint();
    }

    int activeFilteredRow() const
    {
        for (int row = 0; row < static_cast<int>(filteredItemIndices.size()); ++row)
        {
            const auto itemIndex = filteredItemIndices[static_cast<std::size_t>(row)];
            const auto& item = allItems[static_cast<std::size_t>(itemIndex)];
            if (isCurrentPreset(item))
                return row;
        }
        return -1;
    }

    int getNumRows() override
    {
        return static_cast<int>(filteredItemIndices.size());
    }

    void paintListBoxItem(int rowNumber,
                          juce::Graphics& g,
                          int width,
                          int height,
                          bool rowIsSelected) override
    {
        if (rowNumber < 0 || rowNumber >= getNumRows())
            return;

        const auto itemIndex = filteredItemIndices[static_cast<std::size_t>(rowNumber)];
        const auto& item = allItems[static_cast<std::size_t>(itemIndex)];
        const auto active = isCurrentPreset(item);

        auto row = juce::Rectangle<int>(0, 0, width, height).reduced(1, 1);
        g.setColour(active ? accent.withAlpha(0.18f)
                           : (rowIsSelected ? strokeSoft.brighter(0.18f) : fieldBg));
        g.fillRoundedRectangle(row.toFloat(), 4.0f);

        auto favoriteArea = row.removeFromLeft(favoriteHitWidth);
        const auto starBounds = favoriteArea.withSizeKeepingCentre(20, 18).toFloat();
        g.setColour(item.favorite ? staged.withAlpha(0.22f) : strokeSoft.withAlpha(0.55f));
        g.fillRoundedRectangle(starBounds, 5.0f);
        // Filled star when favorited, hollow star otherwise so the toggle affordance always reads.
        g.setColour(item.favorite ? staged : mutedText);
        g.setFont(uiFont(13.0f, false));
        g.drawText(juce::String::fromUTF8(item.favorite ? "\xe2\x98\x85" : "\xe2\x98\x86"),
                   favoriteArea, juce::Justification::centred, false);

        row.removeFromLeft(4);
        auto sourceArea = row.removeFromRight(92);
        row.removeFromRight(8);
        auto metaArea = row.removeFromRight(218);
        row.removeFromRight(8);

        const auto displayName = item.displayName.isNotEmpty()
            ? item.displayName
            : item.file.getFileNameWithoutExtension();

        g.setColour(active ? text : juce::Colour::fromRGB(218, 224, 229));
        g.setFont(uiFont(12.0f, active));
        g.drawFittedText(displayName, row, juce::Justification::centredLeft, 1, 0.68f);

        auto meta = item.bank;
        if (item.category.isNotEmpty())
            meta << " / " << item.category;
        if (!item.tags.isEmpty())
        {
            juce::StringArray visibleTags;
            for (int index = 0; index < juce::jmin(2, item.tags.size()); ++index)
                visibleTags.add(item.tags[index]);
            meta << " / " << visibleTags.joinIntoString(", ");
        }

        g.setColour(mutedText);
        g.setFont(uiFont(10.5f));
        g.drawFittedText(meta, metaArea, juce::Justification::centredRight, 1, 0.58f);

        g.setColour(item.factory ? live : (item.sourceLabel == "Legacy User" ? staged : accent));
        g.setFont(uiFont(10.0f, true));
        g.drawFittedText(item.sourceLabel, sourceArea, juce::Justification::centredRight, 1, 0.62f);
    }

    void listBoxItemClicked(int row, const juce::MouseEvent& event) override
    {
        if (row < 0 || row >= getNumRows())
            return;

        const auto itemIndex = filteredItemIndices[static_cast<std::size_t>(row)];
        if (event.x < favoriteHitWidth)
            owner.togglePresetFavoriteAtIndex(itemIndex);
        else
            owner.loadPresetAtIndex(itemIndex);
    }

    void listBoxItemDoubleClicked(int row, const juce::MouseEvent& event) override
    {
        if (event.x < favoriteHitWidth)
            return;

        if (row >= 0 && row < getNumRows())
            owner.loadPresetAtIndex(filteredItemIndices[static_cast<std::size_t>(row)]);
    }

    bool isCurrentPreset(const SynthAudioProcessor::PresetListItem& item) const
    {
        if (currentPresetPath.isNotEmpty())
            return item.file.getFullPathName() == currentPresetPath;

        return currentPresetName.isNotEmpty() && item.displayName == currentPresetName;
    }

    SynthAudioProcessorEditor& owner;
    juce::TextEditor searchEditor;
    juce::ComboBox categoryBox;
    juce::TextButton factoryButton;
    juce::TextButton userButton;
    juce::TextButton legacyButton;
    juce::TextButton favoritesButton;
    juce::ListBox presetList;
    std::vector<SynthAudioProcessor::PresetListItem> allItems;
    std::vector<int> filteredItemIndices;
    juce::String currentPresetPath;
    juce::String currentPresetName;
    juce::String selectedCategory;
    bool rebuildingCategoryBox = false;
};

// ============================================================================
// MidiControllerPanel: global MIDI learn/forget surface for automatable params.
// ============================================================================
class SynthAudioProcessorEditor::MidiControllerPanel final : public LayoutSection
{
public:
    explicit MidiControllerPanel(SynthAudioProcessorEditor& editor)
        : owner(editor)
    {
        parameterBox.setTextWhenNothingSelected("Select parameter");
        int itemId = 1;
        for (const auto& spec : synth::getParameterSpecs())
        {
            if (!spec.automatable)
                continue;

            parameterIds.push_back(spec.id);
            parameterBox.addItem(juce::String(spec.group).toUpperCase() + " / " + spec.name, itemId++);
        }
        addAndMakeVisible(parameterBox);

        styleFlatButton(learnButton, accent.darker(0.18f), juce::Colour::fromRGB(12, 14, 16));
        styleFlatButton(forgetButton, juce::Colour::fromRGB(42, 48, 54), text);
        styleFlatButton(cancelButton, juce::Colour::fromRGB(42, 48, 54), mutedText);
        addAndMakeVisible(learnButton);
        addAndMakeVisible(forgetButton);
        addAndMakeVisible(cancelButton);

        learnButton.onClick = [this] {
            juce::String message;
            if (owner.audioProcessor.startMidiLearn(selectedParameterId(), message))
                refresh();
            owner.updateStatus(message);
        };
        forgetButton.onClick = [this] {
            juce::String message;
            owner.audioProcessor.forgetMidiControllerForParameter(selectedParameterId(), message);
            refresh();
            owner.updateStatus(message);
        };
        cancelButton.onClick = [this] {
            owner.audioProcessor.cancelMidiLearn();
            refresh();
            owner.updateStatus("MIDI learn canceled");
        };

        statusLabel.setColour(juce::Label::textColourId, mutedText);
        statusLabel.setFont(uiFont(11.0f));
        statusLabel.setMinimumHorizontalScale(0.65f);
        addAndMakeVisible(statusLabel);

        assignmentLabel.setColour(juce::Label::textColourId, text);
        assignmentLabel.setFont(uiFont(11.0f));
        assignmentLabel.setMinimumHorizontalScale(0.55f);
        addAndMakeVisible(assignmentLabel);

        refresh();
    }

    int preferredHeight(int width) const override
    {
        return width < 760 ? 142 : 112;
    }

    void refresh()
    {
        statusLabel.setText(owner.audioProcessor.getMidiControllerStatus(), juce::dontSendNotification);

        const auto assignments = owner.audioProcessor.getMidiControllerAssignments();
        juce::StringArray visible;
        for (int index = 0; index < juce::jmin(4, static_cast<int>(assignments.size())); ++index)
        {
            const auto& assignment = assignments[static_cast<std::size_t>(index)];
            visible.add("CC" + juce::String(assignment.controllerNumber) + " -> "
                        + juce::String(assignment.parameterId));
        }

        const auto hiddenAssignmentCount = static_cast<int>(assignments.size()) - visible.size();
        if (hiddenAssignmentCount > 0)
            visible.add("+" + juce::String(hiddenAssignmentCount) + " more");

        assignmentLabel.setText(visible.isEmpty() ? "No MIDI CC mappings" : visible.joinIntoString("   "),
                                juce::dontSendNotification);
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
        g.fillRect(headerArea.removeFromBottom(8));

        paintModuleHeaderTick(g, getLocalBounds().removeFromTop(headerHeight), zoneUtility);
        auto titleArea = getLocalBounds().removeFromTop(headerHeight).reduced(16, 0);
        auto badgeArea = titleArea.removeFromRight(70).withSizeKeepingCentre(70, 16);
        titleArea.removeFromRight(8);

        g.setColour(text);
        g.setFont(uiFont(12.0f, true));
        g.drawText("MIDI CONTROL", titleArea, juce::Justification::centredLeft, true);

        g.setColour(accent.withAlpha(0.18f));
        g.fillRoundedRectangle(badgeArea.toFloat(), 8.0f);
        g.setColour(accent);
        g.setFont(uiFont(10.0f, true));
        g.drawText("GLOBAL", badgeArea, juce::Justification::centred, false);
    }

    void resized() override
    {
        auto content = getLocalBounds();
        content.removeFromTop(headerHeight);
        content = content.reduced(12, 8);

        if (content.getWidth() < 760)
        {
            parameterBox.setBounds(content.removeFromTop(28));
            content.removeFromTop(6);

            auto buttons = content.removeFromTop(28);
            learnButton.setBounds(buttons.removeFromLeft(70));
            buttons.removeFromLeft(6);
            forgetButton.setBounds(buttons.removeFromLeft(72));
            buttons.removeFromLeft(6);
            cancelButton.setBounds(buttons.removeFromLeft(70));
        }
        else
        {
            auto controls = content.removeFromTop(28);
            parameterBox.setBounds(controls.removeFromLeft(360));
            controls.removeFromLeft(8);
            learnButton.setBounds(controls.removeFromLeft(70));
            controls.removeFromLeft(6);
            forgetButton.setBounds(controls.removeFromLeft(72));
            controls.removeFromLeft(6);
            cancelButton.setBounds(controls.removeFromLeft(70));
        }

        content.removeFromTop(5);
        statusLabel.setBounds(content.removeFromTop(18));
        assignmentLabel.setBounds(content.removeFromTop(18));
    }

private:
    juce::String selectedParameterId() const
    {
        const auto selectedIndex = parameterBox.getSelectedId() - 1;
        if (selectedIndex < 0 || selectedIndex >= static_cast<int>(parameterIds.size()))
            return {};

        return juce::String(parameterIds[static_cast<std::size_t>(selectedIndex)]);
    }

    static constexpr int headerHeight = 26;

    SynthAudioProcessorEditor& owner;
    juce::ComboBox parameterBox;
    juce::TextButton learnButton { "Learn" };
    juce::TextButton forgetButton { "Forget" };
    juce::TextButton cancelButton { "Cancel" };
    juce::Label statusLabel;
    juce::Label assignmentLabel;
    std::vector<std::string> parameterIds;
};

// ============================================================================
// ModulationOverviewPanel: read-only source and route inspection.
// ============================================================================
class SynthAudioProcessorEditor::ModulationOverviewPanel final : public LayoutSection
{
public:
    explicit ModulationOverviewPanel(SynthAudioProcessor& processor)
        : audioProcessor(processor)
    {
        refresh();
    }

    int preferredHeight(int width) const override
    {
        return headerHeight + padY * 2 + labelHeight * 2 + sectionGap
               + sourceRowsForWidth(width) * chipHeight
               + std::max(0, sourceRowsForWidth(width) - 1) * chipGap
               + routeRows * routeHeight + (routeRows - 1) * routeGap;
    }

    void refresh()
    {
        routeView = audioProcessor.getModulationRouteView();
        repaint();
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
        g.fillRect(headerArea.removeFromBottom(8));

        paintModuleHeaderTick(g, getLocalBounds().removeFromTop(headerHeight), zoneMod);
        auto titleArea = getLocalBounds().removeFromTop(headerHeight).reduced(16, 0);
        auto badgeArea = titleArea.removeFromRight(52).withSizeKeepingCentre(52, 16);
        titleArea.removeFromRight(8);

        g.setColour(text);
        g.setFont(uiFont(12.0f, true));
        g.drawText("MODULATION OVERVIEW", titleArea, juce::Justification::centredLeft, true);

        g.setColour(live.withAlpha(0.18f));
        g.fillRoundedRectangle(badgeArea.toFloat(), 8.0f);
        g.setColour(live);
        g.setFont(uiFont(10.0f, true));
        g.drawText("READ", badgeArea, juce::Justification::centred, false);

        auto content = getLocalBounds();
        content.removeFromTop(headerHeight);
        content = content.reduced(padX, padY);

        auto sourceLabelArea = content.removeFromTop(labelHeight);
        paintSectionLabel(g, sourceLabelArea, "SOURCES");
        const auto sourceRows = sourceRowsForWidth(getWidth());
        auto sourceArea = content.removeFromTop(sourceRows * chipHeight + std::max(0, sourceRows - 1) * chipGap);
        paintSources(g, sourceArea);

        content.removeFromTop(sectionGap);
        auto routeLabelArea = content.removeFromTop(labelHeight);
        const auto activeRouteCount = static_cast<int>(routeView.activeRoutes.size());
        paintSectionLabel(g, routeLabelArea,
                          activeRouteCount > 0 ? "ACTIVE ROUTES  (" + juce::String(activeRouteCount) + ")"
                                               : "ACTIVE ROUTES");
        paintRoutes(g, content.removeFromTop(routeRows * routeHeight + (routeRows - 1) * routeGap));
    }

private:
    static constexpr int headerHeight = 26;
    static constexpr int padX = 12;
    static constexpr int padY = 8;
    static constexpr int labelHeight = 15;
    static constexpr int sectionGap = 9;
    static constexpr int chipHeight = 22;
    static constexpr int chipGap = 6;
    static constexpr int routeHeight = 24;
    static constexpr int routeGap = 6;
    static constexpr int routeRows = 2;
    static constexpr int minChipWidth = 92;
    static constexpr int minRouteWidth = 226;
    static constexpr std::size_t sourceCount = static_cast<std::size_t>(synth::ModSource::Macro4) + 1;

    int sourceColumnsForWidth(int width) const
    {
        const auto usableWidth = std::max(1, width - padX * 2);
        return std::max(2, usableWidth / (minChipWidth + chipGap));
    }

    int sourceRowsForWidth(int width) const
    {
        int visibleSources = 0;
        for (const auto& source : synth::modulationSourceCatalog())
        {
            if (source.source != synth::ModSource::None)
                ++visibleSources;
        }

        const auto columns = sourceColumnsForWidth(width);
        return std::max(1, (visibleSources + columns - 1) / columns);
    }

    int routeColumnsForWidth(int width) const
    {
        const auto usableWidth = std::max(1, width - padX * 2);
        return std::max(1, usableWidth / (minRouteWidth + routeGap));
    }

    void incrementSourceUseCount(std::array<int, sourceCount>& counts, synth::ModSource source) const
    {
        const auto sourceIndex = static_cast<int>(source);
        if (sourceIndex >= 0 && sourceIndex < static_cast<int>(counts.size()))
            ++counts[static_cast<std::size_t>(sourceIndex)];
    }

    std::array<int, sourceCount> sourceUseCounts() const
    {
        std::array<int, sourceCount> counts {};
        for (const auto& route : routeView.activeRoutes)
        {
            incrementSourceUseCount(counts, route.source);
            if (route.scaler != synth::ModSource::None && route.scaler != route.source)
                incrementSourceUseCount(counts, route.scaler);
        }
        return counts;
    }

    void paintSectionLabel(juce::Graphics& g, juce::Rectangle<int> area, const juce::String& label) const
    {
        g.setColour(mutedText);
        g.setFont(uiFont(10.0f, true));
        g.drawText(label, area, juce::Justification::centredLeft, true);
    }

    void paintSources(juce::Graphics& g, juce::Rectangle<int> area) const
    {
        const auto counts = sourceUseCounts();
        const auto columns = sourceColumnsForWidth(getWidth());
        const auto chipWidth = (area.getWidth() - (columns - 1) * chipGap) / columns;

        int visibleIndex = 0;
        for (const auto& source : synth::modulationSourceCatalog())
        {
            if (source.source == synth::ModSource::None)
                continue;

            const auto column = visibleIndex % columns;
            const auto row = visibleIndex / columns;
            auto chip = juce::Rectangle<int>(
                area.getX() + column * (chipWidth + chipGap),
                area.getY() + row * (chipHeight + chipGap),
                chipWidth,
                chipHeight);

            const auto sourceIndex = static_cast<int>(source.source);
            const auto routeCount = sourceIndex >= 0 && sourceIndex < static_cast<int>(counts.size())
                ? counts[static_cast<std::size_t>(sourceIndex)]
                : 0;

            const auto activeFill = routeCount > 0 ? accent.withAlpha(0.16f) : fieldBg;
            const auto outline = routeCount > 0 ? accent.withAlpha(0.72f) : strokeSoft;
            g.setColour(activeFill);
            g.fillRoundedRectangle(chip.toFloat(), 5.0f);
            g.setColour(outline);
            g.drawRoundedRectangle(chip.toFloat().reduced(0.5f), 5.0f, 1.0f);

            auto labelArea = chip.reduced(7, 0);
            if (routeCount > 0)
            {
                auto countArea = labelArea.removeFromRight(22);
                g.setColour(accent.withAlpha(0.22f));
                g.fillRoundedRectangle(countArea.toFloat().withSizeKeepingCentre(18.0f, 14.0f), 7.0f);
                g.setColour(accent);
                g.setFont(uiFont(10.0f, true));
                g.drawText(juce::String(routeCount), countArea, juce::Justification::centred, false);
                labelArea.removeFromRight(3);
            }

            g.setColour(routeCount > 0 ? text : mutedText);
            g.setFont(uiFont(10.5f, routeCount > 0));
            g.drawFittedText(juce::String(source.label), labelArea, juce::Justification::centredLeft, 1, 0.72f);

            ++visibleIndex;
        }
    }

    juce::String sourceLabel(synth::ModSource source) const
    {
        if (const auto* info = synth::findModulationSourceInfo(source))
            return juce::String(info->label);
        return "None";
    }

    juce::String destinationLabel(synth::ModulationDestination destination) const
    {
        if (const auto* info = synth::findModulationDestinationInfo(destination))
            return juce::String(info->label);
        return "Target";
    }

    juce::String signedNumber(float value, int decimals) const
    {
        return juce::String(value > 0.0f ? "+" : "") + juce::String(value, decimals);
    }

    juce::String depthLabel(const synth::ModulationRouteSummary& route) const
    {
        const auto* destination = synth::findModulationDestinationInfo(route.destination);
        if (destination == nullptr)
            return signedNumber(route.depth, 2);

        if (destination->unit == "semitones")
            return signedNumber(route.depth, 1) + " st";
        if (destination->unit == "dB")
            return signedNumber(route.depth, 1) + " dB";
        if (destination->unit == "normalized" || destination->unit == "percent")
            return signedNumber(route.depth * 100.0f, 0) + "%";

        return signedNumber(route.depth, 2);
    }

    juce::String routeLabel(const synth::ModulationRouteSummary& route) const
    {
        auto label = sourceLabel(route.source) + " -> " + destinationLabel(route.destination);
        if (route.scaler != synth::ModSource::None)
            label += " x " + sourceLabel(route.scaler);
        label += " " + depthLabel(route);
        return label;
    }

    void paintRoutes(juce::Graphics& g, juce::Rectangle<int> area) const
    {
        const auto columns = routeColumnsForWidth(getWidth());
        const auto routeWidth = (area.getWidth() - (columns - 1) * routeGap) / columns;
        const auto visibleCapacity = std::max(1, columns * routeRows);

        if (routeView.activeRoutes.empty())
        {
            auto emptyArea = area.removeFromTop(routeHeight);
            g.setColour(fieldBg);
            g.fillRoundedRectangle(emptyArea.toFloat(), 5.0f);
            g.setColour(strokeSoft);
            g.drawRoundedRectangle(emptyArea.toFloat().reduced(0.5f), 5.0f, 1.0f);
            g.setColour(mutedText);
            g.setFont(uiFont(10.5f, true));
            g.drawText("NO ACTIVE ROUTES", emptyArea.reduced(8, 0), juce::Justification::centredLeft, true);
            return;
        }

        const auto routeCount = static_cast<int>(routeView.activeRoutes.size());
        const auto paintCount = std::min(routeCount, visibleCapacity);
        const auto needsOverflowPill = routeCount > visibleCapacity;
        const auto routePills = needsOverflowPill ? std::max(0, paintCount - 1) : paintCount;

        for (int index = 0; index < routePills; ++index)
            paintRoutePill(g, area, columns, routeWidth, index, routeLabel(routeView.activeRoutes[static_cast<std::size_t>(index)]));

        if (needsOverflowPill)
            paintRoutePill(g, area, columns, routeWidth, visibleCapacity - 1,
                           "+" + juce::String(routeCount - routePills) + " MORE");
    }

    void paintRoutePill(juce::Graphics& g,
                        juce::Rectangle<int> area,
                        int columns,
                        int routeWidth,
                        int index,
                        const juce::String& label) const
    {
        const auto column = index % columns;
        const auto row = index / columns;
        auto routeArea = juce::Rectangle<int>(
            area.getX() + column * (routeWidth + routeGap),
            area.getY() + row * (routeHeight + routeGap),
            routeWidth,
            routeHeight);

        g.setColour(accent.withAlpha(0.12f));
        g.fillRoundedRectangle(routeArea.toFloat(), 5.0f);
        g.setColour(accent.withAlpha(0.62f));
        g.drawRoundedRectangle(routeArea.toFloat().reduced(0.5f), 5.0f, 1.0f);
        g.setColour(text);
        g.setFont(uiFont(10.5f, true));
        g.drawFittedText(label, routeArea.reduced(8, 0), juce::Justification::centredLeft, 1, 0.64f);
    }

    SynthAudioProcessor& audioProcessor;
    synth::ModulationRouteView routeView;
};

// ============================================================================
// SequencerPanel: dense real controls for arp steps and chord voices.
// ============================================================================
class SynthAudioProcessorEditor::SequencerPanel final : public LayoutSection
{
public:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    explicit SequencerPanel(juce::AudioProcessorValueTreeState& state)
        : valueTreeState(state)
    {
        for (const auto* id : {
                 "arp.enabled", "arp.mode", "arp.rate", "arp.gate",
                 "arp.octaves", "arp.hold", "arp.swing", "arp.step_count"
             })
        {
            if (const auto* spec = synth::findParameterSpec(id))
            {
                auto control = std::make_unique<ParameterControl>(valueTreeState, *spec, "Arp ");
                addAndMakeVisible(*control);
                topControls.push_back(std::move(control));
            }
        }

        for (int step = 1; step <= synth::arpStepCount; ++step)
        {
            const auto index = static_cast<std::size_t>(step - 1);
            const auto prefix = "arp.step." + std::to_string(step) + ".";
            configureToggle(steps[index].enabled, prefix + "enabled", "Step " + juce::String(step) + " on");
            configureSlider(steps[index].pitch, prefix + "pitch_semitones", "Step " + juce::String(step) + " pitch");
            configureSlider(steps[index].velocity, prefix + "velocity", "Step " + juce::String(step) + " velocity");
            configureSlider(steps[index].gate, prefix + "gate", "Step " + juce::String(step) + " gate");
            configureToggle(steps[index].tie, prefix + "tie", "Step " + juce::String(step) + " tie");
        }

        for (int voice = 1; voice <= synth::chordVoiceCount; ++voice)
        {
            const auto index = static_cast<std::size_t>(voice - 1);
            const auto prefix = "chord.voice." + std::to_string(voice) + ".";
            configureToggle(chordVoices[index].enabled, prefix + "enabled", "Chord voice " + juce::String(voice) + " on");
            configureSlider(chordVoices[index].pitch, prefix + "pitch_semitones", "Chord voice " + juce::String(voice) + " pitch");
            configureSlider(chordVoices[index].velocity, prefix + "velocity", "Chord voice " + juce::String(voice) + " velocity");
        }

        // Do not strip the "Chord " prefix here: these share the arp top-control flow, so
        // they must read "Chord Enabled" / "Chord Voice Count" to avoid colliding with the
        // arp "Enabled" toggle that sits a few cells to the left.
        if (const auto* spec = synth::findParameterSpec("chord.enabled"))
            chordTopControls.push_back(makeTopControl(*spec, {}));
        if (const auto* spec = synth::findParameterSpec("chord.voice_count"))
            chordTopControls.push_back(makeTopControl(*spec, {}));
    }

    int preferredHeight(int width) const override
    {
        return computeLayoutHeight(width);
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
        g.fillRect(headerArea.removeFromBottom(8));

        paintModuleHeaderTick(g, getLocalBounds().removeFromTop(headerHeight), zoneSource);
        auto titleArea = getLocalBounds().removeFromTop(headerHeight).reduced(16, 0);
        auto badgeArea = titleArea.removeFromRight(54).withSizeKeepingCentre(54, 16);
        titleArea.removeFromRight(8);

        g.setColour(text);
        g.setFont(uiFont(12.0f, true));
        g.drawText("ARP / STEP / CHORD", titleArea, juce::Justification::centredLeft, true);

        g.setColour(live.withAlpha(0.18f));
        g.fillRoundedRectangle(badgeArea.toFloat(), 8.0f);
        g.setColour(live);
        g.setFont(uiFont(10.0f, true));
        g.drawText("LIVE", badgeArea, juce::Justification::centred, false);

        paintGridLabels(g);
    }

    void resized() override
    {
        layoutControls(getWidth());
    }

private:
    struct GridSlider
    {
        juce::Slider slider;
        std::unique_ptr<SliderAttachment> attachment;
    };

    struct GridToggle
    {
        juce::ToggleButton toggle;
        std::unique_ptr<ButtonAttachment> attachment;
    };

    struct StepControls
    {
        GridToggle enabled;
        GridSlider pitch;
        GridSlider velocity;
        GridSlider gate;
        GridToggle tie;
    };

    struct ChordVoiceControls
    {
        GridToggle enabled;
        GridSlider pitch;
        GridSlider velocity;
    };

    std::unique_ptr<ParameterControl> makeTopControl(const synth::ParameterSpec& spec,
                                                     const juce::String& stripPrefix)
    {
        auto control = std::make_unique<ParameterControl>(valueTreeState, spec, stripPrefix);
        addAndMakeVisible(*control);
        return control;
    }

    void configureToggle(GridToggle& control, const std::string& id, const juce::String& tooltip)
    {
        if (synth::findParameterSpec(id) == nullptr)
            return;

        control.toggle.setClickingTogglesState(true);
        control.toggle.setTooltip(tooltip);
        control.attachment = std::make_unique<ButtonAttachment>(valueTreeState, id, control.toggle);
        addAndMakeVisible(control.toggle);
    }

    void configureSlider(GridSlider& control, const std::string& id, const juce::String& tooltip)
    {
        if (const auto* found = synth::findParameterSpec(id))
        {
            const auto spec = *found;
            control.slider.setSliderStyle(juce::Slider::LinearBar);
            control.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 48, 14);
            control.slider.setColour(juce::Slider::backgroundColourId, fieldBg);
            control.slider.setColour(juce::Slider::trackColourId, accent.withAlpha(0.72f));
            control.slider.setColour(juce::Slider::textBoxTextColourId, text);
            control.slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
            control.slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
            control.slider.setTooltip(tooltip);
            control.slider.setNumDecimalPlacesToDisplay(0);
            control.attachment = std::make_unique<SliderAttachment>(valueTreeState, id, control.slider);
            control.slider.textFromValueFunction = [spec](double value) { return formatGridValue(spec, value); };
            control.slider.valueFromTextFunction = [spec](const juce::String& textValue) {
                return parseValueText(spec, textValue);
            };
            control.slider.updateText();
            addAndMakeVisible(control.slider);
        }
    }

    int computeTopControlsLayout(juce::Rectangle<int> bounds, bool apply)
    {
        const auto columns = std::max(1, (bounds.getWidth() + controlGap) / (topControlWidth + controlGap));
        const auto columnWidth = static_cast<float>(bounds.getWidth() - (columns - 1) * controlGap)
                                 / static_cast<float>(columns);

        int column = 0;
        int row = 0;
        const auto placeControl = [&](ParameterControl& control) {
            const auto span = std::min(control.isWide() ? 2 : 1, columns);
            if (column + span > columns)
            {
                column = 0;
                ++row;
            }

            if (apply)
            {
                const auto x = bounds.getX() + static_cast<int>(std::round(static_cast<float>(column)
                                                                           * (columnWidth + static_cast<float>(controlGap))));
                const auto width = static_cast<int>(std::round(static_cast<float>(span) * columnWidth
                                                              + static_cast<float>((span - 1) * controlGap)));
                const auto y = bounds.getY() + row * (topControlHeight + rowGap);
                control.setBounds(x, y, width, topControlHeight);
            }

            column += span;
        };

        for (const auto& control : topControls)
            placeControl(*control);
        for (const auto& control : chordTopControls)
            placeControl(*control);

        return (row + 1) * topControlHeight + row * rowGap;
    }

    void layoutStepGrid(juce::Rectangle<int> bounds)
    {
        const auto stepWidth = std::max(28, (bounds.getWidth() - rowLabelWidth - (synth::arpStepCount - 1) * cellGap)
                                            / synth::arpStepCount);
        const auto gridWidth = rowLabelWidth + synth::arpStepCount * stepWidth + (synth::arpStepCount - 1) * cellGap;
        const auto xStart = bounds.getX() + std::max(0, (bounds.getWidth() - gridWidth) / 2);
        auto y = bounds.getY();

        for (int step = 0; step < synth::arpStepCount; ++step)
        {
            const auto x = xStart + rowLabelWidth + step * (stepWidth + cellGap);
            const auto index = static_cast<std::size_t>(step);
            steps[index].enabled.toggle.setBounds(x, y + headerRowHeight, stepWidth, toggleRowHeight);
            steps[index].pitch.slider.setBounds(x, y + headerRowHeight + toggleRowHeight + rowGap,
                                                stepWidth, sliderRowHeight);
            steps[index].velocity.slider.setBounds(x, y + headerRowHeight + toggleRowHeight + sliderRowHeight + 2 * rowGap,
                                                   stepWidth, sliderRowHeight);
            steps[index].gate.slider.setBounds(x, y + headerRowHeight + toggleRowHeight + 2 * sliderRowHeight + 3 * rowGap,
                                               stepWidth, sliderRowHeight);
            steps[index].tie.toggle.setBounds(x, y + headerRowHeight + toggleRowHeight + 3 * sliderRowHeight + 4 * rowGap,
                                              stepWidth, toggleRowHeight);
        }
    }

    void layoutChordGrid(juce::Rectangle<int> bounds)
    {
        const auto voiceWidth = std::max(48, (bounds.getWidth() - rowLabelWidth - (synth::chordVoiceCount - 1) * cellGap)
                                             / synth::chordVoiceCount);
        const auto gridWidth = rowLabelWidth + synth::chordVoiceCount * voiceWidth + (synth::chordVoiceCount - 1) * cellGap;
        const auto xStart = bounds.getX() + std::max(0, (bounds.getWidth() - gridWidth) / 2);
        auto y = bounds.getY();

        for (int voice = 0; voice < synth::chordVoiceCount; ++voice)
        {
            const auto x = xStart + rowLabelWidth + voice * (voiceWidth + cellGap);
            const auto index = static_cast<std::size_t>(voice);
            chordVoices[index].enabled.toggle.setBounds(x, y + headerRowHeight, voiceWidth, toggleRowHeight);
            chordVoices[index].pitch.slider.setBounds(x, y + headerRowHeight + toggleRowHeight + rowGap,
                                                      voiceWidth, sliderRowHeight);
            chordVoices[index].velocity.slider.setBounds(x, y + headerRowHeight + toggleRowHeight + sliderRowHeight + 2 * rowGap,
                                                         voiceWidth, sliderRowHeight);
        }
    }

    int computeLayoutHeight(int width) const
    {
        if (width <= 0)
            return headerHeight + padTop + topControlHeight + sectionGap + stepGridHeight + sectionGap
                   + chordGridHeight + padBottom;

        auto bounds = juce::Rectangle<int>(0, 0, width, 10000).reduced(padX, 0);
        auto y = headerHeight + padTop;
        y += computeTopControlsHeight(bounds.withY(y).withHeight(topControlHeight * 2)) + sectionGap;
        y += stepGridHeight + sectionGap;
        y += chordGridHeight + padBottom;
        return y;
    }

    void layoutControls(int width)
    {
        if (width <= 0)
            return;

        auto bounds = juce::Rectangle<int>(0, 0, width, 10000).reduced(padX, 0);
        auto y = headerHeight + padTop;
        y += computeTopControlsLayout(bounds.withY(y).withHeight(topControlHeight * 2), true) + sectionGap;
        layoutStepGrid(bounds.withY(y).withHeight(stepGridHeight));
        y += stepGridHeight + sectionGap;
        layoutChordGrid(bounds.withY(y).withHeight(chordGridHeight));
    }

    void paintGridLabels(juce::Graphics& g) const
    {
        auto bounds = getLocalBounds().reduced(padX, 0);
        auto y = headerHeight + padTop;
        y += computeTopControlsHeight(bounds.withY(y).withHeight(topControlHeight * 2)) + sectionGap;
        paintStepLabels(g, bounds.withY(y).withHeight(stepGridHeight));
        y += stepGridHeight + sectionGap;
        paintChordLabels(g, bounds.withY(y).withHeight(chordGridHeight));
    }

    int computeTopControlsHeight(juce::Rectangle<int> bounds) const
    {
        const auto columns = std::max(1, (bounds.getWidth() + controlGap) / (topControlWidth + controlGap));
        int column = 0;
        int row = 0;
        auto countControl = [&column, &row, columns](const ParameterControl& control) {
            const auto span = std::min(control.isWide() ? 2 : 1, columns);
            if (column + span > columns)
            {
                column = 0;
                ++row;
            }
            column += span;
        };

        for (const auto& control : topControls)
            countControl(*control);
        for (const auto& control : chordTopControls)
            countControl(*control);

        return (row + 1) * topControlHeight + row * rowGap;
    }

    void paintStepLabels(juce::Graphics& g, juce::Rectangle<int> bounds) const
    {
        paintGridFrame(g, bounds);

        const auto stepWidth = std::max(28, (bounds.getWidth() - rowLabelWidth - (synth::arpStepCount - 1) * cellGap)
                                            / synth::arpStepCount);
        const auto gridWidth = rowLabelWidth + synth::arpStepCount * stepWidth + (synth::arpStepCount - 1) * cellGap;
        const auto xStart = bounds.getX() + std::max(0, (bounds.getWidth() - gridWidth) / 2);
        auto y = bounds.getY();

        g.setColour(mutedText);
        g.setFont(uiFont(10.5f, true));
        g.drawText("STEP", xStart, y, rowLabelWidth, headerRowHeight, juce::Justification::centredLeft, false);
        for (int step = 0; step < synth::arpStepCount; ++step)
        {
            const auto x = xStart + rowLabelWidth + step * (stepWidth + cellGap);
            g.drawText(juce::String(step + 1), x, y, stepWidth, headerRowHeight, juce::Justification::centred, false);
        }

        y += headerRowHeight;
        paintRowLabel(g, xStart, y, "On");
        y += toggleRowHeight + rowGap;
        paintRowLabel(g, xStart, y, "Pitch");
        y += sliderRowHeight + rowGap;
        paintRowLabel(g, xStart, y, "Vel");
        y += sliderRowHeight + rowGap;
        paintRowLabel(g, xStart, y, "Gate");
        y += sliderRowHeight + rowGap;
        paintRowLabel(g, xStart, y, "Tie");
    }

    void paintChordLabels(juce::Graphics& g, juce::Rectangle<int> bounds) const
    {
        paintGridFrame(g, bounds);

        const auto voiceWidth = std::max(48, (bounds.getWidth() - rowLabelWidth - (synth::chordVoiceCount - 1) * cellGap)
                                             / synth::chordVoiceCount);
        const auto gridWidth = rowLabelWidth + synth::chordVoiceCount * voiceWidth + (synth::chordVoiceCount - 1) * cellGap;
        const auto xStart = bounds.getX() + std::max(0, (bounds.getWidth() - gridWidth) / 2);
        auto y = bounds.getY();

        g.setColour(mutedText);
        g.setFont(uiFont(10.5f, true));
        g.drawText("CHORD", xStart, y, rowLabelWidth, headerRowHeight, juce::Justification::centredLeft, false);
        for (int voice = 0; voice < synth::chordVoiceCount; ++voice)
        {
            const auto x = xStart + rowLabelWidth + voice * (voiceWidth + cellGap);
            g.drawText(juce::String(voice + 1), x, y, voiceWidth, headerRowHeight, juce::Justification::centred, false);
        }

        y += headerRowHeight;
        paintRowLabel(g, xStart, y, "On");
        y += toggleRowHeight + rowGap;
        paintRowLabel(g, xStart, y, "Pitch");
        y += sliderRowHeight + rowGap;
        paintRowLabel(g, xStart, y, "Vel");
    }

    void paintRowLabel(juce::Graphics& g, int x, int y, const juce::String& label) const
    {
        g.setColour(mutedText);
        g.setFont(uiFont(10.0f, true));
        g.drawText(label, x, y, rowLabelWidth - 6, sliderRowHeight, juce::Justification::centredLeft, false);
    }

    void paintGridFrame(juce::Graphics& g, juce::Rectangle<int> bounds) const
    {
        const auto frame = bounds.toFloat().reduced(0.5f);
        g.setColour(juce::Colour::fromRGB(23, 27, 31));
        g.fillRoundedRectangle(frame, 4.0f);
        g.setColour(strokeSoft.withAlpha(0.72f));
        g.drawRoundedRectangle(frame, 4.0f, 1.0f);
    }

    juce::AudioProcessorValueTreeState& valueTreeState;
    std::vector<std::unique_ptr<ParameterControl>> topControls;
    std::vector<std::unique_ptr<ParameterControl>> chordTopControls;
    std::array<StepControls, synth::arpStepCount> steps;
    std::array<ChordVoiceControls, synth::chordVoiceCount> chordVoices;

    static constexpr int headerHeight = 26;
    static constexpr int padX = 12;
    static constexpr int padTop = 6;
    static constexpr int padBottom = 10;
    static constexpr int controlGap = 8;
    static constexpr int cellGap = 4;
    static constexpr int rowGap = 4;
    static constexpr int sectionGap = 12;
    static constexpr int topControlWidth = 66;
    static constexpr int topControlHeight = 58;
    static constexpr int rowLabelWidth = 42;
    static constexpr int headerRowHeight = 18;
    static constexpr int toggleRowHeight = 26;
    static constexpr int sliderRowHeight = 40;
    static constexpr int stepGridHeight = headerRowHeight + 2 * toggleRowHeight + 3 * sliderRowHeight + 4 * rowGap;
    static constexpr int chordGridHeight = headerRowHeight + toggleRowHeight + 2 * sliderRowHeight + 2 * rowGap;
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

juce::String nonEmptyPresetField(const juce::String& value, const juce::String& fallback)
{
    const auto trimmed = value.trim();
    return trimmed.isNotEmpty() ? trimmed : fallback;
}

juce::String presetMenuSection(const SynthAudioProcessor::PresetListItem& item)
{
    return nonEmptyPresetField(item.sourceLabel, "Preset")
           + " / " + nonEmptyPresetField(item.bank, "Unbanked");
}

juce::String presetMenuLabel(const SynthAudioProcessor::PresetListItem& item)
{
    auto name = nonEmptyPresetField(item.displayName, item.file.getFileNameWithoutExtension());
    auto label = nonEmptyPresetField(item.sourceLabel, "Preset")
                 + " / " + nonEmptyPresetField(item.category, "Uncategorized")
                 + " - " + name;

    if (item.favorite)
        label = "FAV  " + label;

    if (!item.tags.isEmpty())
    {
        juce::StringArray visibleTags;
        for (int i = 0; i < juce::jmin(2, item.tags.size()); ++i)
            visibleTags.add(item.tags[i]);

        label << "  [" << visibleTags.joinIntoString(", ") << "]";
    }

    return label;
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
    titleLabel.setText("SYLENTH-AI", juce::dontSendNotification);
    titleLabel.setColour(juce::Label::textColourId, text);
    titleLabel.setFont(uiFont(18.0f, true));
    addAndMakeVisible(titleLabel);

    engineTag.setText("CORE OSC ENGINE", juce::dontSendNotification);
    engineTag.setColour(juce::Label::textColourId, live);
    engineTag.setFont(uiFont(10.0f, true));
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
    juce::String stripPrefix,
    juce::String enabledParamId)
{
    auto panel = std::make_unique<Panel>(audioProcessor.getValueTreeState(), std::move(title),
                                         ids, std::move(badge), badgeColour, stripPrefix,
                                         enabledParamId);
    auto* raw = panel.get();
    page.addAndMakeVisible(*raw);
    store.push_back(std::move(panel));
    return raw;
}

void SynthAudioProcessorEditor::buildPages()
{
    // ---- SOUND: the live core sound-design surface ------------------------
    presetBrowserPanel = std::make_unique<PresetBrowserPanel>(*this);
    soundPage.addAndMakeVisible(*presetBrowserPanel);
    midiControllerPanel = std::make_unique<MidiControllerPanel>(*this);
    soundPage.addAndMakeVisible(*midiControllerPanel);

    // Legacy flat osc.* is the audible tone source that Osc A1's slot gates and mixes.
    // The honest title makes that A1 relationship clear instead of a second "Oscillator".
    coreOscPanel = addPanel(soundPage, soundPanels, "Osc A1 Tone", {
        "osc.pitch_semitones", "osc.fine_cents", "osc.stack_count", "osc.stack_detune",
        "osc.saw_level", "osc.pulse_level", "osc.pulse_width", "osc.noise_level",
        "osc.sub_wave", "osc.sub_octave", "osc.sub_level", "osc.sub_pulse_width",
        "osc.sync_amount", "osc.phase_reset"
    }, "LIVE", zoneSource);

    filterPanel = addPanel(soundPage, soundPanels, "Filter", {
        "filter.enabled", "filter.mode", "filter.cutoff_semitones", "filter.resonance",
        "filter.drive", "filter.keytrack", "filter.oversampling"
    }, {}, zoneShape);

    ampEnvPanel = addPanel(soundPage, soundPanels, "Amp Envelope", {
        "amp_env.attack_ms", "amp_env.decay_ms", "amp_env.sustain", "amp_env.release_ms"
    }, {}, zoneShape);

    modEnvPanel = addPanel(soundPage, soundPanels, "Mod Envelope", {
        "mod_env.attack_ms", "mod_env.decay_ms", "mod_env.sustain", "mod_env.release_ms"
    }, {}, zoneShape);

    lfoPanel = addPanel(soundPage, soundPanels, "LFO", {
        "lfo.shape", "lfo.rate_mode", "lfo.rate_hz", "lfo.sync_division",
        "lfo.phase_degrees", "lfo.gate_mode", "lfo.mono", "lfo.swing"
    }, {}, zoneShape);

    voicePanel = addPanel(soundPage, soundPanels, "Voice", {
        "voice.mode", "voice.polyphony", "voice.unison_count", "voice.retrigger",
        "voice.glide_ms", "voice.velocity_glide_ms"
    }, {}, zonePerform);

    ampPanel = addPanel(soundPage, soundPanels, "Amp / Stereo", {
        "amp.drive", "amp.level_db", "amp.pan", "amp.pan_spread",
        "amp.unison_spread", "amp.analog"
    }, {}, zonePerform);

    rampPanel = addPanel(soundPage, soundPanels, "Ramp", {
        "ramp.enabled", "ramp.mode", "ramp.delay_ms", "ramp.rise_ms", "ramp.curve"
    }, {}, zonePerform);

    sequencerPanel = std::make_unique<SequencerPanel>(audioProcessor.getValueTreeState());
    soundPage.addAndMakeVisible(*sequencerPanel);

    macroPanel = addPanel(soundPage, soundPanels, "Macros", {
        "macro.motion", "macro.width", "macro.drive", "macro.space"
    }, {}, zonePerform);

    // ---- MOD: read-only route overview, direct routes, and eight TransMod slots
    modulationOverviewPanel = std::make_unique<ModulationOverviewPanel>(audioProcessor);
    modPage.addAndMakeVisible(*modulationOverviewPanel);

    oscPitchModPanel = addPanel(modPage, modPanels, "Osc Pitch Mod", {
        "direct.osc_keytrack_semitones", "direct.osc_lfo_semitones", "direct.osc_mod_env_semitones"
    }, {}, zoneMod, "Osc ");
    pulseWidthModPanel = addPanel(modPage, modPanels, "Pulse Width Mod", {
        "direct.pulse_keytrack", "direct.pulse_lfo", "direct.pulse_mod_env"
    }, {}, zoneMod, "Pulse ");
    filterCutoffModPanel = addPanel(modPage, modPanels, "Filter Cutoff Mod", {
        "direct.filter_keytrack", "direct.filter_lfo_semitones", "direct.filter_mod_env_semitones"
    }, {}, zoneMod, "Filter ");

    for (int slot = 1; slot <= synth::transModSlotCount; ++slot)
    {
        const auto prefix = "transmod." + std::to_string(slot) + ".";
        const auto title = "TransMod " + juce::String(slot);
        transModPanels[static_cast<std::size_t>(slot - 1)] = addPanel(
            modPage, modPanels, title, {
                prefix + "enabled", prefix + "source", prefix + "scaler",
                prefix + "osc_pitch_semitones", prefix + "pulse_width",
                prefix + "filter_cutoff_semitones", prefix + "amp_level_db", prefix + "pan"
            }, {}, zoneMod, title + " ", prefix + "enabled");
    }

    // ---- FX: a post-voice rack grouped by module, plus master/quality -----
    // Each module passes its enable param so the header power dot/dim reflects bypass.
    saturationPanel = addPanel(fxPage, fxPanels, "01 Saturation", {
        "fx.saturation_enabled", "fx.distortion_mode", "fx.saturation_mix", "fx.saturation_drive"
    }, "DRIVE", staged, "Saturation ", "fx.saturation_enabled");
    phaserPanel = addPanel(fxPage, fxPanels, "02 Phaser", {
        "fx.phaser_enabled", "fx.phaser_mix", "fx.phaser_rate_hz",
        "fx.phaser_depth", "fx.phaser_feedback"
    }, "PHASE", accent, "Phaser ", "fx.phaser_enabled");
    chorusPanel = addPanel(fxPage, fxPanels, "03 Chorus", {
        "fx.chorus_enabled", "fx.chorus_mix", "fx.chorus_rate_hz", "fx.chorus_depth_ms"
    }, "MOD", accent, "Chorus ", "fx.chorus_enabled");
    eqPanel = addPanel(fxPage, fxPanels, "04 EQ", {
        "fx.eq_enabled", "fx.eq_low_gain_db", "fx.eq_high_gain_db"
    }, "TONE", live, "EQ ", "fx.eq_enabled");
    delayPanel = addPanel(fxPage, fxPanels, "05 Delay", {
        "fx.delay_enabled", "fx.delay_mix", "fx.delay_sync_division", "fx.delay_feedback"
    }, "ECHO", accent, "Delay ", "fx.delay_enabled");
    reverbPanel = addPanel(fxPage, fxPanels, "06 Reverb", {
        "fx.reverb_enabled", "fx.reverb_mix", "fx.reverb_decay"
    }, "SPACE", accent, "Reverb ", "fx.reverb_enabled");
    compressorPanel = addPanel(fxPage, fxPanels, "07 Compressor", {
        "fx.compressor_enabled", "fx.compressor_threshold_db", "fx.compressor_ratio",
        "fx.compressor_makeup_db", "fx.compressor_mix"
    }, "DYN", staged, "Compressor ", "fx.compressor_enabled");
    masterFxPanel = addPanel(fxPage, fxPanels, "08 Master / Quality", {
        "fx.enabled", "quality.realtime_mode", "quality.offline_mode"
    }, "HOST", live, {}, "fx.enabled");
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

    // Render-boundary truth: layer mix and four slot outputs are audible, while A1
    // still uses the legacy flat osc.* controls as its compatibility source.
    layerStatusPill.setText(selectedLayer == 0 ? "LIVE - compat path" : "LIVE - layer mix",
                            juce::dontSendNotification);
    layerStatusPill.setColour(juce::Label::textColourId, live);

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
    // layer.N.osc.M.* state. A1 gates and mixes the legacy osc.* compatibility source;
    // the other slots render through the current oscillator stack foundation.
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
        }, "LIVE", live, slotStrip);
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

    if (page == Page::Mod && modulationOverviewPanel != nullptr)
        modulationOverviewPanel->refresh();

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

    // Selected-part accent: underline the active Layer button so the chosen part reads at a
    // glance, the way Sylenth's Part Select highlights the live part.
    const auto& activeLayerButton = selectedLayer == 0 ? layerAButton : layerBButton;
    if (!activeLayerButton.getBounds().isEmpty())
    {
        g.setColour(accent);
        g.fillRect(juce::Rectangle<int>(activeLayerButton.getX(), activeLayerButton.getBottom() + 3,
                                        activeLayerButton.getWidth(), 2));
    }

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

    auto left = area.removeFromLeft(132);
    left = left.withSizeKeepingCentre(left.getWidth(), 40);
    titleLabel.setBounds(left.removeFromTop(24));
    engineTag.setBounds(left.removeFromTop(14));

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
    const auto compactPresetNav = nav.getWidth() < 650;
    duplicateButton.setVisible(!compactPresetNav);
    nameEditor.setVisible(!compactPresetNav);

    prevPresetButton.setBounds(centreInHeight(nav.removeFromLeft(30), 30));
    nav.removeFromLeft(4);

    if (!compactPresetNav)
    {
        duplicateButton.setBounds(centreInHeight(nav.removeFromRight(88), 30));
        nav.removeFromRight(6);
    }
    else
    {
        duplicateButton.setBounds({});
    }

    saveButton.setBounds(centreInHeight(nav.removeFromRight(74), 30));
    nav.removeFromRight(6);
    loadButton.setBounds(centreInHeight(nav.removeFromRight(58), 30));
    nav.removeFromRight(compactPresetNav ? 8 : 10);

    if (!compactPresetNav)
    {
        nameEditor.setBounds(centreInHeight(nav.removeFromRight(150), 28));
        nav.removeFromRight(8);
    }
    else
    {
        nameEditor.setBounds({});
    }

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
            rowHeight = std::max(rowHeight, item.section->preferredHeight(itemWidth));
        }

        int x = outerPad;
        for (std::size_t i = 0; i < row.size(); ++i)
        {
            row[i].section->setBounds(x, y, widths[i], rowHeight);
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
        if (slotPanels[0] == nullptr || slotPanels[1] == nullptr || coreOscPanel == nullptr
            || sequencerPanel == nullptr || presetBrowserPanel == nullptr || midiControllerPanel == nullptr)
            return;
        // Synthesis is the hero: oscillator slots, tone source, filter/envelopes/LFO, and
        // performance modules read first, matching the Sylenth everything-visible grid. The
        // preset browser and MIDI utility panels move to the bottom so the core patching
        // surface fills the first screen with minimal scrolling.
        std::vector<std::vector<RowItem>> rows = {
            { { slotPanels[0].get(), 0.5f }, { slotPanels[1].get(), 0.5f } },
            { { coreOscPanel, 1.0f } },
            { { filterPanel, 0.34f }, { ampEnvPanel, 0.16f }, { modEnvPanel, 0.16f }, { lfoPanel, 0.34f } },
            { { voicePanel, 0.26f }, { ampPanel, 0.24f }, { rampPanel, 0.24f }, { macroPanel, 0.26f } },
            { { sequencerPanel.get(), 1.0f } },
            { { presetBrowserPanel.get(), 1.0f } },
            { { midiControllerPanel.get(), 1.0f } },
        };
        layoutRows(soundPage, rows, viewWidth);
    }
    else if (currentPage == Page::Mod)
    {
        if (modulationOverviewPanel == nullptr)
            return;

        std::vector<std::vector<RowItem>> rows = {
            { { modulationOverviewPanel.get(), 1.0f } },
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
            { { saturationPanel, 0.25f }, { phaserPanel, 0.25f },
              { chorusPanel, 0.25f }, { eqPanel, 0.25f } },
            { { delayPanel, 0.25f }, { reverbPanel, 0.25f },
              { compressorPanel, 0.25f }, { masterFxPanel, 0.25f } },
        };
        layoutRows(fxPage, rows, viewWidth);
    }
}

void SynthAudioProcessorEditor::refreshPresetMenu()
{
    presetItems = audioProcessor.getPresetList();
    presetCombo.clear(juce::dontSendNotification);
    const auto currentFilePath = audioProcessor.getCurrentPresetFilePath();
    const auto currentName = audioProcessor.getCurrentPresetName();

    juce::String currentSection;
    for (int i = 0; i < static_cast<int>(presetItems.size()); ++i)
    {
        const auto& item = presetItems[static_cast<std::size_t>(i)];
        const auto section = presetMenuSection(item);
        if (section != currentSection)
        {
            presetCombo.addSectionHeading(section);
            currentSection = section;
        }

        presetCombo.addItem(presetMenuLabel(item), i + 1);
    }

    if (presetItems.empty())
    {
        presetCombo.setTextWhenNothingSelected("Preset browser: no presets scanned");
        if (presetBrowserPanel != nullptr)
            presetBrowserPanel->setItems(presetItems, currentFilePath, currentName);
        return;
    }

    presetCombo.setTextWhenNothingSelected("Select preset");

    int selectedPresetIndex = -1;
    if (currentFilePath.isNotEmpty())
    {
        for (int i = 0; i < static_cast<int>(presetItems.size()); ++i)
        {
            if (presetItems[static_cast<std::size_t>(i)].file.getFullPathName() == currentFilePath)
            {
                selectedPresetIndex = i;
                break;
            }
        }
    }

    if (selectedPresetIndex < 0)
    {
        for (int i = 0; i < static_cast<int>(presetItems.size()); ++i)
        {
            if (presetItems[static_cast<std::size_t>(i)].displayName == currentName)
            {
                selectedPresetIndex = i;
                break;
            }
        }
    }

    if (selectedPresetIndex >= 0)
        presetCombo.setSelectedItemIndex(selectedPresetIndex, juce::dontSendNotification);

    if (presetBrowserPanel != nullptr)
        presetBrowserPanel->setItems(presetItems, currentFilePath, currentName);
}

void SynthAudioProcessorEditor::loadSelectedPreset()
{
    const auto index = presetCombo.getSelectedItemIndex();
    loadPresetAtIndex(index);
}

void SynthAudioProcessorEditor::loadPresetAtIndex(int itemIndex)
{
    const auto index = itemIndex;
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

void SynthAudioProcessorEditor::togglePresetFavoriteAtIndex(int itemIndex)
{
    if (itemIndex < 0 || itemIndex >= static_cast<int>(presetItems.size()))
        return;

    const auto& item = presetItems[static_cast<std::size_t>(itemIndex)];
    if (item.favoriteKey.isEmpty())
    {
        updateStatus("Preset favorite unavailable");
        return;
    }

    std::string error;
    const auto shouldFavorite = !item.favorite;
    if (!synth::setPresetFavorite(synth::defaultPresetFavoritesFile(),
                                  item.favoriteKey.toStdString(),
                                  shouldFavorite,
                                  error))
    {
        updateStatus("Favorite update failed: " + juce::String(error));
        return;
    }

    updateStatus((shouldFavorite ? "Favorited preset: " : "Removed favorite: ")
                 + (item.displayName.isNotEmpty() ? item.displayName : item.file.getFileNameWithoutExtension()));
    refreshPresetMenu();
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

    fileChooser = std::make_unique<juce::FileChooser>("Save sylenth-ai preset", suggestedFile, "*.json");
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
                         + (raw("fx.phaser_enabled", 0.0f) >= 0.5f ? 1 : 0)
                         + (raw("fx.delay_enabled", 1.0f) >= 0.5f ? 1 : 0)
                         + (raw("fx.reverb_enabled", 1.0f) >= 0.5f ? 1 : 0)
                         + (raw("fx.chorus_enabled", 0.0f) >= 0.5f ? 1 : 0)
                         + (raw("fx.eq_enabled", 0.0f) >= 0.5f ? 1 : 0)
                         + (raw("fx.compressor_enabled", 0.0f) >= 0.5f ? 1 : 0);

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
    if (midiControllerPanel != nullptr)
        midiControllerPanel->refresh();

    // Refresh module power dots/dim for the visible page only (keeps hidden tabs idle).
    if (currentPage == Page::Fx)
    {
        for (auto& panel : fxPanels)
            panel->syncEnabledState();
    }
    else if (currentPage == Page::Mod)
    {
        if (modulationOverviewPanel != nullptr)
            modulationOverviewPanel->refresh();
        for (auto* panel : transModPanels)
            if (panel != nullptr)
                panel->syncEnabledState();
    }
}
