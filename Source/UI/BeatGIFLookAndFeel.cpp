#include "BeatGIFLookAndFeel.h"

BeatGIFLookAndFeel::BeatGIFLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, Colors::background);
    setColour(juce::TextButton::buttonColourId, Colors::surface);
    setColour(juce::TextButton::textColourOffId, Colors::text);
    setColour(juce::TextButton::textColourOnId, Colors::highlight);
    setColour(juce::Label::textColourId, Colors::text);
}

void BeatGIFLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                               const juce::Colour& backgroundColour,
                                               bool shouldDrawButtonAsHighlighted,
                                               bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    auto cornerSize = 8.0f;

    juce::Colour baseColour = backgroundColour;

    if (shouldDrawButtonAsDown)
        baseColour = Colors::primary;
    else if (shouldDrawButtonAsHighlighted)
        baseColour = Colors::surfaceLight;
    else if (button.getToggleState())
        baseColour = Colors::primary.withAlpha(0.8f);

    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, cornerSize);

    // Border
    g.setColour(Colors::border);
    g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
}

void BeatGIFLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                         bool shouldDrawButtonAsHighlighted,
                                         bool shouldDrawButtonAsDown)
{
    auto font = getTextButtonFont(button, button.getHeight());
    g.setFont(font);

    juce::Colour textColour = Colors::text;
    if (button.getToggleState() || shouldDrawButtonAsDown)
        textColour = Colors::text;
    else if (shouldDrawButtonAsHighlighted)
        textColour = Colors::highlight;

    g.setColour(textColour);

    auto bounds = button.getLocalBounds();
    g.drawText(button.getButtonText(), bounds, juce::Justification::centred, false);
}

juce::Font BeatGIFLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    return juce::Font(juce::jmin(14.0f, static_cast<float>(buttonHeight) * 0.5f));
}
