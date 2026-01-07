#include "PluginProcessor.h"
#include "PluginEditor.h"

BeatGIFAudioProcessor::BeatGIFAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

BeatGIFAudioProcessor::~BeatGIFAudioProcessor()
{
}

const juce::String BeatGIFAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BeatGIFAudioProcessor::acceptsMidi() const
{
    return false;
}

bool BeatGIFAudioProcessor::producesMidi() const
{
    return false;
}

bool BeatGIFAudioProcessor::isMidiEffect() const
{
    return false;
}

double BeatGIFAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BeatGIFAudioProcessor::getNumPrograms()
{
    return 1;
}

int BeatGIFAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BeatGIFAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String BeatGIFAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void BeatGIFAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void BeatGIFAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void BeatGIFAudioProcessor::releaseResources()
{
}

bool BeatGIFAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void BeatGIFAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    // Pass through audio unchanged
    // (This is a visual-only plugin)

    // Extract BPM and transport info from host
    double currentBpm = 120.0;
    bool isPlaying = false;
    double ppqPosition = 0.0;

    if (auto* playHead = getPlayHead())
    {
        if (auto positionInfo = playHead->getPosition())
        {
            // Get BPM
            if (auto bpm = positionInfo->getBpm())
            {
                if (*bpm > 0.0)
                    currentBpm = *bpm;
            }

            // Get playback state
            isPlaying = positionInfo->getIsPlaying();

            // Get PPQ position for precise beat alignment
            if (auto ppq = positionInfo->getPpqPosition())
            {
                ppqPosition = *ppq;
            }
        }
    }

    // Update atomic state for UI thread
    bpmState.store(currentBpm);
    playingState.store(isPlaying);
    ppqState.store(ppqPosition);
}

bool BeatGIFAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* BeatGIFAudioProcessor::createEditor()
{
    return new BeatGIFAudioProcessorEditor(*this);
}

void BeatGIFAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ValueTree state("BeatGIFState");
    state.setProperty("selectedGif", selectedGifIndex.load(), nullptr);
    state.setProperty("customGifPath", customGifPath, nullptr);

    juce::MemoryOutputStream stream(destData, false);
    state.writeToStream(stream);
}

void BeatGIFAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ValueTree state = juce::ValueTree::readFromData(data, static_cast<size_t>(sizeInBytes));

    if (state.isValid())
    {
        selectedGifIndex.store(state.getProperty("selectedGif", 0));
        customGifPath = state.getProperty("customGifPath", "").toString();
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BeatGIFAudioProcessor();
}
