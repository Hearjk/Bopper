#pragma once

#include <JuceHeader.h>

class BopperLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // Modern dark color palette
    struct Colors
    {
        static inline const juce::Colour background{0xFF1A1A2E};   // Deep navy
        static inline const juce::Colour surface{0xFF16213E};      // Slightly lighter
        static inline const juce::Colour surfaceLight{0xFF1F2B47}; // Card background
        static inline const juce::Colour primary{0xFF7B2CBF};      // Purple accent
        static inline const juce::Colour secondary{0xFFE94560};    // Pink accent
        static inline const juce::Colour text{0xFFEEEEEE};         // Off-white
        static inline const juce::Colour textDim{0xFF888888};      // Muted text
        static inline const juce::Colour border{0xFF2D2D44};       // Subtle borders
        static inline const juce::Colour highlight{0xFF00D9FF};    // Cyan highlight
    };

    BopperLookAndFeel();

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
};
