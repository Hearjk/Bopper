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

void GifAnimator::update(double bpm, double ppqPosition, bool isPlaying)
{
    if (frames.empty())
        return;

    if (!isPlaying)
    {
        // When not playing, stay on current frame or first frame
        return;
    }

    // Calculate beat phase (0.0 to 1.0) and map to frame index
    double beatPhase = BpmSync::beatPhase(ppqPosition);
    int newFrameIndex = BpmSync::frameIndexFromPhase(beatPhase, static_cast<int>(frames.size()));

    currentFrameIndex = newFrameIndex;
}

const juce::Image& GifAnimator::getCurrentFrame() const
{
    if (frames.empty())
        return blankImage;

    return frames[static_cast<size_t>(currentFrameIndex)];
}
