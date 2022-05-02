/*

    MIT License

    Copyright (c) 2022 Sean C

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*/


#ifndef CPPQOI_HPP_INCLUDED
#define CPPQOI_HPP_INCLUDED

#include <cstdint>
#include <fstream>
#include <string>
#include <memory>
#include <vector>
#include <filesystem>

#include <iostream>

namespace cppqoi
{

constexpr unsigned char CPPQOI_OP_RGB = 0b11111110; /// Tag for the RGB QOI operation
constexpr unsigned char CPPQOI_OP_RGBA = 0b11111111; /// Tag for the RGBA QOI operation
constexpr unsigned char CPPQOI_OP_INDEX = 0b00000000; /// Tag for the Index QOI operation
constexpr unsigned char CPPQOI_OP_DIFF = 0b01000000; /// Tag for the DIFF QOI operation
constexpr unsigned char CPPQOI_OP_LUMA = 0b10000000; /// Tag for the Luma QOI operation
constexpr unsigned char CPPQOI_OP_RUN = 0b11000000; /// Tag for the Run QOI operation

constexpr uint32_t CPPQOI_HEADER_SIZE = 14; /// Size of the QOI file header
constexpr std::array<uint8_t, 8> CPPQOI_ENDTAG {0, 0, 0, 0, 0, 0, 0, 1}; /// QOI's endtag, marks the end of a  QOI file.
constexpr std::array<uint8_t, 4> CPPQOI_MAGIC {'q', 'o', 'i', 'f'}; /// QOI's magic, identifying a QOI file

/**
  * @brief Represents an RGBA pixel.
  */
class Rgba
{
public:

    /**
      * @brief Default constructor.
      * Initializes r=0, g=0, b=0, a=255
      */
    Rgba() {}

    /**
      * @brief Constructor.
      * @param dr Red value of the pixel.
      * @param dg Green value of the pixel.
      * @param db Blue value of the pixel.
      * @param da Alpha value of the pixel.
      */
    Rgba(uint8_t dr, uint8_t dg, uint8_t db, uint8_t da) : r(dr), g(dg), b(db), a(da) {}

    /**
      * @brief Tests if two pixels are equal.
      * @return True if equal, false otherwise.
      */
    bool operator==(const Rgba& o)
    {
        return r == o.r && g == o.g && b == o.b && a == o.a;
    }

