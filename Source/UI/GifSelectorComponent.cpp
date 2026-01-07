#include "GifSelectorComponent.h"
#include "UI/BeatGIFLookAndFeel.h"

GifSelectorComponent::GifSelectorComponent()
{
    // Preset names
    const char* presetNames[NUM_PRESETS] = {
        "Cat", "Ball", "Disco", "Wave", "Heart", "Notes"};

    for (int i = 0; i < NUM_PRESETS; ++i)
    {
        presetButtons[i] = std::make_unique<juce::TextButton>(presetNames[i]);
        presetButtons[i]->setClickingTogglesState(true);
        presetButtons[i]->setRadioGroupId(1);
        presetButtons[i]->onClick = [this, i]()
        {
            selectedIndex = i;
            updateButtonStates();
            if (onPresetSelected)
                onPresetSelected(i);
        };
        addAndMakeVisible(presetButtons[i].get());
    }

    // Set first preset as selected
    presetButtons[0]->setToggleState(true, juce::dontSendNotification);

    // Upload button
    uploadButton = std::make_unique<juce::TextButton>("Upload Custom GIF...");
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

    // Preset buttons row
    auto presetRow = bounds.removeFromTop(40);
    int buttonWidth = presetRow.getWidth() / NUM_PRESETS - 4;

    for (int i = 0; i < NUM_PRESETS; ++i)
    {
        presetButtons[i]->setBounds(
            presetRow.removeFromLeft(buttonWidth).reduced(2));
        presetRow.removeFromLeft(4); // spacing
    }

    bounds.removeFromTop(8); // spacing

    // Upload button
    uploadButton->setBounds(bounds.reduced(2, 0).withHeight(36));
}

void GifSelectorComponent::setSelectedPreset(int index)
{
    if (index >= 0 && index < NUM_PRESETS)
    {
        selectedIndex = index;
        updateButtonStates();
    }
}

void GifSelectorComponent::updateButtonStates()
{
    for (int i = 0; i < NUM_PRESETS; ++i)
    {
        presetButtons[i]->setToggleState(i == selectedIndex, juce::dontSendNotification);
    }
}
