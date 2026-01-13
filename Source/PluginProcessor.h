#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <array>

// Effect types
enum class ColorFilterType
{
    None = 0,
    Invert,
    Sepia,
    Cyberpunk,  // Cyan/pink tint
    Vaporwave,  // Purple/pink tint
    Matrix      // Green tint
};

class BopperAudioProcessor : public juce::AudioProcessor
{
public:
    BopperAudioProcessor();
    ~BopperAudioProcessor() override;

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

    // Speed divisor (0 = 1x, 1 = 1/2, 2 = 1/4, 3 = 1/8, 4 = 1/16)
    void setSpeedDivisor(int divisor) { speedDivisor.store(divisor); }
    int getSpeedDivisor() const { return speedDivisor.load(); }

    // Saved GIFs (3 slots)
    static constexpr int NUM_SAVED_SLOTS = 3;
    void setSavedGifPath(int slot, const juce::String& path);
    juce::String getSavedGifPath(int slot) const;

    // Effects
    void setReverseEnabled(bool enabled) { reverseEnabled.store(enabled); }
    bool getReverseEnabled() const { return reverseEnabled.load(); }

    void setPingPongEnabled(bool enabled) { pingPongEnabled.store(enabled); }
    bool getPingPongEnabled() const { return pingPongEnabled.load(); }

    void setColorFilter(ColorFilterType filter) { colorFilter.store(static_cast<int>(filter)); }
    ColorFilterType getColorFilter() const { return static_cast<ColorFilterType>(colorFilter.load()); }

    void setPulseEnabled(bool enabled) { pulseEnabled.store(enabled); }
    bool getPulseEnabled() const { return pulseEnabled.load(); }

    void setShakeEnabled(bool enabled) { shakeEnabled.store(enabled); }
    bool getShakeEnabled() const { return shakeEnabled.load(); }

private:
    std::atomic<double> bpmState{120.0};
    std::atomic<double> ppqState{0.0};
    std::atomic<bool> playingState{false};
    std::atomic<int> selectedGifIndex{0};
    std::atomic<int> speedDivisor{0};
    juce::String customGifPath;
    std::array<juce::String, NUM_SAVED_SLOTS> savedGifPaths;

    // Effects state
    std::atomic<bool> reverseEnabled{false};
    std::atomic<bool> pingPongEnabled{false};
    std::atomic<int> colorFilter{0};
    std::atomic<bool> pulseEnabled{false};
    std::atomic<bool> shakeEnabled{false};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BopperAudioProcessor)
};
