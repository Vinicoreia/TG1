#include "deflate.h"
#include <algorithm>
#include <queue>
#include <bitset>


/*1- Como aplicar Burrows wheeler (antes e depois, só antes ou só depois)*/
/*2- Como splitar, quantos blocos, a partir de qual tamanho*/
/*3- lembrar do rle definido*/

std::string offLenToBin(USIZE c)
{
    std::string charBin;
    charBin.clear();
    for (int i = 8 * sizeof(USIZE) - 1; i >= 0; --i)
    {
        (c & (1 << i)) ? charBin += '1' : charBin += '0';
    }
    return charBin;
}

void mapCodesOffLen(struct nodeU32 *root, int len, std::vector<std::pair<USIZE, int>> &pairOffLenCodeLength)
{
    if (!root)
        return;

    if (root->leaf)
    {
        pairOffLenCodeLength.push_back(std::make_pair(root->code, len));
    }
    mapCodesOffLen(root->left, len + 1, pairOffLenCodeLength);
    mapCodesOffLen(root->right, len + 1, pairOffLenCodeLength);
}
void calcOffLenCodeLengths(std::vector<std::pair<USIZE, int>> &pairOffLenCodeLength, std::vector<int> &offLenCodeLengths)
{
    std::sort(pairOffLenCodeLength.begin(), pairOffLenCodeLength.end(), [](auto &left, auto &right) {
        if (left.second == right.second)
        {
            return left.first < right.first;
        }
        return left.second < right.second;
    });

    for (int i = 0; i < pairOffLenCodeLength.size(); i++)
    {
        int index = pairOffLenCodeLength[i].second;
        offLenCodeLengths[index - 1] += 1;
    }
}

void buildOffLenCodes(std::vector<std::pair<USIZE, int>> &pairOffLenCodeLength, std::vector<int> &offLenCodeLengths, std::unordered_map<USIZE, std::pair<std::string, int>> &mapOffLenCodeLength)
{
    std::vector<int> start_code;
    int count, code, nCodes;
    code = 0;

    for (int i = static_cast<int>(offLenCodeLengths.size()) - 1; i >= 0; --i)
    {
        if (offLenCodeLengths.at(i) != 0)
        {
            count = i + 1;
            break;
        }
    }

    start_code.resize(count);
    start_code[count - 1] = code;
    nCodes = offLenCodeLengths[count - 1];
    count--;
    for (int i = count - 1; i >= 0; i--)
    {
        code = code + nCodes;
        code = code >> 1;
        start_code[i] = code;
        nCodes = offLenCodeLengths[i];
    }

    int codeLen;
    std::string codeStr;
    for (int i = 0; i < pairOffLenCodeLength.size(); i++)
    {
        codeLen = pairOffLenCodeLength[i].second - 1;
        std::bitset<30> bs = start_code[codeLen];
        codeStr = bs.to_string().substr(30 - pairOffLenCodeLength[i].second);

        mapOffLenCodeLength[pairOffLenCodeLength[i].first] = std::make_pair(codeStr, pairOffLenCodeLength[i].second);
        start_code[codeLen] += 1;
    }
}

std::string WriteDeflateBitString(std::deque<Data> &codeTriples, std::vector<std::pair<char, int>> &pairCharCodeLength, std::vector<int> &charCodeLengths, std::unordered_map<char, std::pair<std::string, int>> &mapCharCodeLength, std::vector<std::pair<USIZE, int>> &pairOffLenCodeLength, std::vector<int> &offLenCodeLengths, std::unordered_map<USIZE, std::pair<std::string, int>> &mapOffLenCodeLength)
{
    int count = 0;
    /*HEADER*/
    std::string out;
    for (int i = 0; i < charCodeLengths.size(); i++)
    {
        if (charCodeLengths[i] == 0)
        {
            out += "0";
        }
        else
        {
            out += "1";
            out.append(decimalToBitString(charCodeLengths[i],8));
        }
    }

    for (auto it : pairCharCodeLength)
    {
        out.append(charToBin(it.first));
    }

    for (int i = 0; i < offLenCodeLengths.size(); i++)
    {
        if (offLenCodeLengths[i] == 0)
        {
            out += "0";
        }
        else
        {
            out += "1";
            out.append(decimalToBitString(offLenCodeLengths[i], 12));
        }
    }

    for (auto it : pairOffLenCodeLength)
    {
        out.append(offLenToBin(it.first));
    }

    /*FIM DO HEADER*/

    USIZE aux;

    for (auto it : codeTriples)
    {
        aux = it.offset;
        aux = aux << SHIFT;
        aux |= it.match.size();
        out.append(mapOffLenCodeLength[aux].first);
        out.append(mapCharCodeLength[it.nextChar].first);
        
    }

    while (((out.size() + 3) % 8) != 0)
    {
        out += "0";
        count += 1;
    }
    std::bitset<3> bs = count;
    out.insert(0, bs.to_string());
    return out;
}

