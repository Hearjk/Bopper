#pragma once

#include <JuceHeader.h>
#include <functional>

class GifSelectorComponent : public juce::Component
{
public:
    GifSelectorComponent();

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Callback when a preset is selected (index 0-2)
    std::function<void(int)> onPresetSelected;

    // Callback when a saved GIF slot is selected (index 0-2)
    std::function<void(int)> onSavedGifSelected;

    // Callback when upload to slot is requested (slot 0-2)
    std::function<void(int)> onUploadToSlot;

    // Callback when delete from slot is requested (slot 0-2)
    std::function<void(int)> onDeleteFromSlot;

    // Set the currently selected preset (-1 for none)
    void setSelectedPreset(int index);

    // Set the currently selected saved slot (-1 for none)
    void setSelectedSavedSlot(int index);

    // Update saved slot button appearance (has GIF or empty)
    void updateSavedSlotState(int slot, bool hasGif);

private:
    static constexpr int NUM_PRESETS = 3;
    static constexpr int NUM_SAVED_SLOTS = 3;

    std::array<std::unique_ptr<juce::TextButton>, NUM_PRESETS> presetButtons;
    std::array<std::unique_ptr<juce::TextButton>, NUM_SAVED_SLOTS> savedSlotButtons;
    std::array<std::unique_ptr<juce::TextButton>, NUM_SAVED_SLOTS> deleteButtons;

    int selectedPresetIndex = 0;
    int selectedSavedIndex = -1;
    std::array<bool, NUM_SAVED_SLOTS> savedSlotHasGif{false, false, false};

    void updateButtonStates();
};
