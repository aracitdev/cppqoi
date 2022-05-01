#include "Bitmap.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <png.h>

Bitmap::Bitmap() : sizeX(0), sizeY(0), colorData()
{
}

Bitmap::Bitmap(size_t x, size_t y)  : sizeX(0), sizeY(0), colorData()
{
    Create(x,y);
}


Bitmap::Bitmap(const std::string& filename)   : sizeX(0), sizeY(0), colorData()
{
    LoadFromFile(filename);
}

void Bitmap::Create(uint32_t szX, uint32_t szY)
{
    sizeX = szX;
    sizeY = szY;
    colorData.resize(szX * szY);
}

bool Bitmap::LoadFromFile(const std::string& filename)
{
    if(std::filesystem::path(filename).extension() == ".bmp")
        return LoadBMP(filename);
    else
    if(std::filesystem::path(filename).extension() == ".png")
        return LoadPNG(filename);
    std::cout <<"Unsuported format: "<<std::filesystem::path(filename).extension()<<"\n";
    return false;
}


bool Bitmap::SaveBMP(const std::string& filename)
{
    std::ofstream out(filename.c_str(), std::ofstream::binary);
    if(!out.is_open())
    {
        std::cout <<"Failed to open output file " <<filename<<"\n";
        return false;
    }

    int padding = (4 - ((sizeX * 3) % 4) ) % 4;
    out.put('B');
    out.put('M');
    Header head;
    InfoHeader info;
    head.fileSize = sizeof(head) + sizeof(info) + sizeof(Color) * colorData.size() + 2 + padding * sizeY;
    head.offset = sizeof(head) + sizeof(info) + 2;
    out.write((char*)(&head), sizeof(head));

    info.width = sizeX;
    info.height = sizeY;
    out.write((char*)(&info), sizeof(info));

    for(int64_t y = 0; y < sizeY; y++)
    {
        for(uint32_t x = 0; x < sizeX; x++)
        {
            Color c = GetPixel(x,y);
            out.write((char*)&c.b, sizeof(c.b));
            out.write((char*)&c.g, sizeof(c.g));
            out.write((char*)&c.r, sizeof(c.r));
            for(int i=0; i < padding; i++)
                out.put('\0');
        }
    }
    return true;
}