/*Chamar LZ77 ENCODE com flag encode = 1*/

void DeflateEncode(std::string filenameIn, std::string filenameOut, int encode){
    /*LZ77 PART*/
    /*This will put the triples in the strBuffer*/
    std::deque<Data> codeTriples;

    codeTriples = EncodeLZ77(filenameIn, filenameOut, 1);

    std::vector<USIZE> bufferOffLen;
    std::string bufferChar;
    std::vector<std::pair<USIZE, long long>> pairOffLenProb; /*-*/
    std::vector<std::pair<char, long long>> paircharProb;    /*-*/
    std::priority_queue<nodeChar *, std::vector<nodeChar *>, compareChar> heapChar;
    std::priority_queue<nodeU32 *, std::vector<nodeU32 *>, compareU32> heapOffLen;
    std::vector<std::pair<char, int>> pairCharCodeLength;
    std::vector<int> charCodeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<std::pair<USIZE, int>> pairOffLenCodeLength;
    std::vector<int> offLenCodeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::unordered_map<char, std::pair<std::string, int>> mapCharCodeLength;
    std::unordered_map<USIZE, std::pair<std::string, int>> mapOffLenCodeLength;

    USIZE aux = 0;
    for (auto it : codeTriples)
    {
        aux = it.offset;
        aux = aux << SHIFT;
        aux |= it.match.length();
        bufferOffLen.push_back(aux);
        bufferChar.push_back(it.nextChar);
    }

    /*Huffman Part*/
    paircharProb = getFrequencyU8(bufferChar);
    pairOffLenProb = getFrequencyU32(bufferOffLen);
    struct nodeChar *nLeftC, *nRightC, *nTopC;
    for (int i = 0; i < paircharProb.size(); i++)
    {
        heapChar.push(new nodeChar(paircharProb[i].first, paircharProb[i].second, true));
    }

    struct nodeU32 *nLeftOL, *nRightOL, *nTopOL;
    for (int i = 0; i < pairOffLenProb.size(); i++)
    {
        heapOffLen.push(new nodeU32(pairOffLenProb[i].first, pairOffLenProb[i].second, true));
    }

    while (heapChar.size() != 1)
    {
        nLeftC = heapChar.top();
        heapChar.pop();
        nRightC = heapChar.top();
        heapChar.pop();
        nTopC = new nodeChar(char(0x1f), nLeftC->key_value + nRightC->key_value, false);
        nTopC->left = nLeftC;
        nTopC->right = nRightC;
        heapChar.push(nTopC);
    }

    while (heapOffLen.size() != 1)
    {
        nLeftOL = heapOffLen.top();
        heapOffLen.pop();
        nRightOL = heapOffLen.top();
        heapOffLen.pop();
        nTopOL = new nodeU32((USIZE)0x1f, nLeftOL->key_value + nRightOL->key_value, false);
        nTopOL->left = nLeftOL;
        nTopOL->right = nRightOL;
        heapOffLen.push(nTopOL);
    }

    mapCodes(heapChar.top(),0, pairCharCodeLength); /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
    mapCodesOffLen(heapOffLen.top(),0, pairOffLenCodeLength); /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
    calcCodeLengths(pairCharCodeLength, charCodeLengths);
    calcOffLenCodeLengths(pairOffLenCodeLength, offLenCodeLengths);
    buildCodes(pairCharCodeLength, charCodeLengths, mapCharCodeLength);
    buildOffLenCodes(pairOffLenCodeLength, offLenCodeLengths, mapOffLenCodeLength);
    bitString.clear();
    bitString = WriteDeflateBitString(codeTriples, pairCharCodeLength, charCodeLengths, mapCharCodeLength, pairOffLenCodeLength, offLenCodeLengths, mapOffLenCodeLength);
    writeEncodedFile(filenameOut);
}

