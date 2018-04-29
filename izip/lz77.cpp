#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include "lz77.h"
#include <sstream>
#include <chrono>
#include <algorithm>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

using namespace std::chrono;

int DICTSIZE = 0;
int LOOKAHEADSIZE = 0;
int DICTBITS = 0;
int LOOKBITS = 0;
/*O LZ77 vai receber os dados já tratados, cada .cpp deve funcionar atomicamente*/
#define ALPHABET_LEN 256
#define NOT_FOUND patlen
#define max(a, b) ((a < b) ? b : a)

void make_delta1(int *delta1, uint8_t *pat, int32_t patlen) {
    int i;
    /* mark all as NOT FOUND*/
    for (i=0; i < ALPHABET_LEN; i++) {
        delta1[i] = NOT_FOUND;
    }
    /* mark THE ONES THAT ARE PART OF THE PATTERN */
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

void writeLZ77BitString(int offset, std::string match, uint8_t nextChar)
{
    if (offset == 0 or match.size() == 0)
    {
        bitString += "0";
        bitString.append(decimalToBitString(nextChar, 8));
    }
    else if ((match.size() * 9 + 8) < (1 + DICTBITS + LOOKBITS + 8))
    {
        for (int j = 0; j < match.size(); j++)
        {
            bitString += "0"; /*FLAG*/
            bitString.append(decimalToBitString((uint8_t) match[j], 8));
        }
        bitString += "0"; /*FLAG*/
        bitString.append(decimalToBitString(nextChar, 8));
    }
    else
    {
        bitString += "1"; /*FLAG*/
        bitString.append(decimalToBitString(offset, DICTBITS));
        bitString.append(decimalToBitString(match.size(), LOOKBITS));
        bitString.append(decimalToBitString(nextChar, 8));
    }
}

Dictionary::Dictionary(){
    dpe += 1;
    writeLZ77BitString(0, "", u8Buffer[0]);
};



void Dictionary::findBestMatch(int lpb, int lpe)
{
    int i =1;
    std::string match;
    int position=0;
    uint8_t nchar;
    std::pair<uint8_t *, int> p;
    
    if(lpe-lpb==1){
        triplas.emplace_back(0, "", u8Buffer[lpb], 0);
        writeLZ77BitString(0, "", u8Buffer[lpb]);
        matchSz = 1;
        return;        
    }

    p = boyer_moore(u8Buffer + dpb, dpe - dpb, u8Buffer + lpb, lpe-lpb);
    position = (dpe-(dpb+ p.second));    

    if (p.second != -1)
    {
        match.append((const char*)(u8Buffer+dpb+p.second), lpe-lpb);
        nchar = match[match.size() - 1];
        match.pop_back();
        triplas.emplace_back(position, match, nchar, 0);
        writeLZ77BitString(position, match, nchar);
        matchSz = match.size()+1;
        return;
    }

    position = -1;
    match += u8Buffer[lpb];
    
    while(i<lpe-lpb){
        p = boyer_moore(u8Buffer+dpb, dpe-dpb, u8Buffer+lpb, i);   
        if(p.second==-1){
            if(i==1){
                triplas.emplace_back(0, "", u8Buffer[lpb], 0);
                writeLZ77BitString(0, "", u8Buffer[lpb]);
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

            match += u8Buffer[lpb + i];
            /*circbuffer*/
            int circbuffer = dpb+p.second+i;
            if(circbuffer == dpe){
                circbuffer -= (dpe-dpb);
            }
            i++;
            while(u8Buffer[circbuffer]==match[i-1] and i<lpe-lpb){
                match += u8Buffer[lpb + i];
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

void EncodeLZ77(std::string filenameIn, std::string filenameOut)
{
    /*READ FILE*/
    readFileAsU8(filenameIn);
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
    writeEncodedFile(filenameOut);/* i might put this in the main later*/
    free(u8Buffer);
    delete look;
    delete dict;
    return;
}

void DecodeLZ77(std::string filenameIn, std::string filenameOut)
{
    readFileToBufferAsString(filenameIn);
    bitString.clear();

    while (strBuffer.size() > 0)
    {
        bitString.append(charToBin(strBuffer.at(0)));
        strBuffer.erase(strBuffer.begin());
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

    writeDecodedFile(filenameOut, outString);
}

int main(int argc, char *argv[])
{
    DICTSIZE = stoi(std::string(argv[4]));
    DICTBITS = floor(log2(DICTSIZE) + 1);
    LOOKAHEADSIZE = stoi(std::string(argv[5]));
    LOOKBITS = floor(log2(LOOKAHEADSIZE) + 1);
    
    if (std::string(argv[1]) == "1")
    {
        EncodeLZ77(argv[2], argv[3]);
    }else if(std::string(argv[1])== "2"){
    DecodeLZ77(argv[2], argv[3]);
    }
    return 0;
}