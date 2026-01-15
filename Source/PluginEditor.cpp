#include "PluginEditor.h"
#include "BinaryData.h"

BopperAudioProcessorEditor::BopperAudioProcessorEditor(BopperAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&lookAndFeel);

    // Title - thin futuristic font
    titleLabel.setText("BOPPER", juce::dontSendNotification);
    titleLabel.setFont(BopperLookAndFeel::getTechFont(22.0f));
    titleLabel.setColour(juce::Label::textColourId, BopperLookAndFeel::Colors::text);
    addAndMakeVisible(titleLabel);

    // BPM display - thin futuristic font
    bpmLabel.setText("BPM: 120", juce::dontSendNotification);
    bpmLabel.setFont(BopperLookAndFeel::getTechFont(14.0f));
    bpmLabel.setColour(juce::Label::textColourId, BopperLookAndFeel::Colors::neonCyan);
    bpmLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(bpmLabel);

    // Speed slider (0-4 for Normal, Slow, Slower, Even Slower, Slowest)
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

    // Speed label - thin futuristic font
    speedLabel.setFont(BopperLookAndFeel::getTechFont(11.0f));
    speedLabel.setColour(juce::Label::textColourId, BopperLookAndFeel::Colors::textDim);
    speedLabel.setJustificationType(juce::Justification::centred);
    updateSpeedLabel();
    addAndMakeVisible(speedLabel);

    // Effects controls
    reverseButton.setButtonText("REV");
    reverseButton.setClickingTogglesState(true);
    reverseButton.setToggleState(audioProcessor.getReverseEnabled(), juce::dontSendNotification);
    reverseButton.onClick = [this]()
    {
        audioProcessor.setReverseEnabled(reverseButton.getToggleState());
        // Disable ping-pong if reverse is enabled (mutually exclusive)
        if (reverseButton.getToggleState())
        {
            pingPongButton.setToggleState(false, juce::dontSendNotification);
            audioProcessor.setPingPongEnabled(false);
        }
    };
    addAndMakeVisible(reverseButton);

    pingPongButton.setButtonText("PING");
    pingPongButton.setClickingTogglesState(true);
    pingPongButton.setToggleState(audioProcessor.getPingPongEnabled(), juce::dontSendNotification);
    pingPongButton.onClick = [this]()
    {
        audioProcessor.setPingPongEnabled(pingPongButton.getToggleState());
        // Disable reverse if ping-pong is enabled (mutually exclusive)
        if (pingPongButton.getToggleState())
        {
            reverseButton.setToggleState(false, juce::dontSendNotification);
            audioProcessor.setReverseEnabled(false);
        }
    };
    addAndMakeVisible(pingPongButton);

    colorFilterCombo.addItem("None", 1);
    colorFilterCombo.addItem("Invert", 2);
    colorFilterCombo.addItem("Sepia", 3);
    colorFilterCombo.addItem("Cyber", 4);
    colorFilterCombo.addItem("Vapor", 5);
    colorFilterCombo.addItem("Matrix", 6);
    colorFilterCombo.setSelectedId(static_cast<int>(audioProcessor.getColorFilter()) + 1, juce::dontSendNotification);
    colorFilterCombo.onChange = [this]()
    {
        audioProcessor.setColorFilter(static_cast<ColorFilterType>(colorFilterCombo.getSelectedId() - 1));
    };
    addAndMakeVisible(colorFilterCombo);

    pulseButton.setButtonText("PULSE");
    pulseButton.setClickingTogglesState(true);
    pulseButton.setToggleState(audioProcessor.getPulseEnabled(), juce::dontSendNotification);
    pulseButton.onClick = [this]()
    {
        audioProcessor.setPulseEnabled(pulseButton.getToggleState());
    };
    addAndMakeVisible(pulseButton);

    shakeButton.setButtonText("SHAKE");
    shakeButton.setClickingTogglesState(true);
    shakeButton.setToggleState(audioProcessor.getShakeEnabled(), juce::dontSendNotification);
    shakeButton.onClick = [this]()
    {
        audioProcessor.setShakeEnabled(shakeButton.getToggleState());
    };
    addAndMakeVisible(shakeButton);

    // Theater mode button
    theaterButton.setButtonText("Theater");
    theaterButton.onClick = [this]()
    {
        if (isTheaterMode)
            exitTheaterMode();
        else
            enterTheaterMode();
    };
    addAndMakeVisible(theaterButton);

    // Theater mode banner (hidden by default)
    theaterBannerLabel.setColour(juce::Label::backgroundColourId, juce::Colour(0xDD1a1a2e));
    theaterBannerLabel.setVisible(false);
    addAndMakeVisible(theaterBannerLabel);

    // GIF display
    gifDisplay.setAnimator(&gifAnimator);
    addAndMakeVisible(gifDisplay);

    // GIF selector callbacks
    gifSelector.onPresetSelected = [this](int index)
    {
        loadPresetGif(index);
        audioProcessor.setSelectedGifIndex(index);
    };

    gifSelector.onSavedGifSelected = [this](int slot)
    {
        loadSavedGif(slot);
    };

    gifSelector.onUploadToSlot = [this](int slot)
    {
        uploadToSlot(slot);
    };

    gifSelector.onDeleteFromSlot = [this](int slot)
    {
        deleteFromSlot(slot);
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

    // Start timer for UI updates (60fps)
    startTimerHz(60);

    setSize(500, 500);
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
    auto bounds = getLocalBounds();

    if (isTheaterMode)
    {
        // Theater mode: GIF fills area above banner, banner at bottom
        const int bannerHeight = 50;
        auto bannerBounds = bounds.removeFromBottom(bannerHeight);

        // GIF display fills remaining space
        gifDisplay.setBounds(bounds);

        // Hide normal UI elements
        titleLabel.setVisible(false);
        bpmLabel.setVisible(false);
        speedSlider.setVisible(false);
        speedLabel.setVisible(false);
        reverseButton.setVisible(false);
        pingPongButton.setVisible(false);
        colorFilterCombo.setVisible(false);
        pulseButton.setVisible(false);
        shakeButton.setVisible(false);
        gifSelector.setVisible(false);

        // Show theater banner and exit button
        theaterBannerLabel.setVisible(true);
        theaterBannerLabel.setBounds(bannerBounds);

        theaterButton.setVisible(true);
        theaterButton.setButtonText("Exit Theater Mode");
        int buttonWidth = 140;
        int buttonHeight = 32;
        theaterButton.setBounds(
            bannerBounds.getCentreX() - buttonWidth / 2,
            bannerBounds.getCentreY() - buttonHeight / 2,
            buttonWidth, buttonHeight);
        theaterButton.toFront(false); // Bring button above banner
        return;
    }

    // Hide theater banner in normal mode
    theaterBannerLabel.setVisible(false);

    // Normal mode - add padding
    bounds = bounds.reduced(16);

    titleLabel.setVisible(true);
    bpmLabel.setVisible(true);
    speedSlider.setVisible(true);
    speedLabel.setVisible(true);
    reverseButton.setVisible(true);
    pingPongButton.setVisible(true);
    colorFilterCombo.setVisible(true);
    pulseButton.setVisible(true);
    shakeButton.setVisible(true);
    gifSelector.setVisible(true);
    theaterButton.setVisible(true);
    theaterButton.setButtonText("Theater");

    // Header row
    auto headerRow = bounds.removeFromTop(40);
    titleLabel.setBounds(headerRow.removeFromLeft(100));
    theaterButton.setBounds(headerRow.removeFromRight(80));
    bpmLabel.setBounds(headerRow);

    bounds.removeFromTop(8); // spacing

    // Speed control row
    auto speedRow = bounds.removeFromTop(30);
    speedLabel.setBounds(speedRow.removeFromLeft(80));
    speedSlider.setBounds(speedRow.reduced(4, 0));

    bounds.removeFromTop(6); // spacing

    // Effects row
    auto effectsRow = bounds.removeFromTop(28);
    int buttonWidth = 50;
    int spacing = 6;
    reverseButton.setBounds(effectsRow.removeFromLeft(buttonWidth));
    effectsRow.removeFromLeft(spacing);
    pingPongButton.setBounds(effectsRow.removeFromLeft(buttonWidth));
    effectsRow.removeFromLeft(spacing);
    colorFilterCombo.setBounds(effectsRow.removeFromLeft(80));
    effectsRow.removeFromLeft(spacing);
    pulseButton.setBounds(effectsRow.removeFromLeft(55));
    effectsRow.removeFromLeft(spacing);
    shakeButton.setBounds(effectsRow.removeFromLeft(55));

    bounds.removeFromTop(8); // spacing

    // GIF display (full width to align with buttons)
    auto displayHeight = bounds.getHeight() - 90; // Leave room for selector
    gifDisplay.setBounds(bounds.removeFromTop(displayHeight));

    bounds.removeFromTop(12); // spacing

    // GIF selector
    gifSelector.setBounds(bounds);
}