void decompressFile(std::string filenameIn, std::string filenameOut)
{
    readFileAsBinaryString(filenameIn);
    getWindowSize();
    std::string decoding;
    std::string decoded;
    std::vector<USIZE> bufferOffLen;
    std::string bufferChar;
    std::vector<std::pair<USIZE, long long>> pairOffLenProb; /*-*/
    std::vector<std::pair<char, long long>> paircharProb;    /*-*/
    std::priority_queue<nodeChar *, std::vector<nodeChar *>, compareChar> heapChar;
    std::priority_queue<nodeU32 *, std::vector<nodeU32 *>, compareU32> heapOffLen;
    std::vector<std::pair<char, int>> pairCharCodeLength;
    std::vector<int> charCodeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<std::pair<USIZE, int>> pairOffLenCodeLength;
    std::vector<int> offLenCodeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::unordered_map<char, std::pair<std::string, int>> mapCharCodeLength;
    std::unordered_map<USIZE, std::pair<std::string, int>> mapOffLenCodeLength;

    std::unordered_map<std::string, char> mapCharCode;
    std::unordered_map<std::string, USIZE> mapOffLenCode;

    char c;
    int strPointer = 3;
    int addedZeros = stoi(bitString.substr(0, 3), 0, 2);
    bitString.resize(bitString.size() - addedZeros);

    for (int i = 0; i < 30; i++)
    {
        if (bitString[strPointer] == '1')
        {
            charCodeLengths[i] = stoi(bitString.substr(strPointer + 1, 8), 0, 2);
            strPointer += 9;
        }
        else
        {
            strPointer += 1;
        }
    }
    for (int j = 0; j < charCodeLengths.size(); j++)
    {
        for (int i = 0; i < charCodeLengths[j]; i++)
        {
            c = stol(bitString.substr(strPointer, 8), 0, 2);
            pairCharCodeLength.push_back(std::make_pair(c, j + 1));
            strPointer += 8;
        }
    }

    /*Agora pra Uint*/

    for (int i = 0; i < 30; i++)
    {
        if (bitString[strPointer] == '1')
        {
            offLenCodeLengths[i] = stoi(bitString.substr(strPointer + 1, 12), 0, 2);
            strPointer += 13;
        }
        else
        {
            strPointer += 1;
        }
    }
    USIZE aux;

    for (int j = 0; j < offLenCodeLengths.size(); j++)
    {
        for (int i = 0; i < offLenCodeLengths[j]; i++)
        {
            aux = stoll(bitString.substr(strPointer, 8 * sizeof(USIZE)), 0, 2);
            pairOffLenCodeLength.push_back(std::make_pair(aux, j + 1));
            strPointer += 8 * sizeof(USIZE);
        }
    }

    
    buildCodes(pairCharCodeLength,charCodeLengths, mapCharCodeLength);
    buildOffLenCodes(pairOffLenCodeLength, offLenCodeLengths, mapOffLenCodeLength);

    for (std::unordered_map<char, std::pair<std::string, int>>::iterator i = mapCharCodeLength.begin(); i != mapCharCodeLength.end(); ++i)
    {
        mapCharCode[i->second.first] = i->first;
    }

    for (std::unordered_map<USIZE, std::pair<std::string, int>>::iterator i = mapOffLenCodeLength.begin(); i != mapOffLenCodeLength.end(); ++i)
    {
        mapOffLenCode[i->second.first] = i->first;
    }

    std::deque<char> ch;
    std::deque<USIZE> offLen;
    for (std::string::iterator it = bitString.begin() + strPointer; it != bitString.end(); it++)
    {
        decoding += *it;
        try
        {
            offLen.push_back(mapOffLenCode.at(decoding));
            decoding.clear();
            for (std::string::iterator jt = it + 1; jt != bitString.end(); jt++)
            {
                decoding += *jt;
                try
                {
                    ch.push_back(mapCharCode.at(decoding));
                    decoding.clear();
                    it = jt;
                    break;
                }
                catch (const std::out_of_range &e)
                {
                }
            }
        }
        catch (const std::out_of_range &e)
        {
        }
    }
    decoding.clear();

    std::string bitChar;
    std::string lookaheadBits;
    std::string dictBits;
    std::string outString;
    std::string dict;
    int windowPointer = 0;
    int jump;
    int len;
    char nextChar;
    
    for (int k = 0; k < ch.size(); k++)
    {

        nextChar = ch[k];
        jump = offLen[k] >> SHIFT;
        len = offLen[k] & MASK;

        if (jump == 0)
        {
            outString += nextChar;
            dict += nextChar;
            windowPointer += 1;
            if (dict.size() > DICTSIZE)
            {
                dict.erase(0, 1);
            }
        }
        else
        {
            for (int i = 0; i < len; i++)
            {
                outString += dict[(dict.size() - jump + i) % dict.size()];
            }
            outString += nextChar;
            dict.append(outString.substr(windowPointer, len + 1));
            windowPointer += len + 1;
            if (dict.size() > DICTSIZE)
            {
                dict.erase(0, len + 1);
            }
        }
    }
    writeDecodedFile(filenameOut, outString);
    std::cout << "Final filesize after decompressing: " << outString.size() << " bytes";
}