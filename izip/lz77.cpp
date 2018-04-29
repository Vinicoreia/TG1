#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include "lz77.h"
#include <bitset>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <stdint.h>
#include <stdlib.h>

using namespace std::chrono;

/*O LZ77 vai receber os dados já tratados, cada .cpp deve funcionar atomicamente*/
#define ALPHABET_LEN 256
#define NOT_FOUND patlen
#define max(a, b) ((a < b) ? b : a)

void make_delta1(int *delta1, uint8_t *pat, int32_t patlen) {
    int i;
    for (i=0; i < ALPHABET_LEN; i++) {
        delta1[i] = NOT_FOUND;
    }
    for (i=0; i < patlen-1; i++) {
        delta1[pat[i]] = patlen-1 - i;
    }
}

int is_prefix(uint8_t *word, int wordlen, int pos) {
    int i;
    int suffixlen = wordlen - pos;
    for (i = 0; i < suffixlen; i++) {
        if (word[i] != word[pos+i]) {
            return 0;
        }
    }
    return 1;
}

int suffix_length(uint8_t *word, int wordlen, int pos) {
    int i;
    for (i = 0; (word[pos-i] == word[wordlen-1-i]) && (i < pos); i++);
    return i;
}

void make_delta2(int *delta2, uint8_t *pat, int32_t patlen) {
    int p;
    int last_prefix_index = patlen-1;
    for (p=patlen-1; p>=0; p--) {
        if (is_prefix(pat, patlen, p+1)) {
            last_prefix_index = p+1;
        }
        delta2[p] = last_prefix_index + (patlen-1 - p);
    }
    for (p=0; p < patlen-1; p++) {
        int slen = suffix_length(pat, patlen, p);
        if (pat[p - slen] != pat[patlen-1 - slen]) {
            delta2[patlen-1 - slen] = patlen-1 - p + slen;
        }
    }
}
std::pair<uint8_t *, int> boyer_moore(uint8_t *string, uint32_t stringlen, uint8_t *pat, uint32_t patlen)
{
    int i;
    int delta1[ALPHABET_LEN];
    int *delta2 = (int *)malloc(patlen * sizeof(int));
    make_delta1(delta1, pat, patlen);
    make_delta2(delta2, pat, patlen);

    // The empty pattern must be considered specially
    if (patlen == 0)
    {
        free(delta2);
        return std::make_pair(string, -1);
    }

    i = patlen - 1;
    while (i < stringlen)
    {
        int j = patlen - 1;
        while (j >= 0 && (string[i] == pat[j]))
        {
            --i;
            --j;
        }
        if (j < 0)
        {
            free(delta2);
            return std::make_pair((string + i + 1), i + 1);
        }

        i += max(delta1[string[i]], delta2[j]);
    }
    free(delta2);
    return std::make_pair(string, -1);
}

void Dictionary::updateDict(size_t offset){
    if(dpe >=filesize)
        return;
    dpe += offset;
    if(dpe>DICTSIZE){
        dpb = dpe-DICTSIZE;
    }
}

void Lookahead::updateLook(size_t offset)
{
    lpe += offset;
    lpb += offset;
    if (lpe >= filesize)
    {
        lpe = filesize;
    }
    if(lpb >= filesize)
    {
        return;
    }
    
}

Lookahead::Lookahead(int filesize){
    lpb = 1;
    lpe = 1;
    if (filesize >= LOOKAHEADSIZE)
    {
        lpe += LOOKAHEADSIZE;
    }
    else
    {
        lpe += filesize;
    }
};

void writeLZ77BitString(int offset, std::string match, char nextChar)
{
    if (offset == 0 or match.size() == 0)
    {
        bitString += "0";
        bitString.append(std::bitset<8>(nextChar).to_string());
    }
    else if ((match.size() * 9 + 8) < (1 + DICTBITS + LOOKBITS + 8))
    {
        for (int j = 0; j < match.size(); j++)
        {
            bitString += "0"; /*FLAG*/
            bitString.append(std::bitset<8>(match[j]).to_string());
        }
        bitString += "0"; /*FLAG*/
        bitString.append(std::bitset<8>(nextChar).to_string());
    }
    else
    {
        bitString += "1"; /*FLAG*/
        bitString.append(std::bitset<DICTBITS>(offset).to_string());
        bitString.append(std::bitset<LOOKBITS>(match.size()).to_string());
        bitString.append(std::bitset<8>(nextChar).to_string());
    }
}

Dictionary::Dictionary(){
    dpe += 1;
    writeLZ77BitString(0, "", inBuffer[0]);
};



