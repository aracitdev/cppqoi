#include <iostream>
#include <cppqoi.hpp>


const std::vector<uint8_t> imageData
{
    0xff, 0x00, 0x00,
    0x00, 0xff, 0x00,
    0x00, 0x00, 0xff,
    0xff, 0xff, 0xff
};

int main(int argc, char* argv[])
{
    bool success = cppqoi::WriteQoi("test.qoi", {imageData, 2, 2, 3, 0});

    cppqoi::QoiFile file;
    success = success && cppqoi::LoadQoi("test.qoi", file);
    success =  success && cppqoi::WriteQoi("out.qoi", file);
    std::cout << success <<"\n";



    cppqoi::QoiIStream stream("out.qoi");

    uint32_t width = stream.GetWidth();
    uint32_t height = stream.GetHeight();
    std::cout << width << " "<< height<<"\n";

    while(stream.GetPixelIndex() < width * height)
    {
        cppqoi::Rgba rgba = stream.Get();
        std::cout << ((uint32_t)rgba.r) <<" " <<((uint32_t)rgba.g) <<" " <<
        ((uint32_t)rgba.b) <<" " << ((uint32_t)rgba.a) <<"\n";
    }

    return 0;
}