void BopperAudioProcessorEditor::timerCallback()
{
    // Update BPM display
    double bpm = audioProcessor.getBpm();
    bpmLabel.setText("BPM: " + juce::String(static_cast<int>(bpm)), juce::dontSendNotification);

    // Update animation with speed divisor and direction effects
    int speedDiv = audioProcessor.getSpeedDivisor();
    bool reverse = audioProcessor.getReverseEnabled();
    bool pingPong = audioProcessor.getPingPongEnabled();
    gifAnimator.update(bpm, audioProcessor.getPpqPosition(), audioProcessor.isHostPlaying(),
                       speedDiv, reverse, pingPong);

    // Pass effect settings to display
    gifDisplay.setEffects(audioProcessor.getColorFilter(),
                          audioProcessor.getPulseEnabled(),
                          audioProcessor.getShakeEnabled(),
                          gifAnimator.getCurrentBeatPhase());

    // Repaint GIF display
    gifDisplay.updateDisplay();
}

void BopperAudioProcessorEditor::updateSpeedLabel()
{
    int divisor = static_cast<int>(speedSlider.getValue());
    juce::String speedText;
    switch (divisor)
    {
        case 0: speedText = "Normal"; break;
        case 1: speedText = "Slow"; break;
        case 2: speedText = "Slower"; break;
        case 3: speedText = "Even Slower"; break;
        case 4: speedText = "Slowest"; break;
        default: speedText = "Normal"; break;
    }
    speedLabel.setText(speedText, juce::dontSendNotification);
}

