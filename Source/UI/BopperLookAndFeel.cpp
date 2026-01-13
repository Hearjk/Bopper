#include "BopperLookAndFeel.h"

BopperLookAndFeel::BopperLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, Colors::background);
    setColour(juce::TextButton::buttonColourId, Colors::surface);
    setColour(juce::TextButton::textColourOffId, Colors::text);
    setColour(juce::TextButton::textColourOnId, Colors::neonCyan);
    setColour(juce::Label::textColourId, Colors::text);
    setColour(juce::ComboBox::backgroundColourId, Colors::surface);
    setColour(juce::ComboBox::textColourId, Colors::text);
    setColour(juce::ComboBox::outlineColourId, Colors::border);
    setColour(juce::PopupMenu::backgroundColourId, Colors::surface);
    setColour(juce::PopupMenu::textColourId, Colors::text);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, Colors::surfaceBright);
    setColour(juce::PopupMenu::highlightedTextColourId, Colors::neonCyan);
}

void BopperLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                               const juce::Colour& backgroundColour,
                                               bool shouldDrawButtonAsHighlighted,
                                               bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    auto cornerSize = 6.0f;

    // Determine button state colors
    juce::Colour baseColour = Colors::surface;
    juce::Colour glowColour = Colors::neonCyan.withAlpha(0.0f);

    if (shouldDrawButtonAsDown)
    {
        baseColour = Colors::surfaceBright;
        glowColour = Colors::neonCyan.withAlpha(0.4f);
    }
    else if (shouldDrawButtonAsHighlighted)
    {
        baseColour = Colors::surfaceLight;
        glowColour = Colors::neonCyan.withAlpha(0.2f);
    }
    else if (button.getToggleState())
    {
        baseColour = Colors::surfaceBright;
        glowColour = Colors::neonPink.withAlpha(0.3f);
    }

    // Draw glow effect for active/highlighted states
    if (glowColour.getAlpha() > 0)
    {
        g.setColour(glowColour);
        g.fillRoundedRectangle(bounds.expanded(2.0f), cornerSize + 2.0f);
    }

    // Main button background
    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, cornerSize);

    // Border with potential glow
    if (button.getToggleState() || shouldDrawButtonAsDown)
    {
        g.setColour(Colors::neonCyan.withAlpha(0.8f));
    }
    else if (shouldDrawButtonAsHighlighted)
    {
        g.setColour(Colors::neonCyan.withAlpha(0.5f));
    }
    else
    {
        g.setColour(Colors::border);
    }
    g.drawRoundedRectangle(bounds, cornerSize, 1.0f);

    // Inner highlight line at top for depth
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.drawHorizontalLine(static_cast<int>(bounds.getY() + 2),
                         bounds.getX() + cornerSize,
                         bounds.getRight() - cornerSize);
}

void BopperLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                         bool shouldDrawButtonAsHighlighted,
                                         bool shouldDrawButtonAsDown)
{
    auto font = getTextButtonFont(button, button.getHeight());
    g.setFont(font);

    juce::Colour textColour = Colors::text;

    if (button.getToggleState() || shouldDrawButtonAsDown)
    {
        textColour = Colors::neonCyan;
    }
    else if (shouldDrawButtonAsHighlighted)
    {
        textColour = Colors::text.brighter(0.2f);
    }

    g.setColour(textColour);

    auto bounds = button.getLocalBounds();
    g.drawText(button.getButtonText(), bounds, juce::Justification::centred, false);
}

void BopperLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float minSliderPos, float maxSliderPos,
                                          juce::Slider::SliderStyle style, juce::Slider& slider)
{
    auto trackWidth = 6.0f;
    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                          static_cast<float>(width), static_cast<float>(height));

    // Track background
    auto trackBounds = bounds.withSizeKeepingCentre(bounds.getWidth(), trackWidth);
    g.setColour(Colors::sliderTrack);
    g.fillRoundedRectangle(trackBounds, trackWidth / 2.0f);

    // Active track (filled portion)
    auto fillWidth = sliderPos - static_cast<float>(x);
    if (fillWidth > 0)
    {
        auto fillBounds = trackBounds.withWidth(fillWidth);

        // Gradient fill for the active track
        juce::ColourGradient gradient(Colors::neonCyan, fillBounds.getX(), 0,
                                       Colors::neonPink, fillBounds.getRight(), 0, false);
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(fillBounds, trackWidth / 2.0f);

        // Glow effect
        g.setColour(Colors::neonCyan.withAlpha(0.3f));
        g.fillRoundedRectangle(fillBounds.expanded(2.0f), trackWidth / 2.0f + 2.0f);
    }

    // Thumb
    auto thumbRadius = 8.0f;
    auto thumbX = sliderPos - thumbRadius;
    auto thumbY = bounds.getCentreY() - thumbRadius;

    // Thumb glow
    g.setColour(Colors::neonCyan.withAlpha(0.4f));
    g.fillEllipse(thumbX - 3.0f, thumbY - 3.0f, thumbRadius * 2.0f + 6.0f, thumbRadius * 2.0f + 6.0f);

    // Thumb body
    g.setColour(Colors::neonCyan);
    g.fillEllipse(thumbX, thumbY, thumbRadius * 2.0f, thumbRadius * 2.0f);

    // Thumb inner highlight
    g.setColour(juce::Colours::white.withAlpha(0.5f));
    g.fillEllipse(thumbX + 3.0f, thumbY + 2.0f, thumbRadius - 2.0f, thumbRadius - 2.0f);
}

void BopperLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                      int buttonX, int buttonY, int buttonW, int buttonH,
                                      juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float>(0, 0, static_cast<float>(width), static_cast<float>(height));
    auto cornerSize = 6.0f;

    // Background
    g.setColour(isButtonDown ? Colors::surfaceBright : Colors::surface);
    g.fillRoundedRectangle(bounds.reduced(1.0f), cornerSize);

    // Border
    g.setColour(isButtonDown ? Colors::neonCyan : Colors::border);
    g.drawRoundedRectangle(bounds.reduced(1.0f), cornerSize, 1.0f);

    // Arrow
    auto arrowZone = juce::Rectangle<float>(static_cast<float>(width - 20), 0, 15.0f, static_cast<float>(height));
    juce::Path arrow;
    arrow.addTriangle(arrowZone.getCentreX() - 4.0f, arrowZone.getCentreY() - 2.0f,
                      arrowZone.getCentreX() + 4.0f, arrowZone.getCentreY() - 2.0f,
                      arrowZone.getCentreX(), arrowZone.getCentreY() + 4.0f);
    g.setColour(Colors::neonCyan);
    g.fillPath(arrow);
}

void BopperLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    auto bounds = juce::Rectangle<float>(0, 0, static_cast<float>(width), static_cast<float>(height));

    // Background with subtle gradient
    juce::ColourGradient gradient(Colors::surface, 0, 0,
                                   Colors::surfaceLight, 0, static_cast<float>(height), false);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, 6.0f);

    // Border
    g.setColour(Colors::border);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    // Top accent line
    g.setColour(Colors::neonCyan.withAlpha(0.5f));
    g.fillRect(4.0f, 1.0f, static_cast<float>(width - 8), 1.0f);
}

void BopperLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                           bool isSeparator, bool isActive, bool isHighlighted,
                                           bool isTicked, bool hasSubMenu,
                                           const juce::String& text, const juce::String& shortcutKeyText,
                                           const juce::Drawable* icon, const juce::Colour* textColour)
{
    if (isSeparator)
    {
        auto sepArea = area.reduced(8, 0).withHeight(1).withCentre(area.getCentre());
        g.setColour(Colors::border);
        g.fillRect(sepArea);
        return;
    }

    auto bounds = area.reduced(4, 1);

    if (isHighlighted && isActive)
    {
        g.setColour(Colors::surfaceBright);
        g.fillRoundedRectangle(bounds.toFloat(), 4.0f);

        // Accent line on left
        g.setColour(Colors::neonCyan);
        g.fillRect(bounds.getX(), bounds.getY() + 4, 2, bounds.getHeight() - 8);
    }

    // Text
    g.setColour(isHighlighted ? Colors::neonCyan : (isActive ? Colors::text : Colors::textDim));
    g.setFont(getPopupMenuFont());

    auto textArea = bounds.reduced(8, 0);

    if (isTicked)
    {
        // Checkmark
        auto tickArea = textArea.removeFromLeft(20);
        g.setColour(Colors::neonGreen);
        g.drawText(juce::String::charToString(0x2713), tickArea, juce::Justification::centred);
    }

    g.drawText(text, textArea, juce::Justification::centredLeft);
}

juce::Font BopperLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    // Thin, futuristic font
    auto size = juce::jmin(12.0f, static_cast<float>(buttonHeight) * 0.42f);
    return juce::Font("Avenir Next", size, juce::Font::plain).withExtraKerningFactor(0.05f);
}

juce::Font BopperLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return juce::Font("Avenir Next", 12.0f, juce::Font::plain).withExtraKerningFactor(0.05f);
}

juce::Font BopperLookAndFeel::getPopupMenuFont()
{
    return juce::Font("Avenir Next", 12.0f, juce::Font::plain).withExtraKerningFactor(0.03f);
}

juce::Font BopperLookAndFeel::getTechFont(float size)
{
    return juce::Font("Avenir Next", size, juce::Font::plain).withExtraKerningFactor(0.05f);
}
