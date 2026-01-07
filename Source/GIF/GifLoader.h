#pragma once

#include <JuceHeader.h>
#include <vector>
#include <optional>

class GifLoader
{
public:
    struct GifData
    {
        std::vector<juce::Image> frames;
        int width = 0;
        int height = 0;
    };

    // Load GIF from file path
    static std::optional<GifData> loadFromFile(const juce::File& file);

    // Load GIF from memory (for embedded presets)
    static std::optional<GifData> loadFromMemory(const void* data, size_t size);

private:
    static std::optional<GifData> loadGifInternal(const std::string& filePath);
    static std::optional<GifData> loadGifFromMemoryInternal(const void* data, size_t size);
};
