#include "util.h"
#include "deflate.h"
#include "lz77.h"
#include <iostream>
#include <fstream>
#include <string>
    /*
    This is a simple compressor implementation using some ideas of the folowing compressing methods:
    - Deflate
    - Lz77
    - Huffman
    - BWT
    - MTF
    - RLE
    
    */
int main(int argc, char *argv[]){
    std::string fileMode="compress";
    std::string fileName = "teste.txt";
    std::string fileNameOut=fileName;
    std::string fileBuffer;
    std::ifstream file(fileName, std::ios::in | std::ios::binary | std::ios::ate);

    if (file.is_open())
        {
            unsigned long long filesize =  getFileSize(file);
            std::cout << "Your file has : " << filesize << " bytes" << std::endl;
        
            fileBuffer = readFileToBuffer(file);
            if (fileMode == "compress")
            {
            }
            else if (fileMode == "decompress")
            {
            }
            else
            {
                std::cout << "No option named " << fileMode << std::endl;
            }
            file.close();
        }
    else
    {
        std::cout << "Wrong format" << std::endl;
        std::cout << "The format is <compress|decompress filenameIn filenameOut>" << std::endl;
    }

    escreve();
    return 0;
}
