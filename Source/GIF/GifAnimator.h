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
    // speedDivisor: 0=1x, 1=1/2, 2=1/4, 3=1/8, 4=1/16
    // reverse: play backwards
    // pingPong: play forward then backward
    void update(double bpm, double ppqPosition, bool isPlaying,
                int speedDivisor = 0, bool reverse = false, bool pingPong = false);

    // Get current frame for display
    const juce::Image& getCurrentFrame() const;

    // Check if GIF is loaded
    bool isLoaded() const { return !frames.empty(); }

    // Get frame count
    int getFrameCount() const { return static_cast<int>(frames.size()); }

    // Get dimensions
    int getWidth() const { return width; }
    int getHeight() const { return height; }

    // Get current beat phase (0.0 to 1.0) for effects
    double getCurrentBeatPhase() const { return currentBeatPhase; }

private:
    std::vector<juce::Image> frames;
    int currentFrameIndex = 0;
    int width = 0;
    int height = 0;
    double currentBeatPhase = 0.0;

    // Fallback blank image
    juce::Image blankImage{juce::Image::ARGB, 1, 1, true};
};
