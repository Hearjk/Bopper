#pragma once

#include <JuceHeader.h>

class BopperLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // Cyberpunk/Neon aesthetic - vibrant colors against deep dark backgrounds
    struct Colors
    {
        // Base colors - deep space black with subtle blue undertone
        static inline const juce::Colour background{0xFF0A0E17};      // Deep space
        static inline const juce::Colour surface{0xFF131B2E};         // Elevated surface
        static inline const juce::Colour surfaceLight{0xFF1A2540};    // Card/panel background
        static inline const juce::Colour surfaceBright{0xFF243354};   // Highlighted surface

        // Neon accent colors
        static inline const juce::Colour neonCyan{0xFF00F5FF};        // Electric cyan (primary)
        static inline const juce::Colour neonPink{0xFFFF2E97};        // Hot pink (accent)
        static inline const juce::Colour neonGreen{0xFF39FF14};       // Lime green (success)
        static inline const juce::Colour neonOrange{0xFFFF6B35};      // Warm orange (warning)
        static inline const juce::Colour neonPurple{0xFFBF40FF};      // Electric purple

        // Text colors
        static inline const juce::Colour text{0xFFE8F4F8};            // Crisp white-blue
        static inline const juce::Colour textDim{0xFF6B8299};         // Muted blue-gray
        static inline const juce::Colour textGlow{0xFF00F5FF};        // Glowing text

        // UI elements
        static inline const juce::Colour border{0xFF2A3F5F};          // Subtle blue border
        static inline const juce::Colour borderGlow{0xFF00F5FF};      // Glowing border
        static inline const juce::Colour sliderTrack{0xFF1A2540};     // Slider background
        static inline const juce::Colour sliderThumb{0xFF00F5FF};     // Slider thumb
    };

    BopperLookAndFeel();

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle style, juce::Slider& slider) override;

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override;

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted,
                           bool isTicked, bool hasSubMenu,
                           const juce::String& text, const juce::String& shortcutKeyText,
                           const juce::Drawable* icon, const juce::Colour* textColour) override;

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;

    // Custom tech font for labels
    static juce::Font getTechFont(float size);
};
