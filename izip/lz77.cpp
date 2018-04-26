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
#include <unistd.h>
#include "boost/algorithm/searching/boyer_moore.hpp"
#include <stdint.h>
#include <stdlib.h>
#define DICTSIZE 32767
#define LOOKAHEADSIZE 255
#define WINDOWSIZE LOOKAHEADSIZE + DICTSIZE

#define DICTBITS 15
#define LOOKBITS 8
using namespace std::chrono;

uint8_t* filebuffer;
int filesize;
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
struct Data
{
    size_t offset;
    std::string match;
    uint8_t nextChar;
    size_t flagFull= 0;
    Data(size_t offset, std::string match, uint8_t nextChar, size_t flagFull) : offset(offset), match(match), nextChar(nextChar), flagFull(flagFull) {}
};

class Dictionary{
    /*The dictionary is responsible to get the biggest match in the dictionary*/
    public:
        std::string dictionary;
        Dictionary();
        void updateDict(size_t offset);
        size_t matchSz = 0;
        size_t dpb = 0; /*Dictionary pointer to begin of Dictionary*/
        size_t dpe = 0; /*Dictionary pointer to end of Dictionary*/
        void findBestMatch(int lpe, int lpb);/*This function has to return the Data to the lookahead*/
        std::deque<Data> triplas;
};

void Dictionary::updateDict(size_t offset){
    dpe += offset;
    if(dpe>DICTSIZE){
        dpb = dpe-DICTSIZE;
    }
}
class Lookahead{
    
    public:
        std::string lookahead;
        Lookahead(int filesize);
        size_t lpb=0; /*lookahead pointer to begin of lookahead*/
        size_t lpe = 0; /*lookahead pointer to end of lookahead*/
        void updateLook(size_t offset);
};

void Lookahead::updateLook(size_t offset)
{
    lpe += offset;
    lpb += offset;
    if (lpe >= filesize)
    {
        lpe = filesize;
    }
    if(lpb == filesize)
    {
        return;
    }
    
}

Lookahead::Lookahead(int filesize){
    lpb = 1;
    lpe = 1;
    if (filesize > LOOKAHEADSIZE)
    {
        lpe += LOOKAHEADSIZE;
    }
    else
    {
        lpe += filesize;
    }
};


Dictionary::Dictionary(){
    dpe += 1;
    triplas.emplace_back(0, "", filebuffer[0],0);
};

void Dictionary::findBestMatch(int lpb, int lpe)
{
    matchSz = 0;
    int i =1;
    std::string match;
    int position=0;
    char nchar;
    /*Usar Boyer-moore pra achar a maior substring*/
    /*first retorna o começo da match*/
    /*second retorna o resto*/
    /*The dictionary is my corpus and the lookahead is the pattern*/
    /*se não achar retorna tripla vazia;*/
    /*se achar tal que é no fim do dicionario, checa se a match é circular*/
    /*se achar longe do fim do dicionario procura proximo*/
    
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    std::pair<uint8_t *, int> p;
    match += filebuffer[lpb];
    while(i<=lpe-lpb){
        p = boyer_moore(filebuffer+dpb, dpe-dpb, filebuffer+lpb, i);
        if(p.second==-1){
            if(i==1){
                triplas.emplace_back(0, "", filebuffer[lpb], 0);
                matchSz = 1;
                high_resolution_clock::time_point t2 = high_resolution_clock::now();
                auto duration = duration_cast<microseconds>(t2 - t1).count();
                return;
            }else{
                nchar = match[match.size() - 1];
                match.pop_back();
                triplas.emplace_back(position, match, nchar, 0);
                matchSz = match.size() + 1;

                high_resolution_clock::time_point t2 = high_resolution_clock::now();
                auto duration = duration_cast<microseconds>(t2 - t1).count();
                return;
            }
        }else{
            /*ve se tem mais match*/

            match += filebuffer[lpb + i];
            /*circbuffer*/
            int circbuffer = p.second+i;
            if(circbuffer == dpe){
                circbuffer -= (dpe-dpb);
            }
            i++;
            while(filebuffer[circbuffer]==match[i-1] and i<lpe-lpb){
                match += filebuffer[lpb + i];
                i++;
                circbuffer++;
                if (circbuffer == dpe)
                {
                    circbuffer -= (dpe - dpb);
                }
            }
            }
            position = dpe-p.second;
        if (match.size() == lpe - lpb and dictionary[dictionary.size() - 1] == match[match.size() - 1])
        {
            nchar = match[match.size() - 1];
            triplas.emplace_back(position, match, '\0', 1);
            matchSz = match.size();

            high_resolution_clock::time_point t2 = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(t2 - t1).count();
            return;
        }
    }
    // std::cout<<"aqui"<<dictionary<<" "<<look;

    match.pop_back();
    nchar = match[match.size() - 1];
    triplas.emplace_back(position, match, '\0', 1);
    matchSz = match.size();

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(t2 - t1).count();
    /*Se achar aumenta a match e procura novamente a partir da posição q achou*/
    /*Se nao achar retorna a tripla vazia*/
    return;
}

