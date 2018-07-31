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

#define USIZE uint32_t
#define SHIFT sizeof(USIZE)*4
#define MASK (1<<SHIFT)-1

std::string offLenToBin(USIZE c);
void mapCodesOffLen(struct nodeU32 *root, int len, std::vector<std::pair<USIZE, int>> &pairOffLenCodeLength);
void calcOffLenCodeLengths(std::vector<std::pair<USIZE, int>> &pairOffLenCodeLength, std::vector<int> &offLenCodeLengths);
void buildOffLenCodes(std::vector<std::pair<USIZE, int>> &pairOffLenCodeLength, std::vector<int> &offLenCodeLengths, std::unordered_map<USIZE, std::pair<std::string, int>> &mapOffLenCodeLength);
std::string WriteDeflateBitString(std::deque<Data> &codeTriples, std::vector<std::pair<char, int>> &pairCharCodeLength, std::vector<int> &charCodeLengths, std::unordered_map<char, std::pair<std::string, int>> &mapCharCodeLength, std::vector<std::pair<USIZE, int>> &pairOffLenCodeLength, std::vector<int> &offLenCodeLengths, std::unordered_map<USIZE, std::pair<std::string, int>> &mapOffLenCodeLength);
void DeflateEncode(std::string filenameIn, std::string filenameOut, int encode=0);
void decompressFile(std::string filenameIn, std::string filenameOut);
#endif