    uint8_t r{0}; /// red value of the pixel
    uint8_t g{0}; /// green value of the pixel
    uint8_t b{0}; /// blue value of the pixel
    uint8_t a{255}; /// alpha value of the pixel
};


/**
  * @brief Struct with all information to create a qoi file.
  */
struct QoiFile
{
    std::vector<uint8_t> pixelData; /// the raw rgb(a) image data
    uint32_t width; /// width of the image (>0)
    uint32_t height; /// height of the image (>0)
    uint8_t channels; /// channels, 3=RGB, 4=RGBA
    uint8_t colorspace; ///colorspace, 0 = sRGB, 1 = linear
};

constexpr uint32_t HashPixel(const Rgba& pix)
{
    return (pix.r * 3 + pix.g * 5 + pix.b * 7 + pix.a * 11);
}

namespace Utility
{

inline void Write32(std::vector<uint8_t>& mem, uint32_t value, size_t& position)
{
    mem[position] = (value >> 24) & 0xff;
    mem[position + 1] = (value >> 16) & 0xff;
    mem[position + 2] = (value >>  8) & 0xff;
    mem[position + 3] = value & 0xff;
    position += 4;
}

inline uint32_t Read32(const std::vector<uint8_t>& mem, size_t& position)
{
    unsigned w = mem[position];
    unsigned x = mem[position + 1];
    unsigned y = mem[position + 2];
    unsigned z = mem[position + 3];

    position += 4;
    return (w << 24) + (x << 16) + (y << 8) + z;
}

}

inline bool LoadQoi(QoiFile& qoi, const std::vector<uint8_t>& buffer)
{
    if(buffer.size() < CPPQOI_HEADER_SIZE + CPPQOI_ENDTAG.size())
        return false; //we can't even read in our header to verify it


    size_t position = 0;
    for(size_t i = 0; i < CPPQOI_MAGIC.size(); i++)
        if(buffer[position++] != CPPQOI_MAGIC[i])
            return false;

    qoi.width = Utility::Read32(buffer, position);
    qoi.height = Utility::Read32(buffer, position);
    qoi.channels = buffer[position++];
    qoi.colorspace = buffer[position++];

    if(qoi.channels < 3 || qoi.channels > 4 || (qoi.colorspace != 0 && qoi.colorspace != 1) ||  qoi.width == 0 || qoi.height == 0)
        return false;

    std::array<Rgba, 64> seen;
    Rgba pixel(0, 0, 0, 255);

    size_t pixelPosition = 0;
    qoi.pixelData.resize(qoi.width * qoi.height * qoi.channels);

    while(pixelPosition < qoi.pixelData.size())
    {
        if(position < (buffer.size() - CPPQOI_ENDTAG.size()))
        {
            uint8_t tag = buffer[position++];
            if(tag == CPPQOI_OP_RGB || tag == CPPQOI_OP_RGBA) //this is just flat loading the pixel
            {
                uint8_t r = buffer[position++];
                uint8_t g = buffer[position++];
                uint8_t b = buffer[position++];
                uint8_t a = (tag == CPPQOI_OP_RGBA) ? buffer[position++] : pixel.a;
                pixel = Rgba(r, g, b, a);
            }
            else
            {
                uint8_t tagOp = (tag & 0b11000000);
                uint8_t tagOperand = (tag & 0b00111111);
                if(tagOp == CPPQOI_OP_INDEX) // 00
                    pixel = seen[tagOperand];
                else if(tagOp == CPPQOI_OP_DIFF) // 01
                {
                    pixel.r += static_cast<uint8_t>( (((tagOperand & 0b110000) >> 4U) & 0x3) - 2);
                    pixel.g += static_cast<uint8_t>( (((tagOperand & 0b001100) >> 2U) & 0x3) - 2);
                    pixel.b += static_cast<uint8_t>( (((tagOperand & 0b000011) >> 0U) & 0x3) - 2);
                }
                else if(tagOp == CPPQOI_OP_LUMA) // 10
                {
                    uint8_t l = buffer[position++];
                    const uint8_t dg = static_cast<uint8_t>( static_cast<unsigned>(tagOperand) - 32);

                    pixel.r += (dg - 8 + static_cast<uint8_t>((l >> 4U)) && 0b00001111U);
                    pixel.g += dg;
                    pixel.b += (dg - 8 + static_cast<uint8_t>((l >> 0U)) && 0b00001111U);
                }
                else if(tagOp == CPPQOI_OP_RUN) // 11
                    for(uint8_t k = 0; k < tagOperand; k++)
                    {
                        qoi.pixelData[pixelPosition++] = pixel.r;
                        qoi.pixelData[pixelPosition++] = pixel.g;
                        qoi.pixelData[pixelPosition++] = pixel.b;
                        if(qoi.channels == 4)
                            qoi.pixelData[pixelPosition++] = pixel.a;
                    }
            }
            seen[HashPixel(pixel) % 64] = pixel;
        }

        qoi.pixelData[pixelPosition++] = pixel.r;
        qoi.pixelData[pixelPosition++] = pixel.g;
        qoi.pixelData[pixelPosition++] = pixel.b;
        if(qoi.channels == 4)
            qoi.pixelData[pixelPosition++] = pixel.a;
    }

    return true;
}

inline bool LoadQoi(std::istream& stream, QoiFile& qoi, size_t dataCount)
{
    std::vector<uint8_t> buffer(dataCount);
    stream.read(reinterpret_cast<char*>(buffer.data()), dataCount);
    return LoadQoi(qoi, buffer);
}

inline bool LoadQoi(const std::string& filename, QoiFile& qoi)
{
    std::ifstream stream(filename.c_str(), std::ifstream::in | std::ifstream::binary);
    if(!stream.is_open())
        return false;
    return LoadQoi(stream, qoi, std::filesystem::file_size(std::filesystem::path{filename}));
}

inline bool WriteQoi(const QoiFile& qoi, std::vector<uint8_t>& buffer)
{
    size_t pixelCount = qoi.width * qoi.height * qoi.channels;
    if(qoi.width == 0 || qoi.height == 0 || qoi.pixelData.size() != qoi.width * qoi.height * qoi.channels || qoi.channels < 3 || qoi.channels > 4 || qoi.colorspace > 1)
        return false;

    size_t bufferSize = qoi.width * qoi.height * (qoi.channels + 1) + CPPQOI_HEADER_SIZE + sizeof(CPPQOI_ENDTAG);
    buffer.resize(bufferSize);
    std::array<Rgba,64> seen;

    size_t position = 0;
    Rgba lastPixel(0, 0, 0, 255);
    Rgba pixel(0, 0, 0,255);

    //Write the header

    for(size_t i = 0; i < CPPQOI_MAGIC.size(); i++)
        buffer[position++] = CPPQOI_MAGIC[i];

    Utility::Write32(buffer, qoi.width, position);
    Utility::Write32(buffer, qoi.height, position);
    buffer[position++] = qoi.channels;
    buffer[position++] = qoi.colorspace;

    uint8_t run = 0;
    for(size_t i = 0; i < pixelCount; i += qoi.channels)
    {
        pixel.r = qoi.pixelData[i];
        pixel.g = qoi.pixelData[i + 1];
        pixel.b = qoi.pixelData[i + 2];

        if(qoi.channels > 3)
            pixel.a = qoi.pixelData[i + 3];

        if(lastPixel == pixel)
        {
            run++;
            if(run == 62 || i == pixelCount - qoi.channels) //we are too far into a run or simply at the end of the file
            {
                buffer[position++] = static_cast<uint8_t>(CPPQOI_OP_RUN | (run - 1));
                run = 0;
            }
        }
        else
        {
            if(run > 0) //we encountered a different pixel during a run, end the run we had going
            {
                buffer[position++] = static_cast<uint8_t>(CPPQOI_OP_RUN | (run - 1));
                run = 0;
            }

            uint8_t pixelHash = static_cast<uint8_t>(HashPixel(pixel) % 64);

            if(seen[pixelHash] == pixel)
                buffer[position++] = CPPQOI_OP_INDEX | pixelHash;
            else
            {
                seen[pixelHash] = pixel;

                if(pixel.a == lastPixel.a)
                {
                    int8_t dr = static_cast<int8_t>(pixel.r - lastPixel.r);
                    int8_t dg = static_cast<int8_t>(pixel.g - lastPixel.g);
                    int8_t db = static_cast<int8_t>(pixel.b - lastPixel.b);

                    int8_t dgr = dr - dg;
                    int8_t dgb = db - dg;

                    if(dr > -3 && dr < 2 && dg > -3 && dg < 2 && db > -3 && db < 2)
                        buffer[position++] = CPPQOI_OP_DIFF | (dr + 2) << 4 | (dg + 2) << 2 | (db + 2);
                    else if(dgr > -9 && dgr < 8 && dg > -33 && dg < 32 && dgb > -9 && dgb < 8)
                    {
                        buffer[position++] = CPPQOI_OP_LUMA | (dg + 32);
                        buffer[position++] = (dgr + 8 ) << 4 | (dgb + 8);
                    }
                    else
                    {
                        buffer[position++] = CPPQOI_OP_RGB;
                        buffer[position++] = pixel.r;
                        buffer[position++] = pixel.g;
                        buffer[position++] = pixel.b;
                    }
                }
                else
                {
                    buffer[position++] = CPPQOI_OP_RGBA;
                    buffer[position++] = pixel.r;
                    buffer[position++] = pixel.g;
                    buffer[position++] = pixel.b;
                    buffer[position++] = pixel.a;
                }
            }
        }
        lastPixel = pixel;
    }

    for(size_t i = 0; i < CPPQOI_ENDTAG.size(); i++)
        buffer[position++] = CPPQOI_ENDTAG[i];
    buffer.resize(position);
    return true;
}


inline bool WriteQoi(std::ostream& out, const QoiFile& qoi)
{
    std::vector<uint8_t> buffer;
    if(!WriteQoi(qoi, buffer))
        return false;
    out.write( reinterpret_cast<char*>(buffer.data()), buffer.size());
    return !out.bad();
}

inline bool WriteQoi(const std::string& filename, const QoiFile& qoi)
{
    std::ofstream stream(filename.c_str(), std::ofstream::out | std::ofstream::binary);
    if(!stream.is_open())
        return false;
    return WriteQoi(stream, qoi);
}


}

#endif // CPPQOI_HPP_INCLUDED
