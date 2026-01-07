#pragma once

#include <cmath>
#include <algorithm>

class BpmSync
{
public:
    // Calculate milliseconds per beat
    static double msPerBeat(double bpm)
    {
        if (bpm <= 0.0)
            return 500.0; // Fallback to 120 BPM
        return (60.0 / bpm) * 1000.0;
    }

    // Calculate frame interval for GIF playback
    // Spreads N frames evenly across one beat duration
    static double frameIntervalMs(double bpm, int totalFrames)
    {
        if (totalFrames <= 0)
            return 100.0;
        return msPerBeat(bpm) / static_cast<double>(totalFrames);
    }

    // Get current beat phase (0.0 to 1.0) from PPQ position
    static double beatPhase(double ppqPosition)
    {
        return ppqPosition - std::floor(ppqPosition);
    }

    // Map beat phase to frame index
    static int frameIndexFromPhase(double phase, int totalFrames)
    {
        if (totalFrames <= 0)
            return 0;
        int frameIdx = static_cast<int>(phase * totalFrames);
        return std::clamp(frameIdx, 0, totalFrames - 1);
    }
};
