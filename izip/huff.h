#ifndef IZIP_HUFFMAN_H_
#define IZIP_HUFFMAN_H_
#include "util.h"

struct node
{
    long long key_value;
    char code;
    bool leaf;
    node *left;
    node *right;

    node(char code, long long key_value, bool leaf)
    {
        this->code = code;
        this->key_value = key_value;
        left = NULL;
        right = NULL;
        this->leaf = leaf;
    }
};

struct compare
{
    bool operator()(node *leftNode, node *rightNode)
    {
        return (leftNode->key_value > rightNode->key_value);
    }
};
void mapCodes(struct node *root, int len, std::vector<std::pair<char, int>> &pairSymbCodeLength);
void calcCodeLengths(std::vector<std::pair<char, int>> &pairSymbCodeLength, std::vector<int> &codeLengths);
void buildCodes(std::vector<std::pair<char, int>> &pairSymbCodeLength, std::vector<int> &codeLengths, std::unordered_map<char, std::pair<std::string, int>> &mapSymbCodeLength);
std::string writeHuffmanBitString(std::vector<std::pair<char, int>> &pairSymbCodeLength, std::vector<int> &codeLengths, std::unordered_map<char, std::pair<std::string, int>> &mapSymbCodeLength);
void huffmanDecode(std::string fileOutName);
void HuffmanEncode(std::string fileOutName, int encode);
#endif