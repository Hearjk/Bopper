#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GIF/GifAnimator.h"
#include "UI/BeatGIFLookAndFeel.h"
#include "UI/GifDisplayComponent.h"
#include "UI/GifSelectorComponent.h"

class BeatGIFAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    BeatGIFAudioProcessorEditor(BeatGIFAudioProcessor&);
    ~BeatGIFAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void loadPresetGif(int index);
    void openFileChooser();
    void loadCustomGif(const juce::File& file);

    // Embedded preset GIF data
    struct PresetGif
    {
        const char* name;
        const unsigned char* data;
        size_t size;
    };

    static const std::array<PresetGif, 6>& getPresetGifs();

    BeatGIFAudioProcessor& audioProcessor;
    BeatGIFLookAndFeel lookAndFeel;

    // GIF animation
    GifAnimator gifAnimator;

    // UI Components
    GifDisplayComponent gifDisplay;
    GifSelectorComponent gifSelector;

    // Header elements
    juce::Label titleLabel;
    juce::Label bpmLabel;

    // For file browsing
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BeatGIFAudioProcessorEditor)
};
