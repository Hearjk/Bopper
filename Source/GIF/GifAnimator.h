#pragma once

#include <JuceHeader.h>
#include "GifLoader.h"
#include "Utils/BpmSync.h"
#include <vector>

class GifAnimator
{
public:
    GifAnimator() = default;

    // Load a new GIF
    bool loadGif(const juce::File& file);
    bool loadGif(const void* data, size_t size);

    // Load frames directly (for programmatic animations)
    void loadFrames(std::vector<juce::Image>&& newFrames);

    // Update animation state based on BPM/PPQ
    void update(double bpm, double ppqPosition, bool isPlaying);

    // Get current frame for display
    const juce::Image& getCurrentFrame() const;

    // Check if GIF is loaded
    bool isLoaded() const { return !frames.empty(); }

    // Get frame count
    int getFrameCount() const { return static_cast<int>(frames.size()); }

    // Get dimensions
    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    std::vector<juce::Image> frames;
    int currentFrameIndex = 0;
    int width = 0;
    int height = 0;

    // Fallback blank image
    juce::Image blankImage{juce::Image::ARGB, 1, 1, true};
};
