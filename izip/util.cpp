#include "util.h"
#include <bitset>
#include <fstream>
#include <sstream>
#include <vector>
std::string bitString;
std::string strBuffer;
std::vector<char> vecBuffer;
uint8_t* u8Buffer=0;
size_t filesize=0;

void readFileToBufferAsString(std::string filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    getFileSize(filename);
    std::cout << "Initial Size: " << filesize << " bytes" << std::endl;    
    strBuffer = static_cast<std::stringstream const &>(std::stringstream() << file.rdbuf()).str();
    file.close();
}

void readFileAsVector(std::string filename){
    char c;
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    getFileSize(filename);
    vecBuffer.reserve(filesize);
    std::cout << "Initial Size: " << filesize << " bytes" << std::endl;    
    while (file.get(c))
    {
       vecBuffer.push_back(c);
    }
    file.close();
}


void readFileAsU8(std::string filenameIn){
    FILE *file = fopen(filenameIn.c_str(), "rb");
    if (!file)
        exit(1);

    getFileSize(filenameIn);
    std::cout << "Initial Size: " << filesize << " bytes" << std::endl;        
    u8Buffer = (unsigned char *)malloc(filesize);
    fread(u8Buffer, 1, filesize, file);
    fclose(file);
}


int writeEncodedFile(std::string fileOutName)
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
        std::cout << "Final filesize after encoding: " << outString.size() << " bytes" << std::endl;
        
        return 0;
    }else{
        return 1;
    }
}

std::string charToBin(char c)
{
    return std::bitset<8>(c).to_string();
}

void writeDecodedFile(std::string filenameOut, std::string outString){
    std::ofstream output(filenameOut, std::ios::out | std::ios::binary);
    output << outString; //WRITE TO FILE
    std::cout << "Final filesize after decoding: " << outString.size() << " bytes" << std::endl;
    output.close();
}
void getFileSize(std::string filename)
{
    struct stat filestatus;
    stat( filename.c_str(), &filestatus );
    filesize = filestatus.st_size;
}
