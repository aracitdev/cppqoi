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
    return 0;
}
