#include "GifAnimator.h"

bool GifAnimator::loadGif(const juce::File& file)
{
    auto result = GifLoader::loadFromFile(file);
    if (!result.has_value())
        return false;

    frames = std::move(result->frames);
    width = result->width;
    height = result->height;
    currentFrameIndex = 0;

    return true;
}

bool GifAnimator::loadGif(const void* data, size_t size)
{
    auto result = GifLoader::loadFromMemory(data, size);
    if (!result.has_value())
        return false;

    frames = std::move(result->frames);
    width = result->width;
    height = result->height;
    currentFrameIndex = 0;

    return true;
}

void GifAnimator::loadFrames(std::vector<juce::Image>&& newFrames)
{
    frames = std::move(newFrames);
    if (!frames.empty())
    {
        width = frames[0].getWidth();
        height = frames[0].getHeight();
    }
    else
    {
        width = 0;
        height = 0;
    }
    currentFrameIndex = 0;
}

void GifAnimator::update(double bpm, double ppqPosition, bool isPlaying,
                          int speedDivisor, bool reverse, bool pingPong)
{
    if (frames.empty())
        return;

    if (!isPlaying)
    {
        // When not playing, stay on current frame
        return;
    }

    // Apply speed divisor: divide the ppq position to slow down the animation
    double divisor = static_cast<double>(1 << speedDivisor); // 1, 2, 4, 8, 16
    double adjustedPpq = ppqPosition / divisor;

    // Calculate beat phase (0.0 to 1.0)
    double beatPhase = BpmSync::beatPhase(adjustedPpq);
    currentBeatPhase = beatPhase;

    int totalFrames = static_cast<int>(frames.size());
    int newFrameIndex;

    if (pingPong)
    {
        // Ping-pong: 0->N->0 over one beat cycle
        // First half: forward (0.0-0.5 -> frames 0 to N-1)
        // Second half: backward (0.5-1.0 -> frames N-1 to 0)
        if (beatPhase < 0.5)
        {
            double forwardPhase = beatPhase * 2.0; // 0.0 to 1.0
            newFrameIndex = static_cast<int>(forwardPhase * totalFrames);
        }
        else
        {
            double backwardPhase = (1.0 - beatPhase) * 2.0; // 1.0 to 0.0
            newFrameIndex = static_cast<int>(backwardPhase * totalFrames);
        }
    }
    else if (reverse)
    {
        // Reverse: play from end to beginning
        double reversePhase = 1.0 - beatPhase;
        newFrameIndex = static_cast<int>(reversePhase * totalFrames);
    }
    else
    {
        // Normal forward playback
        newFrameIndex = BpmSync::frameIndexFromPhase(beatPhase, totalFrames);
    }

    currentFrameIndex = std::clamp(newFrameIndex, 0, totalFrames - 1);
}

const juce::Image& GifAnimator::getCurrentFrame() const
{
    if (frames.empty())
        return blankImage;

    return frames[static_cast<size_t>(currentFrameIndex)];
}
