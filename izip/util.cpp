#include "util.h"
#include <bitset>
#include <fstream>
#include <sstream>

std::string bitString;
std::string outBuffer;
uint8_t* inBuffer=0;
size_t filesize=0;

std::string readFileToBufferAsString(std::ifstream &fileIn)
{
    return static_cast<std::stringstream const &>(std::stringstream() << fileIn.rdbuf()).str();
}

uint8_t* readFileAsU8(std::string filenameIn){
    FILE *file = fopen(filenameIn.c_str(), "rb");
    if (!file)
        return 0;

    fseek(file, 0, SEEK_END);
    filesize = ftell(file);
    rewind(file);
    inBuffer = (unsigned char *)malloc(filesize);
    fread(inBuffer, 1, filesize, file);
    fclose(file);
    return inBuffer;
}

std::string charToBin(char c)
{
    return std::bitset<8>(c).to_string();
}

void writeOutFile(std::string filenameOut){
    std::ofstream output(filenameOut, std::ios::out | std::ios::binary);
    unsigned long c;
    while (!bitString.empty())
    {
        std::bitset<8> b(bitString);
        c = b.to_ulong();
        output.write(reinterpret_cast<const char *>(&c), 1);
        bitString.erase(0, 8);
    }
    output.close();
}

unsigned long long getFileSize(std::ifstream &fileIn)
{
    unsigned long long filesize = fileIn.tellg();
    fileIn.seekg(0, std::ios::beg);
    fileIn.clear();
    return filesize;
}

int WriteOutString(std::string bitString, std::string fileOutName)
{
    /*This is made to make sure the binaryString is multiple of 8*/
    while (bitString.size() % 8 != 0)
    {
        bitString += "0";
    }

    std::string outString;
    for (int i = 0; i < bitString.size(); i += 8)
    {
        outString += static_cast<char>(stoi(bitString.substr(i, 8), 0, 2));
    }
    std::ofstream fileOut(fileOutName, std::ios::out | std::ios::binary);
    
    if(fileOut.is_open()){
        fileOut<<outString;
        fileOut.close();
        return 0;
    }else{
        return 1;
    }
    }