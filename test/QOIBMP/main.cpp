#include <iostream>
#include <cppqoi.hpp>
#include "Bitmap.h"

int main(int argc, char* argv[])
{
    std::string mode;
    std::string inputFile;
    std::string outputFile;

    if(argc < 4)
    {
        std::cout <<"Error: missing arguments...\n";
        std::cout <<"Usage is QOIBMP e/d sourceFile destFile\n";
        std::cout <<"e: encode mode\n";
        std::cout <<"d: decode mode\n";
        //return 0;
        mode = "e";
        inputFile = "testbmp.bmp";
        outputFile = "output.qoi";
    }
    else
    {
        mode = std::string(argv[1]);
        inputFile = std::string(argv[2]);
        outputFile = std::string(argv[3]);
    }

    if(mode == "e")
    {
        std::cout <<"Encoding " << inputFile <<" to " <<outputFile <<" in the qoi format\n";
        Bitmap bmp;
        if(!bmp.LoadFromFile(inputFile))
        {
            std::cout <<"Failed to load " <<inputFile <<"\n";
            return 0;
        }

        std::vector<uint8_t> rgba;
        bmp.GetRaw(rgba);
        cppqoi::WriteQoi(outputFile, {rgba, (uint32_t)bmp.GetWidth(), (uint32_t)bmp.GetHeight(), 4, 1});
    }
    return 0;
}
