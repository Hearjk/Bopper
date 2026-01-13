#include "GifDisplayComponent.h"
#include "UI/BopperLookAndFeel.h"

GifDisplayComponent::GifDisplayComponent()
{
    setOpaque(false);
}

void GifDisplayComponent::setEffects(ColorFilterType filter, bool pulse, bool shake, double beatPhase)
{
    currentFilter = filter;
    pulseEnabled = pulse;
    shakeEnabled = shake;
    currentBeatPhase = beatPhase;
}

juce::Image GifDisplayComponent::applyColorFilter(const juce::Image& source, ColorFilterType filter)
{
    if (filter == ColorFilterType::None)
        return source;

    juce::Image filtered = source.createCopy();
    juce::Image::BitmapData data(filtered, juce::Image::BitmapData::readWrite);

    for (int y = 0; y < data.height; ++y)
    {
        for (int x = 0; x < data.width; ++x)
        {
            juce::Colour pixel = data.getPixelColour(x, y);

            if (pixel.getAlpha() == 0)
                continue;

            juce::Colour newPixel;

            switch (filter)
            {
                case ColorFilterType::Invert:
                    newPixel = juce::Colour(
                        static_cast<juce::uint8>(255 - pixel.getRed()),
                        static_cast<juce::uint8>(255 - pixel.getGreen()),
                        static_cast<juce::uint8>(255 - pixel.getBlue()),
                        pixel.getAlpha());
                    break;

                case ColorFilterType::Sepia:
                {
                    float r = static_cast<float>(pixel.getRed());
                    float g = static_cast<float>(pixel.getGreen());
                    float b = static_cast<float>(pixel.getBlue());

                    int newR = static_cast<int>(std::min(255.0f, r * 0.393f + g * 0.769f + b * 0.189f));
                    int newG = static_cast<int>(std::min(255.0f, r * 0.349f + g * 0.686f + b * 0.168f));
                    int newB = static_cast<int>(std::min(255.0f, r * 0.272f + g * 0.534f + b * 0.131f));

                    newPixel = juce::Colour(
                        static_cast<juce::uint8>(newR),
                        static_cast<juce::uint8>(newG),
                        static_cast<juce::uint8>(newB),
                        pixel.getAlpha());
                    break;
                }

                case ColorFilterType::Cyberpunk:
                {
                    // Cyan/pink tint - boost cyan and pink channels
                    float r = static_cast<float>(pixel.getRed());
                    float g = static_cast<float>(pixel.getGreen());
                    float b = static_cast<float>(pixel.getBlue());

                    // Shift toward cyan (boost G and B) and pink (boost R)
                    int newR = static_cast<int>(std::min(255.0f, r * 1.1f + 20.0f));
                    int newG = static_cast<int>(std::min(255.0f, g * 0.9f + b * 0.2f));
                    int newB = static_cast<int>(std::min(255.0f, b * 1.2f + 30.0f));

                    newPixel = juce::Colour(
                        static_cast<juce::uint8>(newR),
                        static_cast<juce::uint8>(newG),
                        static_cast<juce::uint8>(newB),
                        pixel.getAlpha());
                    break;
                }

                case ColorFilterType::Vaporwave:
                {
                    // Purple/pink aesthetic
                    float r = static_cast<float>(pixel.getRed());
                    float g = static_cast<float>(pixel.getGreen());
                    float b = static_cast<float>(pixel.getBlue());

                    // Shift toward purple/magenta
                    int newR = static_cast<int>(std::min(255.0f, r * 1.0f + b * 0.3f + 20.0f));
                    int newG = static_cast<int>(std::min(255.0f, g * 0.6f));
                    int newB = static_cast<int>(std::min(255.0f, b * 1.1f + r * 0.2f + 40.0f));

                    newPixel = juce::Colour(
                        static_cast<juce::uint8>(newR),
                        static_cast<juce::uint8>(newG),
                        static_cast<juce::uint8>(newB),
                        pixel.getAlpha());
                    break;
                }

                case ColorFilterType::Matrix:
                {
                    // Green terminal style
                    float luma = pixel.getRed() * 0.299f + pixel.getGreen() * 0.587f + pixel.getBlue() * 0.114f;

                    newPixel = juce::Colour(
                        static_cast<juce::uint8>(luma * 0.2f),
                        static_cast<juce::uint8>(std::min(255.0f, luma * 1.2f)),
                        static_cast<juce::uint8>(luma * 0.3f),
                        pixel.getAlpha());
                    break;
                }

                default:
                    newPixel = pixel;
                    break;
            }

            data.setPixelColour(x, y, newPixel);
        }
    }

    return filtered;
}

void GifDisplayComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Draw background with rounded corners
    g.setColour(BopperLookAndFeel::Colors::surfaceLight);
    g.fillRoundedRectangle(bounds, 12.0f);

    // Draw border
    g.setColour(BopperLookAndFeel::Colors::border);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 12.0f, 1.0f);

    if (gifAnimator != nullptr && gifAnimator->isLoaded())
    {
        juce::Image frame = gifAnimator->getCurrentFrame();

        // Apply color filter if set
        if (currentFilter != ColorFilterType::None)
        {
            frame = applyColorFilter(frame, currentFilter);
        }

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

        // Apply pulse effect (scale on beat)
        if (pulseEnabled)
        {
            float pulseAmount = static_cast<float>(std::sin(currentBeatPhase * juce::MathConstants<double>::twoPi));
            float scale = 1.0f + pulseAmount * 0.08f; // ±8% scale

            auto center = drawArea.getCentre();
            float newWidth = drawArea.getWidth() * scale;
            float newHeight = drawArea.getHeight() * scale;
            drawArea = juce::Rectangle<float>(
                center.x - newWidth / 2,
                center.y - newHeight / 2,
                newWidth,
                newHeight);
        }

        // Apply shake effect (offset on beat)
        if (shakeEnabled)
        {
            float shakePhase = static_cast<float>(currentBeatPhase * juce::MathConstants<double>::twoPi * 4.0);
            float shakeX = std::sin(shakePhase) * 4.0f;
            float shakeY = std::cos(shakePhase * 1.3f) * 3.0f;
            drawArea.translate(shakeX, shakeY);
        }

        // Draw the GIF frame directly without any blending effects
        g.drawImage(frame,
                    drawArea.getX(), drawArea.getY(),
                    drawArea.getWidth(), drawArea.getHeight(),
                    0, 0,
                    frame.getWidth(), frame.getHeight(),
                    false); // false = no interpolation artifacts
    }
    else
    {
        // No GIF loaded - show placeholder text
        g.setColour(BopperLookAndFeel::Colors::textDim);
        g.setFont(16.0f);
        g.drawText("Select a GIF below", bounds, juce::Justification::centred);
    }
}

void GifDisplayComponent::resized()
{
}
