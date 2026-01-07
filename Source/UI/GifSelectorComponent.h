#pragma once

#include <JuceHeader.h>
#include <functional>

class GifSelectorComponent : public juce::Component
{
public:
    GifSelectorComponent();

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Callback when a preset is selected (index 0-5)
    std::function<void(int)> onPresetSelected;

    // Callback when upload button clicked
    std::function<void()> onUploadClicked;

    // Set the currently selected preset
    void setSelectedPreset(int index);

private:
    static constexpr int NUM_PRESETS = 6;

    std::array<std::unique_ptr<juce::TextButton>, NUM_PRESETS> presetButtons;
    std::unique_ptr<juce::TextButton> uploadButton;

    int selectedIndex = 0;

    void updateButtonStates();
};
