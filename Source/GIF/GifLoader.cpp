#include "GifLoader.h"
#include "EasyGifReader/EasyGifReader.h"

std::optional<GifLoader::GifData> GifLoader::loadFromFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return std::nullopt;

    return loadGifInternal(file.getFullPathName().toStdString());
}

std::optional<GifLoader::GifData> GifLoader::loadFromMemory(const void* data, size_t size)
{
    return loadGifFromMemoryInternal(data, size);
}

std::optional<GifLoader::GifData> GifLoader::loadGifInternal(const std::string& filePath)
{
    try
    {
        EasyGifReader gif = EasyGifReader::openFile(filePath.c_str());

        GifData data;
        data.width = gif.width();
        data.height = gif.height();

        for (const auto& frame : gif)
        {
            juce::Image img(juce::Image::ARGB, data.width, data.height, true);
            const EasyGifReader::PixelComponent* src = frame.pixels();

            juce::Image::BitmapData bitmap(img, juce::Image::BitmapData::writeOnly);

            for (int y = 0; y < data.height; ++y)
            {
                for (int x = 0; x < data.width; ++x)
                {
                    int srcIdx = (y * data.width + x) * 4;
                    uint8_t r = src[srcIdx + 0];
                    uint8_t g = src[srcIdx + 1];
                    uint8_t b = src[srcIdx + 2];
                    uint8_t a = src[srcIdx + 3];

                    bitmap.setPixelColour(x, y, juce::Colour(r, g, b, a));
                }
            }

            data.frames.push_back(std::move(img));
        }

        if (data.frames.empty())
            return std::nullopt;

        return data;
    }
    catch (...)
    {
        return std::nullopt;
    }
}

std::optional<GifLoader::GifData> GifLoader::loadGifFromMemoryInternal(const void* data, size_t size)
{
    try
    {
        EasyGifReader gif = EasyGifReader::openMemory(data, size);

        GifData gifData;
        gifData.width = gif.width();
        gifData.height = gif.height();

        for (const auto& frame : gif)
        {
            juce::Image img(juce::Image::ARGB, gifData.width, gifData.height, true);
            const EasyGifReader::PixelComponent* src = frame.pixels();

            juce::Image::BitmapData bitmap(img, juce::Image::BitmapData::writeOnly);

            for (int y = 0; y < gifData.height; ++y)
            {
                for (int x = 0; x < gifData.width; ++x)
                {
                    int srcIdx = (y * gifData.width + x) * 4;
                    uint8_t r = src[srcIdx + 0];
                    uint8_t g = src[srcIdx + 1];
                    uint8_t b = src[srcIdx + 2];
                    uint8_t a = src[srcIdx + 3];

                    bitmap.setPixelColour(x, y, juce::Colour(r, g, b, a));
                }
            }

            gifData.frames.push_back(std::move(img));
        }

        if (gifData.frames.empty())
            return std::nullopt;

        return gifData;
    }
    catch (...)
    {
        return std::nullopt;
    }
}
