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
    std::cout <<success;
    return 0;
}
