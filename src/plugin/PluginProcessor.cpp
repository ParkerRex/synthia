#include "PluginProcessor.h"

#include "PluginEditor.h"

SynthAudioProcessor::SynthAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "SYNTH_STATE", synth::createParameterLayout())
{
}

void SynthAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engine.prepare(sampleRate, samplesPerBlock);
}

void SynthAudioProcessor::releaseResources()
{
    engine.reset();
}

bool SynthAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& output = layouts.getMainOutputChannelSet();
    return output == juce::AudioChannelSet::mono()
        || output == juce::AudioChannelSet::stereo();
}

void SynthAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    for (const auto metadata : midiMessages)
    {
        const auto message = metadata.getMessage();

        if (message.isNoteOn())
            engine.noteOn(message.getNoteNumber(), message.getFloatVelocity());
        else if (message.isNoteOff())
            engine.noteOff(message.getNoteNumber());
        else if (message.isAllNotesOff() || message.isAllSoundOff())
            engine.allNotesOff();
    }

    for (int channel = 2; channel < buffer.getNumChannels(); ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    auto* left = buffer.getNumChannels() > 0 ? buffer.getWritePointer(0) : nullptr;
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : left;

    engine.process(left, right, buffer.getNumSamples());
}

juce::AudioProcessorEditor* SynthAudioProcessor::createEditor()
{
    return new SynthAudioProcessorEditor(*this);
}

void SynthAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    state.setProperty("schema_version", 1, nullptr);
    state.setProperty("plugin_version", ProjectInfo::versionString, nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void SynthAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        if (xml->hasTagName("SYNTH_STATE"))
        {
            auto state = juce::ValueTree::fromXml(*xml);
            if (state.isValid())
                parameters.replaceState(state);
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SynthAudioProcessor();
}
