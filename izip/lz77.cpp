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
#define DICTSIZE 4
#define LOOKAHEADSIZE 10
#define WINDOWSIZE LOOKAHEADSIZE + DICTSIZE

#define DICTBITS 3
#define LOOKBITS 2
using namespace std::chrono;

std::string filebuffer;
int filesize;
/*O LZ77 vai receber os dados já tratados, cada .cpp deve funcionar atomicamente*/
struct Data
{
    size_t offset;
    std::string match;
    char nextChar;
    size_t flagFull= 0;
    Data(size_t offset, std::string match, char nextChar, size_t flagFull) : offset(offset), match(match), nextChar(nextChar), flagFull(flagFull){}
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
    if(dpe-dpb>DICTSIZE)
        dpb+=offset;
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
    std::pair<__gnu_cxx::__normal_iterator<char *, std::__cxx11::basic_string<char>>, __gnu_cxx::__normal_iterator<char *, std::__cxx11::basic_string<char>>> b;
    int position=0;
    char nchar;
    /*Usar Boyer-moore pra achar a maior substring*/
    /*first retorna o começo da match*/
    /*second retorna o resto*/
    /*The dictionary is my corpus and the lookahead is the pattern*/
    /*se não achar retorna tripla vazia;*/
    /*se achar tal que é no fim do dicionario, checa se a match é circular*/
    /*se achar longe do fim do dicionario procura proximo*/
    
    dictionary = filebuffer.substr(dpb, dpe-dpb);
    std::string look = filebuffer.substr(lpb, 255);
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    b = boost::algorithm::boyer_moore_search(dictionary, look);
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(t2 - t1).count();
    if(duration >2000){
        std::cout<<"saindo"<<duration;
        exit(1);
    }
    if(strlen(&b.first[0])!=0){
        triplas.emplace_back(strlen(&b.first[0]), look, look[0], 1);
        matchSz = look.size();
        return;
        }
    look = filebuffer.substr(lpb, 1);
   
    while(i<=lpe-lpb){
        b = boost::algorithm::boyer_moore_search(dictionary, look);
        if(strlen(&b.first[0])==0){
            if(look.size()==1){
                triplas.emplace_back(0, "", look[0], 0);
                matchSz = 1;
                return;
            }else{
                nchar = look[look.size() - 1];
                look.pop_back();
                triplas.emplace_back(position, look, nchar, 0);
                matchSz = look.size()+1;
                return;
            }
        }else{
            /*ve se tem mais match*/
            while(dictionary[(dictionary.size()-strlen(&b.first[0])+i)%dictionary.size()]==look[i-1] and i<lpe-lpb){
                look += filebuffer[lpb + i];
                i++;
            }
            std::cout<<dictionary<<" "<<look<<" "<<i;
        }
        position = strlen(&b.first[0]);
        if(look.size()==lpe-lpb){
            nchar = look[look.size() - 1];
            triplas.emplace_back(position, look, '\0', 1);
            matchSz = look.size();
            return;
        }

            look += filebuffer[lpb + i];
            i++;

    }
    std::cout<<"nao deve entrar aqui";
    exit(1);
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

void CompressFile(std::ifstream &file)
{
    filesize = getFileSize(file);
    std::cout<<filesize<<std::endl;

    filebuffer = readFileToBuffer(file);
    Lookahead *look = new Lookahead(filesize);
    Dictionary *dict = new Dictionary();
    std::string bitString;
    int k=0;

    while (look->lpb <filesize)
    {
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    dict->findBestMatch(look->lpb, look->lpe);
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(t2 - t1).count();
    look->updateLook(dict->matchSz);
    dict->updateDict(dict->matchSz);

    }
    std::cout<<dict->triplas.size()<<std::endl;
    for (int i = 0; i < dict->triplas.size(); i++)
    {
        std::cout <<"TRIPLA: "<< dict->triplas[i].offset << " " << dict->triplas[i].match.size() << " "<< dict->triplas[i].nextChar<<std::endl;
    }
    
        for (int i = 0; i < dict->triplas.size(); i++)
        {
            if(dict->triplas[i].offset == 0){
                /*nao teve match adiciona flag 0 e o nextchar*/
                bitString+= "0";
                bitString.append(std::bitset<8>(dict->triplas[i].nextChar).to_string());
            }else if((dict->triplas[i].match.size()*9 + 8) < (1+DICTBITS+LOOKBITS+8)){
                /*representar a match com DICTSIZE+DICTBITS nao vale a pena*/
                for (int j = 0; j < dict->triplas[i].match.size(); j++)
                {
                    bitString += "0"; /*FLAG*/
                    bitString.append(std::bitset<8>(dict->triplas[i].match[j]).to_string());
                }
                bitString += "0"; /*FLAG*/
                bitString.append(std::bitset<8>(dict->triplas[i].nextChar).to_string());
            }else{
                bitString += "1"; /*FLAG*/
                bitString.append(std::bitset<DICTBITS>(dict->triplas[i].offset).to_string());
                bitString.append(std::bitset<LOOKBITS>(dict->triplas[i].match.size()).to_string());
                bitString.append(std::bitset<8>(dict->triplas[i].nextChar).to_string());
            }
        }
        while (bitString.size() % 8 != 0)
        {
            bitString += "0";
        }
        std::cout<<bitString.size()/8;
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
        delete look;
        delete dict;
}


int main(){
    std::ifstream file("teste.txt", std::ios::in | std::ios::binary | std::ios::ate);
    CompressFile(file);

    return 0;
}