void Dictionary::findBestMatch(int lpb, int lpe)
{
    int i =1;
    std::string match;
    int position=0;
    char nchar;
    std::pair<uint8_t *, int> p;
    
    p = boyer_moore(inBuffer + dpb, dpe - dpb, inBuffer + lpb, lpe-lpb);
    position = (dpe-(dpb+ p.second));    

    if (p.second != -1)
    {
        match.append((const char*)(inBuffer+dpb+p.second), lpe-lpb);
        nchar = match[match.size() - 1];
        match.pop_back();
        triplas.emplace_back(position, match, nchar, 0);
        writeLZ77BitString(position, match, nchar);
        matchSz = match.size()+1;
        return;
    }

    position = -1;
    match += inBuffer[lpb];
    
    while(i<lpe-lpb){
        p = boyer_moore(inBuffer+dpb, dpe-dpb, inBuffer+lpb, i);   
        if(p.second==-1){
            if(i==1){
                triplas.emplace_back(0, "", inBuffer[lpb], 0);
                writeLZ77BitString(0, "", inBuffer[lpb]);
                matchSz = 1;
                return;
            }else{
                
                nchar = match[match.size() - 1];
                match.pop_back();
                triplas.emplace_back(position, match, nchar, 0);
                writeLZ77BitString(position, match, nchar);
                matchSz = match.size() + 1;
                return;
            }
        }else{
            /*ve se tem mais match*/

            match += inBuffer[lpb + i];
            /*circbuffer*/
            int circbuffer = dpb+p.second+i;
            if(circbuffer == dpe){
                circbuffer -= (dpe-dpb);
            }
            i++;
            while(inBuffer[circbuffer]==match[i-1] and i<lpe-lpb){
                match += inBuffer[lpb + i];
                i++;
                circbuffer++;
                if (circbuffer == dpe)
                {
                    circbuffer -= (dpe - dpb);
                }
            }
            }
            position = (dpe - (dpb + p.second));
    }

    
    if(match.size()>LOOKAHEADSIZE){
        match.pop_back();
    }
    nchar = match[match.size() - 1];
    match.pop_back();
    triplas.emplace_back(position, match, nchar, 0);
    writeLZ77BitString(position, match, nchar);
    matchSz = match.size()+1;

    /*Se achar aumenta a match e procura novamente a partir da posição q achou*/
    /*Se nao achar retorna a tripla vazia*/
    return;
}

void Encode(std::string filenameIn, std::string filenameOut)
{
    /*READ FILE*/
    inBuffer = readFileAsU8(filenameIn);
    /*Create virtual structures*/

    /*LZ77*/
    Lookahead *look = new Lookahead(filesize);
    Dictionary *dict = new Dictionary();
    while (look->lpb < filesize)
    {
        dict->findBestMatch(look->lpb, look->lpe);
        look->updateLook(dict->matchSz);
        dict->updateDict(dict->matchSz);
    }

    /*Finish bitString*/
    while (bitString.size() % 8 != 0)
    {
        bitString += "0";
    }

    writeOutFile(filenameOut);
    free(inBuffer);
    delete look;
    delete dict;
    return;
}

void decompressFile(std::string filenameIn, std::string filenameOut)
{
    std::ifstream file(filenameIn, std::ios::in | std::ios::binary);
    std::string outBuffer = readFileToBufferAsString(file);
    file.close();

    std::string bitString;

    while (outBuffer.size() > 0)
    {
        bitString.append(charToBin(outBuffer.at(0)));
        outBuffer.erase(outBuffer.begin());
    }
    std::string bitChar;
    std::string lookaheadBits;
    std::string dictBits;
    std::string outString;
    std::string dict;
    int jump;
    int len;
    char nextChar;
    size_t windowPointer = 0;

    while (bitString.size() >= 8)
    {
        if (bitString[0] == '0')
        {
            bitString.erase(0, 1);
            bitChar = bitString.substr(0, 8);
            bitString.erase(0, 8);
            nextChar = static_cast<char>(std::stoi(bitChar, 0, 2));

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
            bitString.erase(0, 1);
            dictBits = bitString.substr(0, DICTBITS);
            lookaheadBits = bitString.substr(DICTBITS, LOOKBITS);
            bitChar = bitString.substr(DICTBITS + LOOKBITS, 8);
            bitString.erase(0, DICTBITS + LOOKBITS + 8);
            nextChar = static_cast<char>(std::stoi(bitChar, 0, 2));
            jump = stoi(dictBits, 0, 2);
            len = stoi(lookaheadBits, 0, 2);
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

    std::ofstream output(filenameOut, std::ios::out | std::ios::binary);
    output << outString; //WRITE TO FILE
    std::cout << "Final filesize after decompressing: " << outString.size() << " bytes" << std::endl;

    output.close();
}

int main(){
    Encode("bee.bmp", "d.bin");
    // decompressFile("d.bin", "C.bmp");
    return 0;
}