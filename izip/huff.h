#ifndef IZIP_HUFFMAN_H_
#define IZIP_HUFFMAN_H_
#include "util.h"

struct nodeChar
{
    long long key_value;
    char code;
    bool leaf;
    nodeChar *left;
    nodeChar *right;

    nodeChar(char code, long long key_value, bool leaf)
    {
        this->code = code;
        this->key_value = key_value;
        left = NULL;
        right = NULL;
        this->leaf = leaf;
    }
};

struct compareChar
{
    bool operator()(nodeChar *leftNode, nodeChar *rightNode)
    {
        return (leftNode->key_value > rightNode->key_value);
    }
};

struct nodeU16
{
    long long key_value;
    uint16_t code;
    bool leaf;
    nodeU16 *left;
    nodeU16 *right;

    nodeU16(uint16_t code, long long key_value, bool leaf)
    {
        this->code = code;
        this->key_value = key_value;
        left = NULL;
        right = NULL;
        this->leaf = leaf;
    }
};

struct compareU16
{
    bool operator()(nodeU16 *leftNode, nodeU16 *rightNode)
    {
        return (leftNode->key_value > rightNode->key_value);
    }
};

struct nodeU32
{
    long long key_value;
    uint32_t code;
    bool leaf;
    nodeU32 *left;
    nodeU32 *right;

    nodeU32(uint32_t code, long long key_value, bool leaf)
    {
        this->code = code;
        this->key_value = key_value;
        left = NULL;
        right = NULL;
        this->leaf = leaf;
    }
};

struct compareU32
{
    bool operator()(nodeU32 *leftNode, nodeU32 *rightNode)
    {
        return (leftNode->key_value > rightNode->key_value);
    }
};

void mapCodes(struct nodeChar *root, int len, std::vector<std::pair<char, int>> &pairSymbCodeLength);
void calcCodeLengths(std::vector<std::pair<char, int>> &pairSymbCodeLength, std::vector<int> &codeLengths);
void buildCodes(std::vector<std::pair<char, int>> &pairSymbCodeLength, std::vector<int> &codeLengths, std::unordered_map<char, std::pair<std::string, int>> &mapSymbCodeLength);
std::string writeHuffmanBitString(std::vector<std::pair<char, int>> &pairSymbCodeLength, std::vector<int> &codeLengths, std::unordered_map<char, std::pair<std::string, int>> &mapSymbCodeLength);
void HuffmanDecode(std::string filenameIn, std::string filenameOut);
void HuffmanEncode(std::string filenameIn, std::string filenameOut, int encode=0);
#endif