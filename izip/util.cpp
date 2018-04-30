#include "util.h"
#include <bitset>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
/*GLOBAL VARIABLES*/
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

void readFileAsBinaryString(std::string filename){
    readFileToBufferAsString(filename);
    for (int i = 0; i < strBuffer.size(); i++)
    {
        std::bitset<8> bin(strBuffer.c_str()[i]);
        bitString += bin.to_string();
    }
    strBuffer.clear();
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

int writeStringToFile(std::string fileOutName, std::string outString)
{
    std::ofstream fileOut(fileOutName, std::ios::out | std::ios::binary);

    if (fileOut.is_open())
    {
        fileOut << outString;
        fileOut.close();
        return 0;
    }
    else
    {
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

std::string decimalToBitString(unsigned int n, unsigned int len)
{
    std::string r;
    while (n != 0)
    {
        r = (n % 2 == 0 ? "0" : "1") + r;
        n /= 2;
    }
    while(r.size()<len){
        r.insert(0, "0");
    }
    return r;
}

std::vector<std::pair<char, long long>> getFrequencyU8(std::string buffer)
{
    
    std::unordered_map<char, long long> mapSymbAmount;/*-*/
    std::vector<std::pair<char, long long>> pairSymbProb; /*-*/
    
    for (long long i = 0; i < buffer.length(); i++)
    {
        mapSymbAmount[buffer[i]] += 1;
    }

    for (std::unordered_map<char, long long>::iterator it = mapSymbAmount.begin(); it != mapSymbAmount.end(); ++it)
    {
        pairSymbProb.push_back(std::make_pair((char)(it->first), (long long)(it->second)));
    }

    sort(pairSymbProb.begin(), pairSymbProb.end(), [](auto &left, auto &right) {
        return left.second < right.second;
    });
    sort(pairSymbProb.begin(), pairSymbProb.end(), [](auto &left, auto &right) {
        return left.first < right.first;
    });
    return pairSymbProb;
}

std::vector<std::pair<uint16_t, long long>> getFrequencyU16(std::vector<uint16_t> buffer)
{

    std::unordered_map<uint16_t, long long> mapSymbAmount;    /*-*/
    std::vector<std::pair<uint16_t, long long>> pairSymbProb; /*-*/

    for (long long i = 0; i < buffer.size(); i++)
    {
        mapSymbAmount[buffer[i]] += 1;
    }

    for (std::unordered_map<uint16_t, long long>::iterator it = mapSymbAmount.begin(); it != mapSymbAmount.end(); ++it)
    {
        pairSymbProb.push_back(std::make_pair((char)(it->first), (long long)(it->second)));
    }

    sort(pairSymbProb.begin(), pairSymbProb.end(), [](auto &left, auto &right) {
        return left.second < right.second;
    });
    sort(pairSymbProb.begin(), pairSymbProb.end(), [](auto &left, auto &right) {
        return left.first < right.first;
    });
    return pairSymbProb;
}

std::vector<std::pair<uint32_t, long long>> getFrequencyU32(std::vector<uint32_t> buffer)
{

    std::unordered_map<uint32_t, long long> mapSymbAmount;    /*-*/
    std::vector<std::pair<uint32_t, long long>> pairSymbProb; /*-*/

    for (long long i = 0; i < buffer.size(); i++)
    {
        mapSymbAmount[buffer[i]] += 1;
    }

    for (std::unordered_map<uint32_t, long long>::iterator it = mapSymbAmount.begin(); it != mapSymbAmount.end(); ++it)
    {
        pairSymbProb.push_back(std::make_pair((char)(it->first), (long long)(it->second)));
    }

    sort(pairSymbProb.begin(), pairSymbProb.end(), [](auto &left, auto &right) {
        return left.second < right.second;
    });
    sort(pairSymbProb.begin(), pairSymbProb.end(), [](auto &left, auto &right) {
        return left.first < right.first;
    });
    return pairSymbProb;
}