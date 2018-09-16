#ifndef IZIP_DEFLATE_H_
#define IZIP_DEFLATE_H_
#include <string>
#include <unordered_map>
#include <iostream>
#include <string>
#include <vector>
#include "util.h"
#include "huff.h"
#include "lz77.h"

#define USIZE uint16_t
#define SHIFT sizeof(USIZE) * 4
#define MASK (1 << SHIFT) - 1

struct DeflateData
{
    USIZE offset;
    USIZE matchLen;
    USIZE firstInMatchRange;
    USIZE firstInOffsetRange;
    USIZE matchExtraBits;
    USIZE offsetExtraBits;
    DeflateData(USIZE offset, USIZE matchLen, USIZE firstInMatchRange, USIZE firstInOffsetRange, USIZE matchExtraBits, USIZE offsetExtraBits) : offset(offset), matchLen(matchLen), firstInMatchRange(firstInMatchRange), firstInOffsetRange(firstInOffsetRange), matchExtraBits(matchExtraBits), offsetExtraBits(offsetExtraBits){}
};


std::string offLenToBin(USIZE c);
void mapCodesUSIZE(
    struct nodeU16 *root,
    int len,
    std::vector<std::pair<USIZE, int>> &pairCodeLength);

void calcUSIZECodeLengths(
    std::vector<std::pair<USIZE, int>> &pairOffLenCodeLength,
    std::vector<int> &offLenCodeLengths);

void buildUSIZECodes(
    std::vector<std::pair<USIZE, int>> &pairOffLenCodeLength,
    std::vector<int> &offLenCodeLengths,
    std::unordered_map<USIZE, std::pair<std::string, int>> &mapOffLenCodeLength);

std::string WriteDeflateBitString(
    std::deque<Data> &codeTriples,
    std::vector<std::pair<USIZE, int>> &pairCharCodeLength,
    std::vector<int> &charCodeLengths,
    std::unordered_map<USIZE, std::pair<std::string, int>> &mapCharCodeLength,
    std::vector<std::pair<USIZE, int>> &pairOffLenCodeLength,
    std::vector<int> &offLenCodeLengths,
    std::unordered_map<USIZE, std::pair<std::string, int>> &mapOffLenCodeLength);

void DeflateEncode(std::string filenameIn, std::string filenameOut, int encode = 0);
void DeflateDecode(std::string filenameIn, std::string filenameOut);
#endif