#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GIF/GifAnimator.h"
#include "UI/BopperLookAndFeel.h"
#include "UI/GifDisplayComponent.h"
#include "UI/GifSelectorComponent.h"

class BopperAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    BopperAudioProcessorEditor(BopperAudioProcessor&);
    ~BopperAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void loadPresetGif(int index);
    void loadSavedGif(int slot);
    void uploadToSlot(int slot);
    void deleteFromSlot(int slot);
    void enterTheaterMode();
    void exitTheaterMode();
    void updateSpeedLabel();

    // Embedded preset GIF data
    struct PresetGif
    {
        const char* name;
        const unsigned char* data;
        size_t size;
    };

    static const std::array<PresetGif, 3>& getPresetGifs();

    BopperAudioProcessor& audioProcessor;
    BopperLookAndFeel lookAndFeel;

    // GIF animation
    GifAnimator gifAnimator;

    // UI Components
    GifDisplayComponent gifDisplay;
    GifSelectorComponent gifSelector;

    // Header elements
    juce::Label titleLabel;
    juce::Label bpmLabel;

    // Speed control
    juce::Slider speedSlider;
    juce::Label speedLabel;

    // Effects controls
    juce::TextButton reverseButton;
    juce::TextButton pingPongButton;
    juce::ComboBox colorFilterCombo;
    juce::TextButton pulseButton;
    juce::TextButton shakeButton;

    // Theater mode button and banner
    juce::TextButton theaterButton;
    juce::Label theaterBannerLabel;
    bool isTheaterMode = false;

    // For file browsing
    std::unique_ptr<juce::FileChooser> fileChooser;
    int pendingUploadSlot = -1; // Track which slot we're uploading to

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BopperAudioProcessorEditor)
};
