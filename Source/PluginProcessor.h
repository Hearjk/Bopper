#pragma once

#include <JuceHeader.h>
#include <atomic>

class BeatGIFAudioProcessor : public juce::AudioProcessor
{
public:
    BeatGIFAudioProcessor();
    ~BeatGIFAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Thread-safe state accessors for UI
    double getBpm() const { return bpmState.load(); }
    double getPpqPosition() const { return ppqState.load(); }
    bool isHostPlaying() const { return playingState.load(); }

    // Selected GIF state
    void setSelectedGifIndex(int index) { selectedGifIndex.store(index); }
    int getSelectedGifIndex() const { return selectedGifIndex.load(); }

    void setCustomGifPath(const juce::String& path) { customGifPath = path; }
    juce::String getCustomGifPath() const { return customGifPath; }

private:
    std::atomic<double> bpmState{120.0};
    std::atomic<double> ppqState{0.0};
    std::atomic<bool> playingState{false};
    std::atomic<int> selectedGifIndex{0};
    juce::String customGifPath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BeatGIFAudioProcessor)
};
