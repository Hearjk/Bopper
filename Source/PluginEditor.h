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
    void openFileChooser();
    void loadCustomGif(const juce::File& file);
    void loadSavedGif(int slot);
    void saveCurrentGifToSlot(int slot);
    void enterFullscreen();
    void exitFullscreen();
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

    // Fullscreen button
    juce::TextButton fullscreenButton;
    bool isFullscreen = false;
    juce::Rectangle<int> normalBounds;

    // For file browsing
    std::unique_ptr<juce::FileChooser> fileChooser;

    // Currently loaded custom GIF path (for saving to slots)
    juce::String currentCustomGifPath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BopperAudioProcessorEditor)
};
