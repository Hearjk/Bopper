#include "PluginProcessor.h"
#include "PluginEditor.h"

BopperAudioProcessor::BopperAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

BopperAudioProcessor::~BopperAudioProcessor()
{
}

const juce::String BopperAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BopperAudioProcessor::acceptsMidi() const
{
    return false;
}

bool BopperAudioProcessor::producesMidi() const
{
    return false;
}

bool BopperAudioProcessor::isMidiEffect() const
{
    return false;
}

double BopperAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BopperAudioProcessor::getNumPrograms()
{
    return 1;
}

int BopperAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BopperAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String BopperAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void BopperAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void BopperAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void BopperAudioProcessor::releaseResources()
{
}

bool BopperAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void BopperAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
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

bool BopperAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* BopperAudioProcessor::createEditor()
{
    return new BopperAudioProcessorEditor(*this);
}

void BopperAudioProcessor::setSavedGifPath(int slot, const juce::String& path)
{
    if (slot >= 0 && slot < NUM_SAVED_SLOTS)
        savedGifPaths[static_cast<size_t>(slot)] = path;
}

juce::String BopperAudioProcessor::getSavedGifPath(int slot) const
{
    if (slot >= 0 && slot < NUM_SAVED_SLOTS)
        return savedGifPaths[static_cast<size_t>(slot)];
    return {};
}

void BopperAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ValueTree state("BopperState");
    state.setProperty("selectedGif", selectedGifIndex.load(), nullptr);
    state.setProperty("customGifPath", customGifPath, nullptr);
    state.setProperty("speedDivisor", speedDivisor.load(), nullptr);

    for (int i = 0; i < NUM_SAVED_SLOTS; ++i)
        state.setProperty("savedGif" + juce::String(i), savedGifPaths[static_cast<size_t>(i)], nullptr);

    juce::MemoryOutputStream stream(destData, false);
    state.writeToStream(stream);
}

void BopperAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ValueTree state = juce::ValueTree::readFromData(data, static_cast<size_t>(sizeInBytes));

    if (state.isValid())
    {
        selectedGifIndex.store(state.getProperty("selectedGif", 0));
        customGifPath = state.getProperty("customGifPath", "").toString();
        speedDivisor.store(state.getProperty("speedDivisor", 0));

        for (int i = 0; i < NUM_SAVED_SLOTS; ++i)
            savedGifPaths[static_cast<size_t>(i)] = state.getProperty("savedGif" + juce::String(i), "").toString();
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BopperAudioProcessor();
}
