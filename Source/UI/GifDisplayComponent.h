#pragma once

#include <JuceHeader.h>
#include "GIF/GifAnimator.h"
#include "PluginProcessor.h"

class GifDisplayComponent : public juce::Component
{
public:
    GifDisplayComponent();

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Set the animator to display
    void setAnimator(GifAnimator* animator) { gifAnimator = animator; }

    // Set effects for rendering
    void setEffects(ColorFilterType filter, bool pulse, bool shake, double beatPhase);

    // Trigger repaint when frame changes
    void updateDisplay() { repaint(); }

private:
    // Apply color filter to an image
    juce::Image applyColorFilter(const juce::Image& source, ColorFilterType filter);

    GifAnimator* gifAnimator = nullptr;

    // Effect state
    ColorFilterType currentFilter = ColorFilterType::None;
    bool pulseEnabled = false;
    bool shakeEnabled = false;
    double currentBeatPhase = 0.0;
};