bool Bitmap::LoadPNG(const std::string& filename)
{
    int width, height;
    png_byte color_type;
    png_byte bit_depth;
    png_bytep *row_pointers = NULL;
    FILE *fp = fopen(filename.c_str(), "rb");

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png) abort();

    png_infop info = png_create_info_struct(png);
    if(!info) abort();

    if(setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    png_read_info(png, info);

    width      = png_get_image_width(png, info);
    height     = png_get_image_height(png, info);
    color_type = png_get_color_type(png, info);
    bit_depth  = png_get_bit_depth(png, info);
    Create(width, height);if(bit_depth == 16)
    png_set_strip_16(png);

    if(color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
    if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if(png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

      // These color_type don't have an alpha channel then fill it with 0xff.
    if(color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if(color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    if (row_pointers) abort();

    row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for(int y = 0; y < height; y++) {
        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
    }

    png_read_image(png, row_pointers);

    fclose(fp);

    png_destroy_read_struct(&png, &info, NULL);

    for(int y = 0; y < height; y++)
    {
        for(int x=0; x < width; x++)
            SetPixel(x,height-1-y
                     ,row_pointers[y][x * 4], row_pointers[y][x * 4 + 1], row_pointers[y][x * 4 + 2], row_pointers[y][x * 4 + 3]);
        free(row_pointers[y]);
    }
    free(row_pointers);
    return true;

}

bool Bitmap::LoadBMP(const std::string& filename)
{
    std::ifstream in(filename.c_str(), std::ifstream::in | std::ifstream::binary);
    if(!in.is_open())
    {
        std::cout <<"Failed to read bitmap file " <<filename<<"\n";
        colorData.clear();
        sizeX=sizeY=0;
        return false;
    }
    if(in.get() != 'B' || in.get() != 'M')
    {
        std::cout <<"Bitmap header invalid " <<filename<<"\n";
        colorData.clear();
        sizeX=sizeY=0;
        return false; //file header invalid
    }
    Header head;
    InfoHeader info;
    in.read((char*)&head, sizeof(head));
    in.read((char*)&info, sizeof(info));

    if(info.bitsPerPixel != 24 || info.compression)
    {
        std::cout <<"Error: Invalid format. Only supports 24 bit bitmaps with no compression\n";
        colorData.clear();
        sizeX=sizeY=0;
        return false;
    }

    Create(info.width, info.height);

    int padding = (4 - ((sizeX * 3) % 4) ) % 4;

    for(int64_t y = 0; y < sizeY; y++)
    {
        for(uint32_t x = 0; x < sizeX; x++)
        {
            Color c;
            in.read((char*)&c.b, sizeof(c.b));
            in.read((char*)&c.g, sizeof(c.g));
            in.read((char*)&c.r, sizeof(c.r));
            c.a = 255;
            SetPixel(x,y, c);
            for(int i=0; i < padding; i++)
                in.get();
        }
    }
    return true;
}

bool Bitmap::SavePNG(const std::string& filename)
{
    int width      = GetWidth();
    int height     = GetHeight();
    png_bytep *row_pointers = NULL;
    row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for(int y = 0; y < height; y++)
    {
        row_pointers[y] = (png_byte*)malloc(width * 4);
    }

    for(int y = 0; y < height; y++)
    {
        for(int x=0; x < width; x++)
        {
            Color c = GetPixel(x,height-1-y);
            row_pointers[y][x * 4] = c.r;
            row_pointers[y][x * 4 + 1] = c.g;
            row_pointers[y][x * 4 + 2] = c.b;
            row_pointers[y][x * 4 + 3] = c.a;
        }
    }

    FILE *fp = fopen(filename.c_str(), "wb");
    if(!fp) abort();

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) abort();

    png_infop info = png_create_info_struct(png);
    if (!info) abort();

    if (setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    // Output is 8bit depth, RGBA format.
    png_set_IHDR(
    png,
    info,
    width, height,
    8,
    PNG_COLOR_TYPE_RGBA,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);

    // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
    // Use png_set_filler().
    //png_set_filler(png, 0, PNG_FILLER_AFTER);

    if (!row_pointers) abort();

    png_write_image(png, row_pointers);
    png_write_end(png, NULL);

    for(int y = 0; y < height; y++) {
        free(row_pointers[y]);
    }
    free(row_pointers);

    fclose(fp);

    png_destroy_write_struct(&png, &info);

    return true;
}

bool Bitmap::SaveToFile(const std::string& filename)
{
    if(std::filesystem::path(filename).extension() == ".bmp")
        return SaveBMP(filename);
    else
    if(std::filesystem::path(filename).extension() == ".png")
        return SavePNG(filename);
    std::cout <<"Unknown saved bitmap file format " <<std::filesystem::path(filename).extension()<<"\n";
    return false;
}

bool Bitmap::SavePPM(const std::string& filename)
{
    std::ofstream out(filename.c_str());
    if(!out.is_open())
    {
        std::cout <<"Failed to save PPM format " <<filename<<"\n";
        return false;
    }

    out << "P3\n" << sizeX << ' ' << sizeY << "\n255\n";

    for (int32_t j = sizeY-1; j >= 0; --j)
    {
        for (int32_t i = 0; i < (int)sizeX; ++i)
        {
            Color c = GetPixel(i, j);
            out << static_cast<int>(c.r) << ' ' << static_cast<int>(c.g) << ' ' << static_cast<int>(c.b) << '\n';
        }
    }
    return true;
}

Bitmap::Color Bitmap::GetPixel(uint32_t x, uint32_t y)
{
    return colorData[y * sizeX + x];
}

void Bitmap::SetPixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    colorData[y * sizeX + x] = {r,g,b, a};
}

void Bitmap::SetPixel(uint32_t x, uint32_t y, Color c)
{
    colorData[y * sizeX + x] = c;
}

void Bitmap::Clear(uint8_t r, uint8_t g, uint8_t b)
{
    for(size_t i=0; i < colorData.size(); i++)
        colorData[i] = {r,g,b};
}

void Bitmap::Clear(Color c)
{
    for(size_t i=0; i < colorData.size(); i++)
        colorData[i] = c;
}


void Bitmap::GetRaw(std::vector<uint8_t>& rgba)
{
    rgba.resize(colorData.size() * 4);
    size_t i = 0;
    for(size_t y = 1; y < sizeY; y++)
        for(size_t x = 0; x < sizeX; x++)
        {
            Color color = GetPixel(x, sizeY - y);
            rgba[i++] = color.r;
            rgba[i++] = color.g;
            rgba[i++] = color.b;
            rgba[i++] = color.a;
        }
}

void Bitmap::SetRaw(std::vector<uint8_t>& rgba, size_t w, size_t h)
{
    colorData.resize(w * h);
    for(size_t i = 0; i < w * h; i++)
        colorData[i] = { rgba[i * 4 + 0], rgba[i * 4 + 1], rgba[i * 4 + 2], rgba[i * 4 + 3] };
}
