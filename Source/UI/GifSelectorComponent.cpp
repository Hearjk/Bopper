#include "GifSelectorComponent.h"
#include "UI/BopperLookAndFeel.h"

GifSelectorComponent::GifSelectorComponent()
{
    // Preset names: Cat, Heart, Dance (SpongeBob)
    const char* presetNames[NUM_PRESETS] = {
        "Cat", "Heart", "Dance"};

    for (int i = 0; i < NUM_PRESETS; ++i)
    {
        presetButtons[i] = std::make_unique<juce::TextButton>(presetNames[i]);
        presetButtons[i]->setClickingTogglesState(true);
        presetButtons[i]->setRadioGroupId(1);
        presetButtons[i]->onClick = [this, i]()
        {
            selectedPresetIndex = i;
            selectedSavedIndex = -1;
            updateButtonStates();
            if (onPresetSelected)
                onPresetSelected(i);
        };
        addAndMakeVisible(presetButtons[i].get());
    }

    // Set first preset as selected
    presetButtons[0]->setToggleState(true, juce::dontSendNotification);

    // Saved GIF slots
    for (int i = 0; i < NUM_SAVED_SLOTS; ++i)
    {
        savedSlotButtons[i] = std::make_unique<juce::TextButton>("Slot " + juce::String(i + 1));
        savedSlotButtons[i]->setClickingTogglesState(true);
        savedSlotButtons[i]->setRadioGroupId(1);
        savedSlotButtons[i]->onClick = [this, i]()
        {
            if (savedSlotHasGif[static_cast<size_t>(i)])
            {
                selectedSavedIndex = i;
                selectedPresetIndex = -1;
                updateButtonStates();
                if (onSavedGifSelected)
                    onSavedGifSelected(i);
            }
            else
            {
                // No GIF in slot - trigger save action
                if (onSaveToSlot)
                    onSaveToSlot(i);
                updateButtonStates();
            }
        };
        addAndMakeVisible(savedSlotButtons[i].get());
    }

    // Upload button
    uploadButton = std::make_unique<juce::TextButton>("Upload GIF...");
    uploadButton->onClick = [this]()
    {
        if (onUploadClicked)
            onUploadClicked();
    };
    addAndMakeVisible(uploadButton.get());
}

void GifSelectorComponent::paint(juce::Graphics& g)
{
    // Background handled by parent
}

void GifSelectorComponent::resized()
{
    auto bounds = getLocalBounds();

    // Top row: Presets label + preset buttons
    auto topRow = bounds.removeFromTop(36);

    // Preset buttons
    int presetButtonWidth = (topRow.getWidth() - 10) / NUM_PRESETS;
    for (int i = 0; i < NUM_PRESETS; ++i)
    {
        presetButtons[i]->setBounds(
            topRow.removeFromLeft(presetButtonWidth).reduced(2));
        if (i < NUM_PRESETS - 1)
            topRow.removeFromLeft(4); // spacing
    }

    bounds.removeFromTop(8); // spacing

    // Second row: Saved slots
    auto savedRow = bounds.removeFromTop(36);

    int savedButtonWidth = (savedRow.getWidth() - 10) / NUM_SAVED_SLOTS;
    for (int i = 0; i < NUM_SAVED_SLOTS; ++i)
    {
        savedSlotButtons[i]->setBounds(
            savedRow.removeFromLeft(savedButtonWidth).reduced(2));
        if (i < NUM_SAVED_SLOTS - 1)
            savedRow.removeFromLeft(4); // spacing
    }

    bounds.removeFromTop(8); // spacing

    // Upload button
    uploadButton->setBounds(bounds.reduced(2, 0).withHeight(36));
}

void GifSelectorComponent::setSelectedPreset(int index)
{
    if (index >= 0 && index < NUM_PRESETS)
    {
        selectedPresetIndex = index;
        selectedSavedIndex = -1;
        updateButtonStates();
    }
}

void GifSelectorComponent::setSelectedSavedSlot(int index)
{
    if (index >= 0 && index < NUM_SAVED_SLOTS)
    {
        selectedSavedIndex = index;
        selectedPresetIndex = -1;
        updateButtonStates();
    }
}

void GifSelectorComponent::updateSavedSlotState(int slot, bool hasGif)
{
    if (slot >= 0 && slot < NUM_SAVED_SLOTS)
    {
        savedSlotHasGif[static_cast<size_t>(slot)] = hasGif;
        if (hasGif)
        {
            savedSlotButtons[static_cast<size_t>(slot)]->setButtonText("Saved " + juce::String(slot + 1));
        }
        else
        {
            savedSlotButtons[static_cast<size_t>(slot)]->setButtonText("Save to " + juce::String(slot + 1));
        }
    }
}

void GifSelectorComponent::updateButtonStates()
{
    for (int i = 0; i < NUM_PRESETS; ++i)
    {
        presetButtons[i]->setToggleState(i == selectedPresetIndex, juce::dontSendNotification);
    }
    for (int i = 0; i < NUM_SAVED_SLOTS; ++i)
    {
        savedSlotButtons[i]->setToggleState(i == selectedSavedIndex, juce::dontSendNotification);
    }
}