std::string readFileToBuffer(std::ifstream &fileIn)
{
    return static_cast<std::stringstream const &>(std::stringstream() << fileIn.rdbuf()).str();
}

unsigned long long getFileSize(std::ifstream &fileIn)
{
    unsigned long long filesize = fileIn.tellg();
    fileIn.seekg(0, std::ios::beg);
    fileIn.clear();
    return filesize;
}

void CompressFile()
{
    FILE *file = fopen("bee.bmp", "rb");
    std::string bitString;
    filebuffer = 0;
    filesize = 0;
    if (!file)
        return;

    fseek(file, 0, SEEK_END);
    filesize = ftell(file);
    rewind(file);
    filebuffer = (unsigned char *)malloc(filesize);
    size_t testsize = fread(filebuffer, 1, filesize, file);
    fclose(file);

    Lookahead *look = new Lookahead(filesize);
    Dictionary *dict = new Dictionary();
    while (look->lpb < filesize)
    {
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        dict->findBestMatch(look->lpb, look->lpe);
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(t2 - t1).count();
        look->updateLook(dict->matchSz);
        dict->updateDict(dict->matchSz);
    }
    for (int i = 0; i < dict->triplas.size(); i++)
    {
        std::cout << dict->triplas[i].offset << " " << dict->triplas[i].match.size() << " " << dict->triplas[i].nextChar << std::endl;
    }
    std::cout << dict->triplas.size() << std::endl;

    // for (int i = 0; i < dict->triplas.size(); i++)
    // {
    //     if(dict->triplas[i].offset == 0){
    //         /*nao teve match adiciona flag 0 e o nextchar*/
    //         bitString+= "0";
    //         bitString.append(std::bitset<8>(dict->triplas[i].nextChar).to_string());
    //     }else if((dict->triplas[i].match.size()*9 + 8) < (1+DICTBITS+LOOKBITS+8)){
    //         /*representar a match com DICTSIZE+DICTBITS nao vale a pena*/
    //         for (int j = 0; j < dict->triplas[i].match.size(); j++)
    //         {
    //             bitString += "0"; /*FLAG*/
    //             bitString.append(std::bitset<8>(dict->triplas[i].match[j]).to_string());
    //         }
    //         bitString += "0"; /*FLAG*/
    //         bitString.append(std::bitset<8>(dict->triplas[i].nextChar).to_string());
    //     }else{
    //         bitString += "1"; /*FLAG*/
    //         bitString.append(std::bitset<DICTBITS>(dict->triplas[i].offset).to_string());
    //         bitString.append(std::bitset<LOOKBITS>(dict->triplas[i].match.size()).to_string());
    //         bitString.append(std::bitset<8>(dict->triplas[i].nextChar).to_string());
    //     }
    // }
    // while (bitString.size() % 8 != 0)
    // {
    //     bitString += "0";
    // }
    // std::cout<<bitString.size()/8;
    // std::ofstream output("c.bin", std::ios::out | std::ios::binary);
    // unsigned long c;
    // while (!bitString.empty())
    // {
    //     std::bitset<8> b(bitString);
    //     c = b.to_ulong();
    //     output.write(reinterpret_cast<const char *>(&c), 1);
    //     bitString.erase(0, 8);
    // }
    // output.close();
    free(filebuffer);
    delete look;
    delete dict;
    return;
}


int main(){
    CompressFile();

    return 0;
}