void BopperAudioProcessorEditor::loadPresetGif(int index)
{
    if (index < 0 || index >= 3)
        return;

    // Load from embedded binary data
    const char* binaryDataPtrs[] = {
        BinaryData::spongebob_gif,
        BinaryData::gandalf_gif,
        BinaryData::Dance_Band_GIF_gif
    };

    const int binaryDataSizes[] = {
        BinaryData::spongebob_gifSize,
        BinaryData::gandalf_gifSize,
        BinaryData::Dance_Band_GIF_gifSize
    };

    if (gifAnimator.loadGif(binaryDataPtrs[index], static_cast<size_t>(binaryDataSizes[index])))
    {
        gifDisplay.updateDisplay();
        return;
    }

    // Fallback: generate a simple placeholder if load failed
    const int frameCount = 8;
    const int size = 200;
    std::vector<juce::Image> frames;

    juce::Colour baseColor = juce::Colour(0xFF00D4FF); // Cyan placeholder

    for (int frame = 0; frame < frameCount; ++frame)
    {
        juce::Image img(juce::Image::ARGB, size, size, true);
        juce::Graphics g(img);

        float phase = static_cast<float>(frame) / frameCount;
        float bounce = std::sin(phase * juce::MathConstants<float>::twoPi);
        float y = size / 2.0f + bounce * 20.0f;

        g.setColour(baseColor.withAlpha(0.3f));
        g.fillEllipse(size / 2.0f - 60, y - 60, 120, 120);

        g.setColour(baseColor);
        g.setFont(14.0f);
        g.drawText("GIF Not Found", 0, size / 2 - 10, size, 20, juce::Justification::centred);

        frames.push_back(std::move(img));
    }

    gifAnimator.loadFrames(std::move(frames));
    gifDisplay.updateDisplay();
}

void BopperAudioProcessorEditor::uploadToSlot(int slot)
{
    pendingUploadSlot = slot;

    fileChooser = std::make_unique<juce::FileChooser>(
        "Select a GIF file",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        "*.gif");

    auto chooserFlags = juce::FileBrowserComponent::openMode |
                        juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file.existsAsFile() && pendingUploadSlot >= 0)
        {
            // Try to load the GIF first before updating state
            if (gifAnimator.loadGif(file))
            {
                // Save the path to this slot only after successful load
                audioProcessor.setSavedGifPath(pendingUploadSlot, file.getFullPathName());
                gifSelector.updateSavedSlotState(pendingUploadSlot, true);

                // Update selection state
                audioProcessor.setSelectedGifIndex(-1);
                gifSelector.setSelectedPreset(-1);
                gifSelector.setSelectedSavedSlot(pendingUploadSlot);
                gifDisplay.updateDisplay();
                repaint();
            }
        }
        pendingUploadSlot = -1;
    });
}

void BopperAudioProcessorEditor::loadSavedGif(int slot)
{
    juce::String path = audioProcessor.getSavedGifPath(slot);
    if (path.isNotEmpty())
    {
        juce::File file(path);
        if (file.existsAsFile() && gifAnimator.loadGif(file))
        {
            audioProcessor.setSelectedGifIndex(-1);
            gifSelector.setSelectedPreset(-1);
            gifSelector.setSelectedSavedSlot(slot);
            gifDisplay.updateDisplay();
        }
    }
}

void BopperAudioProcessorEditor::deleteFromSlot(int slot)
{
    audioProcessor.setSavedGifPath(slot, "");
    gifSelector.updateSavedSlotState(slot, false);

    // If this slot was selected, go back to first preset
    gifSelector.setSelectedSavedSlot(-1);
    gifSelector.setSelectedPreset(0);
    loadPresetGif(0);
    audioProcessor.setSelectedGifIndex(0);
}

void BopperAudioProcessorEditor::enterTheaterMode()
{
    isTheaterMode = true;
    resized();
}

void BopperAudioProcessorEditor::exitTheaterMode()
{
    isTheaterMode = false;
    resized();
}

const std::array<BopperAudioProcessorEditor::PresetGif, 3>& BopperAudioProcessorEditor::getPresetGifs()
{
    // Preset names - actual files loaded from gifs folder
    static std::array<PresetGif, 3> presets = {{
        {"SpongeBob", nullptr, 0},
        {"Gandalf", nullptr, 0},
        {"Dance Band", nullptr, 0},
    }};
    return presets;
}
