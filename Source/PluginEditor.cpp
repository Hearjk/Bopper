#include "PluginEditor.h"

BeatGIFAudioProcessorEditor::BeatGIFAudioProcessorEditor(BeatGIFAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&lookAndFeel);

    // Title
    titleLabel.setText("BeatGIF", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, BeatGIFLookAndFeel::Colors::text);
    addAndMakeVisible(titleLabel);

    // BPM display
    bpmLabel.setText("BPM: 120", juce::dontSendNotification);
    bpmLabel.setFont(juce::Font(16.0f));
    bpmLabel.setColour(juce::Label::textColourId, BeatGIFLookAndFeel::Colors::highlight);
    bpmLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(bpmLabel);

    // GIF display
    gifDisplay.setAnimator(&gifAnimator);
    addAndMakeVisible(gifDisplay);

    // GIF selector
    gifSelector.onPresetSelected = [this](int index)
    {
        loadPresetGif(index);
        audioProcessor.setSelectedGifIndex(index);
    };

    gifSelector.onUploadClicked = [this]()
    {
        openFileChooser();
    };

    addAndMakeVisible(gifSelector);

    // Load initial preset
    int savedIndex = audioProcessor.getSelectedGifIndex();
    gifSelector.setSelectedPreset(savedIndex);
    loadPresetGif(savedIndex);

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

    setSize(500, 450);
}

BeatGIFAudioProcessorEditor::~BeatGIFAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void BeatGIFAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(BeatGIFLookAndFeel::Colors::background);
}

void BeatGIFAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(16);

    // Header row
    auto headerRow = bounds.removeFromTop(40);
    titleLabel.setBounds(headerRow.removeFromLeft(150));
    bpmLabel.setBounds(headerRow);

    bounds.removeFromTop(12); // spacing

    // GIF display (centered, square-ish)
    auto displayHeight = bounds.getHeight() - 100; // Leave room for selector
    gifDisplay.setBounds(bounds.removeFromTop(displayHeight).reduced(20, 0));

    bounds.removeFromTop(12); // spacing

    // GIF selector
    gifSelector.setBounds(bounds);
}

void BeatGIFAudioProcessorEditor::timerCallback()
{
    // Update BPM display
    double bpm = audioProcessor.getBpm();
    bpmLabel.setText("BPM: " + juce::String(static_cast<int>(bpm)), juce::dontSendNotification);

    // Update animation
    gifAnimator.update(bpm, audioProcessor.getPpqPosition(), audioProcessor.isHostPlaying());

    // Repaint GIF display
    gifDisplay.updateDisplay();
}

void BeatGIFAudioProcessorEditor::loadPresetGif(int index)
{
    // Generate a simple animated pattern based on index
    // In a real implementation, these would be actual embedded GIF resources

    const int frameCount = 8;
    const int size = 200;

    std::vector<juce::Image> frames;

    // Color palette for different presets
    juce::Colour colors[] = {
        juce::Colour(0xFFFF6B6B), // Cat - Red
        juce::Colour(0xFF4ECDC4), // Ball - Teal
        juce::Colour(0xFFFFE66D), // Disco - Yellow
        juce::Colour(0xFF95E1D3), // Wave - Mint
        juce::Colour(0xFFFF6B6B), // Heart - Red
        juce::Colour(0xFF9B59B6)  // Notes - Purple
    };

    juce::Colour baseColor = colors[index % 6];

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
        case 1: // Ball - bouncing ball
        {
            float y = size / 2.0f + bounce * 50.0f;
            float squash = 1.0f - std::abs(bounce) * 0.2f;
            g.setColour(baseColor);
            g.fillEllipse(size / 2.0f - 40, y - 40 * squash,
                          80, 80 * squash);
            // Highlight
            g.setColour(juce::Colours::white.withAlpha(0.5f));
            g.fillEllipse(size / 2.0f - 25, y - 35 * squash, 20, 15);
            break;
        }
        case 2: // Disco - rotating rays
        {
            g.setColour(baseColor);
            g.fillEllipse(size / 2.0f - 30, size / 2.0f - 30, 60, 60);

            float rotation = phase * juce::MathConstants<float>::twoPi;
            for (int ray = 0; ray < 8; ++ray)
            {
                float angle = rotation + ray * juce::MathConstants<float>::pi / 4;
                juce::Path rayPath;
                rayPath.addTriangle(
                    size / 2.0f, size / 2.0f,
                    size / 2.0f + std::cos(angle - 0.1f) * 90,
                    size / 2.0f + std::sin(angle - 0.1f) * 90,
                    size / 2.0f + std::cos(angle + 0.1f) * 90,
                    size / 2.0f + std::sin(angle + 0.1f) * 90);
                g.setColour(baseColor.withAlpha(0.6f));
                g.fillPath(rayPath);
            }
            break;
        }
        case 3: // Wave - waving lines
        {
            for (int i = 0; i < 5; ++i)
            {
                float wavePhase = phase + i * 0.2f;
                float x = 30.0f + i * 35.0f;
                float y = size / 2.0f + std::sin(wavePhase * juce::MathConstants<float>::twoPi) * 40.0f;

                g.setColour(baseColor.withAlpha(0.3f + i * 0.15f));
                g.fillEllipse(x - 15, y - 15, 30, 30);
            }
            break;
        }
        case 4: // Heart - pulsing heart
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
        case 5: // Notes - floating music notes
        {
            for (int note = 0; note < 3; ++note)
            {
                float notePhase = phase + note * 0.33f;
                float x = 50.0f + note * 50.0f;
                float y = size - 50.0f - std::fmod(notePhase, 1.0f) * 120.0f;

                g.setColour(baseColor.withAlpha(1.0f - std::fmod(notePhase, 1.0f) * 0.5f));
                // Note head
                g.fillEllipse(x - 10, y - 8, 20, 16);
                // Stem
                g.fillRect(x + 8, y - 50, 3.0f, 50.0f);
                // Flag
                g.fillRect(x + 8, y - 50, 15.0f, 4.0f);
            }
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

void BeatGIFAudioProcessorEditor::openFileChooser()
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

void BeatGIFAudioProcessorEditor::loadCustomGif(const juce::File& file)
{
    if (gifAnimator.loadGif(file))
    {
        audioProcessor.setCustomGifPath(file.getFullPathName());
        audioProcessor.setSelectedGifIndex(-1); // Custom GIF
        gifDisplay.updateDisplay();
    }
}

const std::array<BeatGIFAudioProcessorEditor::PresetGif, 6>& BeatGIFAudioProcessorEditor::getPresetGifs()
{
    // Placeholder - in production, this would return actual embedded GIF data
    static std::array<PresetGif, 6> presets = {{
        {"Cat", nullptr, 0},
        {"Ball", nullptr, 0},
        {"Disco", nullptr, 0},
        {"Wave", nullptr, 0},
        {"Heart", nullptr, 0},
        {"Notes", nullptr, 0},
    }};
    return presets;
}
