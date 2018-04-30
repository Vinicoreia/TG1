
#include <unordered_map>
#include <vector>
#include <queue>
#include <algorithm>
#include <bitset>
#include <fstream>
#include "huff.h"
#include "util.h"

void mapCodes(struct node *root, int len, std::vector<std::pair<char, int>> &pairSymbCodeLength)
{
    /*Essa função pode ser melhorada pois só precisamos calcular o tamanho do código de cada elemento*/
    if (!root)
        return;

    if (root->leaf)
    {
        pairSymbCodeLength.push_back(std::make_pair(root->code, len));
    }
    mapCodes(root->left, len + 1, pairSymbCodeLength);
    mapCodes(root->right, len + 1, pairSymbCodeLength);
}

void calcCodeLengths(std::vector<std::pair<char, int>> &pairSymbCodeLength, std::vector<int> &codeLengths)
{
    std::sort(pairSymbCodeLength.begin(), pairSymbCodeLength.end(), [](auto &left, auto &right) {
        if (left.second == right.second)
        {
            return left.first < right.first;
        }
        return left.second < right.second;
    });

    for (int i = 0; i < pairSymbCodeLength.size(); i++)
    {
        int index = pairSymbCodeLength[i].second;
        codeLengths[index - 1] += 1;
    }
}

void buildCodes(std::vector<std::pair<char, int>> &pairSymbCodeLength, std::vector<int> &codeLengths, std::unordered_map<char, std::pair<std::string, int>> &mapSymbCodeLength)
{
    std::vector<int> start_code;
    int count, code, nCodes;
    code = 0;

    for (int i = codeLengths.size() - 1; i >= 0; --i)
    {
        if (codeLengths.at(i) != 0)
        {
            count = i + 1;
            break;
        }
    }

    start_code.resize(count);
    start_code[count - 1] = code;
    nCodes = codeLengths[count - 1];
    count--;
    for (int i = count - 1; i >= 0; i--)
    {
        code = code + nCodes;
        code = code >> 1;
        start_code[i] = code;
        nCodes = codeLengths[i];
    }

    int codeLen;
    std::string codeStr;

    for (int i = 0; i < pairSymbCodeLength.size(); i++)
    {
        codeLen = pairSymbCodeLength[i].second - 1;
        std::bitset<30> bs = start_code[codeLen];
        codeStr = bs.to_string().substr(30 - pairSymbCodeLength[i].second);

        mapSymbCodeLength[pairSymbCodeLength[i].first] = make_pair(codeStr, pairSymbCodeLength[i].second);
        start_code[codeLen] += 1;
    }
}

std::string writeHuffmanBitString(std::vector<std::pair<char, int>> &pairSymbCodeLength, std::vector<int> &codeLengths, std::unordered_map<char, std::pair<std::string, int>> &mapSymbCodeLength)
{
    int count = 0;
    std::string huffmanBitString;
    /*HEADER*/

    for (int i = 0; i < codeLengths.size(); i++)
    {
        if (codeLengths[i] == 0)
        {
            huffmanBitString += "0";
        }
        else
        {
            huffmanBitString += "1";
            std::bitset<7> bs = codeLengths[i];
            huffmanBitString.append(bs.to_string());
        }
    }

    for (auto it : pairSymbCodeLength)
    {
        huffmanBitString.append(charToBin(it.first));
    }

    /*FIM DO HEADER*/

    for (int i = 0; i < strBuffer.size(); i++)
    {
        huffmanBitString.append(mapSymbCodeLength[strBuffer[i]].first);
    }

    while (((huffmanBitString.size() + 3) % 8) != 0)
    {
        huffmanBitString += "0";
        count += 1;
    }
    std::bitset<3> bs = count;

    huffmanBitString.insert(0, bs.to_string());
    return huffmanBitString;
}


void huffmanDecode(std::string filenameIn, std::string filenameOut)
{
    bitString.clear();
    readFileAsBinaryString(filenameIn);
    std::string decoding;
    std::string outString;
    std::unordered_map<std::string, char> mapCodeSymb;
    char c;
    int strPointer = 3;
    int addedZeros = stoi(bitString.substr(0, 3), 0, 2);
    bitString.resize(bitString.size() - addedZeros);
    std::vector<int> codeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; /*max CodeLength = 30*/
    std::vector<std::pair<char, int>> pairSymbCodeLength;                                                                      /* maps the codeLength to each symbol*/
    std::unordered_map<char, std::pair<std::string, int>> mapSymbCodeLength;

    for (int i = 0; i < 30; i++)
    {
        if (bitString[strPointer] == '1')
        {
            codeLengths[i] = stoi(bitString.substr(strPointer + 1, 7), 0, 2);
            strPointer += 8;
        }
        else
        {
            strPointer += 1;
        }
    }

    for (int j = 0; j < codeLengths.size(); j++)
    {
        for (int i = 0; i < codeLengths[j]; i++)
        {
            c = stol(bitString.substr(strPointer, 8), 0, 2);
            pairSymbCodeLength.push_back(std::make_pair(c, j + 1));
            strPointer += 8;
        }
    }

    buildCodes(pairSymbCodeLength, codeLengths, mapSymbCodeLength); /*atribui o codigo canonico*/

    for (std::unordered_map<char, std::pair<std::string, int>>::iterator i = mapSymbCodeLength.begin(); i != mapSymbCodeLength.end(); ++i)
    {
        mapCodeSymb[i->second.first] = i->first;
    }

    std::string dec;
    for (std::string::iterator it = bitString.begin() + strPointer; it != bitString.end(); it++)
    {
        decoding += *it;
        try
        {
            outString += mapCodeSymb.at(decoding);
            decoding.clear();
        }
        catch (const std::out_of_range &e)
        {
        }
    }
    /* Falta escrever o output*/
    writeDecodedFile(filenameOut, outString);
}

void HuffmanEncode(std::string filenameIn, std::string filenameOut, int encode)
{
    bitString.clear();
    
    std::vector<std::pair<char, long long>> pairSymbProb = getFrequency(strBuffer);
    std::priority_queue<node *, std::vector<node *>, compare> heap;
    std::vector<std::pair<char, int>> pairSymbCodeLength; /* maps the codeLength to each symbol*/
    std::vector<int> codeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; /*max CodeLength = 30*/
    std::unordered_map<char, std::pair<std::string, int>> mapSymbCodeLength;
    struct node *nLeft, *nRight, *nTop;

    readFileToBufferAsString(filenameIn);

    for(int i=0; i< pairSymbProb.size(); i++){
        heap.push(new node(pairSymbProb[i].first, pairSymbProb[i].second, true));
    }

    while(heap.size()!=1){
        nLeft = heap.top();
        heap.pop();
        nRight = heap.top();
        heap.pop();
        nTop = new node(char(0x1f), nLeft->key_value + nRight->key_value, false);
        nTop->left = nLeft;
        nTop->right = nRight;
        heap.push(nTop);
    }

    mapCodes(heap.top(),0, pairSymbCodeLength); /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
    calcCodeLengths(pairSymbCodeLength, codeLengths); /* calcula o tamanho do codigo de cada char para o codeLengths no header*/
    buildCodes(pairSymbCodeLength, codeLengths,mapSymbCodeLength); /*atribui o codigo canonico*/

    bitString = writeHuffmanBitString(pairSymbCodeLength, codeLengths,mapSymbCodeLength);
    if(encode == 0){
        /*writes to file*/
        writeEncodedFile(filenameOut);
    }else{
        std::cout<<bitString.size()/8;
    }
}