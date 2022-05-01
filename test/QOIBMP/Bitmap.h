#ifndef BITMAP_H_INCLUDED
#define BITMAP_H_INCLUDED

#include <string>
#include <vector>

class Bitmap
{
public:

    struct Header
    {
        uint32_t fileSize = 0;
        uint32_t reserved = 0;
        uint32_t offset = 0;
    };

    struct InfoHeader
    {
        uint32_t size = 40;
        uint32_t width = 0;
        uint32_t height = 0;
        uint16_t planes = 1;
        uint16_t bitsPerPixel = 24;
        uint32_t compression = 0;
        uint32_t compressedImageSize = 0;
        uint32_t xPpm = 0;
        uint32_t yPpm = 0;
        uint32_t usedColors = 256;
        uint32_t importantColors = 0;
    };

    struct Color
    {
        uint8_t r,g,b,a;    //alpha is added anyways size wise in the form of byte padding
    };

    Bitmap();
    Bitmap(const std::string& filename);
    Bitmap(size_t x, size_t y);

    void Create(uint32_t szX, uint32_t szY);
    bool LoadFromFile(const std::string& filename);
    bool SaveToFile(const std::string& filename);


    bool SaveBMP(const std::string& filename);
    bool SavePPM(const std::string& filename);
    bool SavePNG(const std::string& filename);

    bool LoadBMP(const std::string& filename);
    bool LoadPNG(const std::string& filename);

    Color GetPixel(uint32_t x, uint32_t y);
    void SetPixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff);
    void SetPixel(uint32_t x, uint32_t y, Color c);

    void Clear(uint8_t r, uint8_t g, uint8_t b);
    void Clear(Color c);

    void GetRaw(std::vector<uint8_t>& rgba);
    void SetRaw(std::vector<uint8_t>& rgba, size_t w, size_t h);

    size_t GetWidth(void) const
    {
        return sizeX;
    }

    size_t GetHeight(void) const
    {
        return sizeY;
    }

private:
    uint32_t sizeX, sizeY;
    std::vector<Color> colorData;
};

#endif // BITMAP_H_INCLUDED
