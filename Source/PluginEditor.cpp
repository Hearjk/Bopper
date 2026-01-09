#include "PluginEditor.h"

BopperAudioProcessorEditor::BopperAudioProcessorEditor(BopperAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&lookAndFeel);

    // Title
    titleLabel.setText("Bopper", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, BopperLookAndFeel::Colors::text);
    addAndMakeVisible(titleLabel);

    // BPM display
    bpmLabel.setText("BPM: 120", juce::dontSendNotification);
    bpmLabel.setFont(juce::Font(16.0f));
    bpmLabel.setColour(juce::Label::textColourId, BopperLookAndFeel::Colors::highlight);
    bpmLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(bpmLabel);

    // Speed slider (0-4 for 1x, 1/2, 1/4, 1/8, 1/16)
    speedSlider.setRange(0, 4, 1);
    speedSlider.setValue(audioProcessor.getSpeedDivisor());
    speedSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    speedSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    speedSlider.onValueChange = [this]()
    {
        int divisor = static_cast<int>(speedSlider.getValue());
        audioProcessor.setSpeedDivisor(divisor);
        updateSpeedLabel();
    };
    addAndMakeVisible(speedSlider);

    // Speed label
    speedLabel.setFont(juce::Font(12.0f));
    speedLabel.setColour(juce::Label::textColourId, BopperLookAndFeel::Colors::textDim);
    speedLabel.setJustificationType(juce::Justification::centred);
    updateSpeedLabel();
    addAndMakeVisible(speedLabel);

    // Fullscreen button
    fullscreenButton.setButtonText("Fullscreen");
    fullscreenButton.onClick = [this]()
    {
        if (isFullscreen)
            exitFullscreen();
        else
            enterFullscreen();
    };
    addAndMakeVisible(fullscreenButton);

    // GIF display
    gifDisplay.setAnimator(&gifAnimator);
    addAndMakeVisible(gifDisplay);

    // GIF selector
    gifSelector.onPresetSelected = [this](int index)
    {
        loadPresetGif(index);
        audioProcessor.setSelectedGifIndex(index);
        currentCustomGifPath = "";
    };

    gifSelector.onSavedGifSelected = [this](int slot)
    {
        loadSavedGif(slot);
    };

    gifSelector.onUploadClicked = [this]()
    {
        openFileChooser();
    };

    gifSelector.onSaveToSlot = [this](int slot)
    {
        saveCurrentGifToSlot(slot);
    };

    addAndMakeVisible(gifSelector);

    // Initialize saved slot states
    for (int i = 0; i < BopperAudioProcessor::NUM_SAVED_SLOTS; ++i)
    {
        bool hasGif = audioProcessor.getSavedGifPath(i).isNotEmpty();
        gifSelector.updateSavedSlotState(i, hasGif);
    }

    // Load initial preset
    int savedIndex = audioProcessor.getSelectedGifIndex();
    if (savedIndex >= 0 && savedIndex < 3)
    {
        gifSelector.setSelectedPreset(savedIndex);
        loadPresetGif(savedIndex);
    }

    // Check for custom GIF path
    auto customPath = audioProcessor.getCustomGifPath();
    if (customPath.isNotEmpty())
    {
        juce::File customFile(customPath);
        if (customFile.existsAsFile())
        {
            loadCustomGif(customFile);
        }
    }

    // Start timer for UI updates (60fps)
    startTimerHz(60);

    setSize(500, 550);
}

BopperAudioProcessorEditor::~BopperAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void BopperAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(BopperLookAndFeel::Colors::background);
}

void BopperAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(16);

    if (isFullscreen)
    {
        // In fullscreen mode, just show the GIF display
        gifDisplay.setBounds(bounds);
        titleLabel.setVisible(false);
        bpmLabel.setVisible(false);
        speedSlider.setVisible(false);
        speedLabel.setVisible(false);
        gifSelector.setVisible(false);
        fullscreenButton.setVisible(true);
        fullscreenButton.setBounds(bounds.removeFromBottom(36).removeFromRight(100));
        return;
    }

    // Normal mode
    titleLabel.setVisible(true);
    bpmLabel.setVisible(true);
    speedSlider.setVisible(true);
    speedLabel.setVisible(true);
    gifSelector.setVisible(true);
    fullscreenButton.setVisible(true);

    // Header row
    auto headerRow = bounds.removeFromTop(40);
    titleLabel.setBounds(headerRow.removeFromLeft(100));
    fullscreenButton.setBounds(headerRow.removeFromRight(100));
    bpmLabel.setBounds(headerRow);

    bounds.removeFromTop(8); // spacing

    // Speed control row
    auto speedRow = bounds.removeFromTop(30);
    speedLabel.setBounds(speedRow.removeFromLeft(60));
    speedSlider.setBounds(speedRow.reduced(4, 0));

    bounds.removeFromTop(8); // spacing

    // GIF display (centered, square-ish)
    auto displayHeight = bounds.getHeight() - 140; // Leave room for selector
    gifDisplay.setBounds(bounds.removeFromTop(displayHeight).reduced(20, 0));

    bounds.removeFromTop(12); // spacing

    // GIF selector
    gifSelector.setBounds(bounds);
}

