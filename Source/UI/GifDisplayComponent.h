#pragma once

#include <JuceHeader.h>
#include "GIF/GifAnimator.h"

class GifDisplayComponent : public juce::Component
{
public:
    GifDisplayComponent();

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Set the animator to display
    void setAnimator(GifAnimator* animator) { gifAnimator = animator; }

    // Trigger repaint when frame changes
    void updateDisplay() { repaint(); }

private:
    GifAnimator* gifAnimator = nullptr;
};
