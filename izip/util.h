#ifndef IZIP_UTIL_H_
#define IZIP_UTIL_H_

#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
/* Frequently used  streams*/
extern std::string bitString;
extern std::string strBuffer;
extern std::vector<char> vecBuffer;
extern uint8_t *u8Buffer;
extern size_t filesize;
/*Read file as string to the global u8Buffer*/
void readFileToBufferAsString(std::string filename);
void readFileAsBinaryString(std::string filename);
void readFileAsVector(std::string filename);
    /*writes the given string to a file*/
    int writeStringToFile(std::string fileOutName, std::string outString);

    /* Cast a integer to binary and returns the bitString corresponding*/
    std::string decimalToBitString(unsigned int n, unsigned int len);

    /* Read file as a uint8_t* */
    void readFileAsU8(std::string filenameIn);

    /*Write the bitString to a file*/
    int writeEncodedFile(std::string fileOutName);
    void writeDecodedFile(std::string filenameOut, std::string outString);

    /*Function to cast a char to binary using bitset*/
    std::string charToBin(char c);

    /*Function to get the filesize and return as a long long.*/
    void getFileSize(std::string filename);

    /* This function gets the frequency associated with the strbuffer*/
    std::vector<std::pair<char, long long>> getFrequencyU8(std::string buffer);
    std::vector<std::pair<uint16_t, long long>> getFrequencyU16(std::vector<uint16_t> buffer);
    std::vector<std::pair<uint32_t, long long>> getFrequencyU32(std::vector<uint32_t> buffer);
    std::vector<std::pair<uint8_t, long long>> getFrequencyU8Vector(std::vector<uint8_t> buffer);


#endif // IZIP_UTIL_H
