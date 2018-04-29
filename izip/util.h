#ifndef IZIP_UTIL_H_
#define IZIP_UTIL_H_

#include <iostream>
#include <string>

/* Frequently used  streams*/
extern std::string bitString;
extern std::string outBuffer;
extern uint8_t* inBuffer;
extern size_t filesize;

/*Read file as string to the global InBuffer*/
std::string readFileToBufferAsString(std::ifstream &fileIn);

/* Read file as a uint8_t* */
uint8_t* readFileAsU8(std::string filenameIn);

/*Write the bitString to a file*/
void writeOutFile(std::string filenameOut);


/*Function to cast a char to binary using bitset*/
std::string charToBin(char c);

/*Function to read a entire file to buffer using a static cast, as i always want to treat the whole file as a string i don't need to perform any runtime check*/
std::string readFileToBuffer(std::ifstream &fileIn);

/*Function to get the filesize and return as a long long.*/
unsigned long long getFileSize(std::ifstream &fileIn);

/*Function to complete the bitstring if it's not multiple of 8 (each byte is 8 bits) and write to file*/
int WriteOutString(std::string bitString, std::string fileOutName);

#endif // IZIP_UTIL_H
