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

    // Saved GIF slots - start as "Upload" buttons
    for (int i = 0; i < NUM_SAVED_SLOTS; ++i)
    {
        savedSlotButtons[i] = std::make_unique<juce::TextButton>("Upload");
        savedSlotButtons[i]->setClickingTogglesState(false);
        savedSlotButtons[i]->onClick = [this, i]()
        {
            if (savedSlotHasGif[static_cast<size_t>(i)])
            {
                // Has GIF - select it
                selectedSavedIndex = i;
                selectedPresetIndex = -1;
                updateButtonStates();
                if (onSavedGifSelected)
                    onSavedGifSelected(i);
            }
            else
            {
                // No GIF - trigger upload to this slot
                if (onUploadToSlot)
                    onUploadToSlot(i);
            }
        };
        addAndMakeVisible(savedSlotButtons[i].get());

        // Delete buttons (hidden by default)
        deleteButtons[i] = std::make_unique<juce::TextButton>("X");
        deleteButtons[i]->onClick = [this, i]()
        {
            if (onDeleteFromSlot)
                onDeleteFromSlot(i);
        };
        deleteButtons[i]->setVisible(false);
        addAndMakeVisible(deleteButtons[i].get());
    }
}

void GifSelectorComponent::paint(juce::Graphics& g)
{
    // Background handled by parent
}

void GifSelectorComponent::resized()
{
    auto bounds = getLocalBounds();

    // Top row: Preset buttons
    auto topRow = bounds.removeFromTop(36);

    int presetButtonWidth = (topRow.getWidth() - 10) / NUM_PRESETS;
    for (int i = 0; i < NUM_PRESETS; ++i)
    {
        presetButtons[i]->setBounds(
            topRow.removeFromLeft(presetButtonWidth).reduced(2));
        if (i < NUM_PRESETS - 1)
            topRow.removeFromLeft(4); // spacing
    }

    bounds.removeFromTop(8); // spacing

    // Second row: Upload/Saved slots with delete buttons
    auto savedRow = bounds.removeFromTop(36);

    int totalWidth = savedRow.getWidth();
    int slotWidth = (totalWidth - 20) / NUM_SAVED_SLOTS; // Account for spacing

    for (int i = 0; i < NUM_SAVED_SLOTS; ++i)
    {
        auto slotBounds = savedRow.removeFromLeft(slotWidth).reduced(2);

        if (savedSlotHasGif[static_cast<size_t>(i)])
        {
            // Show slot button with delete button next to it
            auto deleteBounds = slotBounds.removeFromRight(30);
            savedSlotButtons[i]->setBounds(slotBounds);
            deleteButtons[i]->setBounds(deleteBounds.reduced(2));
            deleteButtons[i]->setVisible(true);
        }
        else
        {
            // Just show upload button
            savedSlotButtons[i]->setBounds(slotBounds);
            deleteButtons[i]->setVisible(false);
        }

        if (i < NUM_SAVED_SLOTS - 1)
            savedRow.removeFromLeft(6); // spacing
    }
}

void GifSelectorComponent::setSelectedPreset(int index)
{
    if (index >= 0 && index < NUM_PRESETS)
    {
        selectedPresetIndex = index;
        selectedSavedIndex = -1;
        updateButtonStates();
    }
    else if (index < 0)
    {
        selectedPresetIndex = -1;
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
    else if (index < 0)
    {
        selectedSavedIndex = -1;
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
            savedSlotButtons[static_cast<size_t>(slot)]->setButtonText("Slot " + juce::String(slot + 1));
            savedSlotButtons[static_cast<size_t>(slot)]->setClickingTogglesState(true);
            savedSlotButtons[static_cast<size_t>(slot)]->setRadioGroupId(1);
        }
        else
        {
            savedSlotButtons[static_cast<size_t>(slot)]->setButtonText("Upload");
            savedSlotButtons[static_cast<size_t>(slot)]->setClickingTogglesState(false);
            savedSlotButtons[static_cast<size_t>(slot)]->setRadioGroupId(0);
            savedSlotButtons[static_cast<size_t>(slot)]->setToggleState(false, juce::dontSendNotification);
        }
        // Trigger layout update for delete button visibility
        resized();
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
        if (savedSlotHasGif[static_cast<size_t>(i)])
        {
            savedSlotButtons[i]->setToggleState(i == selectedSavedIndex, juce::dontSendNotification);
        }
    }
}