void BopperAudioProcessorEditor::timerCallback()
{
    // Update BPM display
    double bpm = audioProcessor.getBpm();
    bpmLabel.setText("BPM: " + juce::String(static_cast<int>(bpm)), juce::dontSendNotification);

    // Update animation with speed divisor
    int speedDiv = audioProcessor.getSpeedDivisor();
    gifAnimator.update(bpm, audioProcessor.getPpqPosition(), audioProcessor.isHostPlaying(), speedDiv);

    // Repaint GIF display
    gifDisplay.updateDisplay();
}

void BopperAudioProcessorEditor::updateSpeedLabel()
{
    int divisor = static_cast<int>(speedSlider.getValue());
    juce::String speedText;
    switch (divisor)
    {
        case 0: speedText = "1x"; break;
        case 1: speedText = "1/2"; break;
        case 2: speedText = "1/4"; break;
        case 3: speedText = "1/8"; break;
        case 4: speedText = "1/16"; break;
        default: speedText = "1x"; break;
    }
    speedLabel.setText("Speed: " + speedText, juce::dontSendNotification);
}

void BopperAudioProcessorEditor::loadPresetGif(int index)
{
    // Generate a simple animated pattern based on index
    const int frameCount = 8;
    const int size = 200;

    std::vector<juce::Image> frames;

    // Color palette for different presets
    juce::Colour colors[] = {
        juce::Colour(0xFFFF6B6B), // Cat - Red
        juce::Colour(0xFFFF6B6B), // Heart - Red
        juce::Colour(0xFFFFE135)  // Dance - Yellow (SpongeBob-ish)
    };

    juce::Colour baseColor = colors[index % 3];

    for (int frame = 0; frame < frameCount; ++frame)
    {
        juce::Image img(juce::Image::ARGB, size, size, true);
        juce::Graphics g(img);

        float phase = static_cast<float>(frame) / frameCount;
        float bounce = std::sin(phase * juce::MathConstants<float>::twoPi);

        switch (index)
        {
        case 0: // Cat - bouncing circle with ears
        {
            float y = size / 2.0f + bounce * 30.0f;
            g.setColour(baseColor);
            g.fillEllipse(size / 2.0f - 50, y - 50, 100, 100);
            // Ears
            g.fillEllipse(size / 2.0f - 55, y - 70, 30, 40);
            g.fillEllipse(size / 2.0f + 25, y - 70, 30, 40);
            // Eyes
            g.setColour(juce::Colours::white);
            g.fillEllipse(size / 2.0f - 25, y - 20, 20, 25);
            g.fillEllipse(size / 2.0f + 5, y - 20, 20, 25);
            g.setColour(juce::Colours::black);
            g.fillEllipse(size / 2.0f - 20, y - 15, 10, 15);
            g.fillEllipse(size / 2.0f + 10, y - 15, 10, 15);
            break;
        }
        case 1: // Heart - pulsing heart
        {
            float scale = 1.0f + bounce * 0.2f;
            float cx = size / 2.0f;
            float cy = size / 2.0f;

            juce::Path heart;
            heart.startNewSubPath(cx, cy + 30 * scale);
            heart.cubicTo(cx - 50 * scale, cy - 20 * scale,
                          cx - 50 * scale, cy - 50 * scale,
                          cx, cy - 30 * scale);
            heart.cubicTo(cx + 50 * scale, cy - 50 * scale,
                          cx + 50 * scale, cy - 20 * scale,
                          cx, cy + 30 * scale);

            g.setColour(baseColor);
            g.fillPath(heart);
            break;
        }
        case 2: // Dance - dancing figure (SpongeBob-style)
        {
            float cx = size / 2.0f;
            float cy = size / 2.0f;

            // Body sway
            float sway = std::sin(phase * juce::MathConstants<float>::twoPi * 2) * 10.0f;
            float armAngle = bounce * 0.5f;

            // Body (rectangle)
            g.setColour(baseColor);
            juce::Path body;
            body.addRoundedRectangle(cx - 25 + sway * 0.3f, cy - 30, 50, 60, 8);
            g.fillPath(body);

            // Head
            g.fillEllipse(cx - 20 + sway * 0.5f, cy - 60, 40, 35);

            // Eyes
            g.setColour(juce::Colours::white);
            g.fillEllipse(cx - 12 + sway * 0.5f, cy - 55, 12, 15);
            g.fillEllipse(cx + 2 + sway * 0.5f, cy - 55, 12, 15);
            g.setColour(juce::Colour(0xFF87CEEB)); // Light blue
            g.fillEllipse(cx - 9 + sway * 0.5f, cy - 52, 6, 9);
            g.fillEllipse(cx + 5 + sway * 0.5f, cy - 52, 6, 9);

            // Smile
            g.setColour(juce::Colours::white);
            juce::Path smile;
            smile.addArc(cx - 10 + sway * 0.5f, cy - 45, 20, 15, 0.2f, juce::MathConstants<float>::pi - 0.2f, true);
            g.strokePath(smile, juce::PathStrokeType(2.0f));

            // Arms (waving)
            g.setColour(baseColor);
            // Left arm
            float leftArmY = cy - 10 + std::sin((phase + 0.25f) * juce::MathConstants<float>::twoPi) * 20;
            g.drawLine(cx - 25 + sway * 0.3f, cy - 10, cx - 50, leftArmY, 8.0f);
            // Right arm
            float rightArmY = cy - 10 + std::sin((phase + 0.75f) * juce::MathConstants<float>::twoPi) * 20;
            g.drawLine(cx + 25 + sway * 0.3f, cy - 10, cx + 50, rightArmY, 8.0f);

            // Legs
            g.setColour(juce::Colour(0xFF8B4513)); // Brown pants
            g.fillRect(cx - 20 + sway * 0.3f, cy + 30, 15.0f, 30.0f);
            g.fillRect(cx + 5 + sway * 0.3f, cy + 30, 15.0f, 30.0f);

            // Feet bounce
            float footBounce = std::abs(bounce) * 5;
            g.setColour(juce::Colours::black);
            g.fillEllipse(cx - 22 + sway * 0.3f, cy + 55 + footBounce, 20, 10);
            g.fillEllipse(cx + 3 + sway * 0.3f, cy + 55 - footBounce, 20, 10);
            break;
        }
        }

        frames.push_back(std::move(img));
    }

    // Load the generated frames into the animator
    if (!frames.empty())
    {
        gifAnimator.loadFrames(std::move(frames));
        gifDisplay.updateDisplay();
    }
}

