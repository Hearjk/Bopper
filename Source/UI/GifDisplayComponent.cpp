#include "GifDisplayComponent.h"
#include "UI/BeatGIFLookAndFeel.h"

GifDisplayComponent::GifDisplayComponent()
{
    setOpaque(false);
}

void GifDisplayComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Draw background with rounded corners
    g.setColour(BeatGIFLookAndFeel::Colors::surfaceLight);
    g.fillRoundedRectangle(bounds, 12.0f);

    // Draw border
    g.setColour(BeatGIFLookAndFeel::Colors::border);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 12.0f, 1.0f);

    if (gifAnimator != nullptr && gifAnimator->isLoaded())
    {
        const auto& frame = gifAnimator->getCurrentFrame();

        // Calculate scaled size maintaining aspect ratio
        float gifAspect = static_cast<float>(gifAnimator->getWidth()) /
                          static_cast<float>(gifAnimator->getHeight());
        float boundsAspect = bounds.getWidth() / bounds.getHeight();

        juce::Rectangle<float> drawArea;

        if (gifAspect > boundsAspect)
        {
            // GIF is wider - fit to width
            float scaledWidth = bounds.getWidth() - 20.0f;
            float scaledHeight = scaledWidth / gifAspect;
            drawArea = juce::Rectangle<float>(
                bounds.getCentreX() - scaledWidth / 2,
                bounds.getCentreY() - scaledHeight / 2,
                scaledWidth,
                scaledHeight);
        }
        else
        {
            // GIF is taller - fit to height
            float scaledHeight = bounds.getHeight() - 20.0f;
            float scaledWidth = scaledHeight * gifAspect;
            drawArea = juce::Rectangle<float>(
                bounds.getCentreX() - scaledWidth / 2,
                bounds.getCentreY() - scaledHeight / 2,
                scaledWidth,
                scaledHeight);
        }

        g.drawImage(frame, drawArea,
                    juce::RectanglePlacement::centred);
    }
    else
    {
        // No GIF loaded - show placeholder text
        g.setColour(BeatGIFLookAndFeel::Colors::textDim);
        g.setFont(16.0f);
        g.drawText("Select a GIF below", bounds, juce::Justification::centred);
    }
}

void GifDisplayComponent::resized()
{
}