void BopperAudioProcessorEditor::openFileChooser()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Select a GIF file",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        "*.gif");

    auto chooserFlags = juce::FileBrowserComponent::openMode |
                        juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
                             {
        auto file = fc.getResult();
        if (file.existsAsFile())
        {
            loadCustomGif(file);
        } });
}

void BopperAudioProcessorEditor::loadCustomGif(const juce::File& file)
{
    if (gifAnimator.loadGif(file))
    {
        currentCustomGifPath = file.getFullPathName();
        audioProcessor.setCustomGifPath(file.getFullPathName());
        audioProcessor.setSelectedGifIndex(-1); // Custom GIF
        gifSelector.setSelectedPreset(-1);
        gifSelector.setSelectedSavedSlot(-1);
        gifDisplay.updateDisplay();
    }
}

void BopperAudioProcessorEditor::loadSavedGif(int slot)
{
    juce::String path = audioProcessor.getSavedGifPath(slot);
    if (path.isNotEmpty())
    {
        juce::File file(path);
        if (file.existsAsFile() && gifAnimator.loadGif(file))
        {
            currentCustomGifPath = path;
            audioProcessor.setSelectedGifIndex(-1);
            gifSelector.setSelectedPreset(-1);
            gifSelector.setSelectedSavedSlot(slot);
            gifDisplay.updateDisplay();
        }
    }
}

void BopperAudioProcessorEditor::saveCurrentGifToSlot(int slot)
{
    if (currentCustomGifPath.isNotEmpty())
    {
        audioProcessor.setSavedGifPath(slot, currentCustomGifPath);
        gifSelector.updateSavedSlotState(slot, true);
    }
}

void BopperAudioProcessorEditor::enterFullscreen()
{
    if (auto* peer = getPeer())
    {
        normalBounds = getBounds();
        isFullscreen = true;

        // Get the display bounds
        auto displays = juce::Desktop::getInstance().getDisplays();
        auto mainDisplay = displays.getPrimaryDisplay();
        if (mainDisplay != nullptr)
        {
            auto screenBounds = mainDisplay->userArea;
            peer->setBounds(screenBounds, true);
        }

        resized();
        fullscreenButton.setButtonText("Exit");
    }
}

void BopperAudioProcessorEditor::exitFullscreen()
{
    if (auto* peer = getPeer())
    {
        isFullscreen = false;
        peer->setBounds(normalBounds, false);
        resized();
        fullscreenButton.setButtonText("Fullscreen");
    }
}

const std::array<BopperAudioProcessorEditor::PresetGif, 3>& BopperAudioProcessorEditor::getPresetGifs()
{
    // Placeholder - in production, this would return actual embedded GIF data
    static std::array<PresetGif, 3> presets = {{
        {"Cat", nullptr, 0},
        {"Heart", nullptr, 0},
        {"Dance", nullptr, 0},
    }};
    return presets;
}
