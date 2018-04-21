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
#define DICTSIZE 255
#define LOOKAHEADSIZE 255
#define WINDOWSIZE LOOKAHEADSIZE + DICTSIZE

#define DICTBITS 8
#define LOOKBITS 8
using namespace std::chrono;

std::string filebuffer;
int filesize;
/*O LZ77 vai receber os dados já tratados, cada .cpp deve funcionar atomicamente*/

std::string getChars(int position, int len, std::string buffer){
    std::string chars;
    for(int i=0; i<len; i++){
        chars += buffer[position + i];
    }
    return chars;
}
struct Data
{
    size_t offset;
    std::string match;
    char nextChar;
    Data(size_t offset, std::string match, char nextChar) : offset(offset), match(match), nextChar(nextChar) {}
};

struct Suffix{
    std::string suf;
    size_t pos;
    Suffix(std::string suf, size_t pos): suf(suf), pos(pos){}
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
        size_t hpe=0;/*points to end of hash*/
        void hashDict(); /*creates an unordered_map of the values in the dict to reduce search for the biggest match*/
        void findBestMatch(std::string lookahead);/*This function has to return the Data to the lookahead*/
        std::vector<Suffix> SuffixArray;

        std::deque<Data> triplas;
};

void Dictionary::updateDict(size_t offset){
    dpe += offset;
    if(dpe-dpb > DICTSIZE){
        dpb += offset;
    }
    dictionary = filebuffer.substr(dpb, dpe - dpb);
        hashDict();
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
    if(lpb == filesize){
        lookahead.clear();
        return;
    }
    if(lpe > filesize){
        lpe = filesize;
    }
    lookahead = filebuffer.substr(lpb, lpe-lpb);
}

Lookahead::Lookahead(int filesize){

    // if(filesize*8<=LOOKBITS+DICTBITS+8){
    //     /*if file size is less than the space needed to store a triple then exit*/
    //     exit(1);
    // }

    if (filesize > LOOKAHEADSIZE)
    {
        lpe += LOOKAHEADSIZE;
        lookahead.append(getChars(1, LOOKAHEADSIZE, filebuffer));
    }
    else
    {
        lpe += filesize;
        lookahead.append(getChars(1, filesize-1, filebuffer));
    }
    lpb += 1;
};


Dictionary::Dictionary(){
    dpe += 1;
    dictionary.append(getChars(0, 1, filebuffer));
    triplas.emplace_back(0, filebuffer.substr(0,1), filebuffer[0]);
    hashDict();
};

void Dictionary::hashDict()
{   
    SuffixArray.clear();
    for(size_t i = dpb; i< dpe; i++){
        SuffixArray.push_back({filebuffer.substr(i, dictionary.size()-1-i), i});
    }
    std::sort(SuffixArray.begin(), SuffixArray.end(), [](const Suffix &x, const Suffix &y) { return x.suf < y.suf; });
}

void Dictionary::findBestMatch(std::string lookahead)
{
    matchSz = 1; /*minimum match size is 3*/
    if (lookahead.size() <= 3)
    {
        matchSz +=lookahead.size()-1 ;
        triplas.emplace_back(0, lookahead, lookahead[0]);
        return;
    }
    char a = lookahead[0];
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    int pos=0;
    std::string strMatch0, strMatch1;
    for(auto& suff: SuffixArray){
        if (suff.suf[0] == a){
            break;
        }
        pos++;
    }
    if(pos == SuffixArray.size()){
        /*não encontrou match no suffix array*/
        triplas.emplace_back(0, "", lookahead[0]);
        return;
    }

    // std::cout<<pos;
    // std::cout<<"DICT :"<<dictionary<< "LOOK: "<<lookahead<<std::endl;
    // std::cout<< strMatch1<<"\n\n";
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(t2 - t1).count();
    std::cout << "duration: " << duration << std::endl;

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
    filebuffer = readFileToBuffer(file);
    Lookahead *look = new Lookahead(filesize);
    Dictionary *dict = new Dictionary();
    std::string bitString;
    while (!look->lookahead.empty())
    {
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        dict->findBestMatch(look->lookahead);
        look->updateLook(dict->matchSz);
        dict->updateDict(dict->matchSz);
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(t2 - t1).count();
        // std::cout << "duration: " << duration << std::endl;

    }
    for (int i = 0; i < dict->triplas.size(); i++)
    {
        std::cout << dict->triplas[i].offset << " " << dict->triplas[i].match.size() << " "<< dict->triplas[i].nextChar<<std::endl;
    }

        for (int i = 0; i < dict->triplas.size(); i++)
        {
            if (dict->triplas[i].offset == 0)
            {
                bitString += "0"; /*flag that indicates no compression made*/
                for (int j = 0; j < dict->triplas[i].match.size(); j++)
                {
                    bitString.append(std::bitset<8>(dict->triplas[i].match[j]).to_string());
                }
            }
            else
            {
                bitString += "1";
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
        delete look;
        delete dict;
}


int main(){
    std::ifstream file("teste.txt", std::ios::in | std::ios::binary | std::ios::ate);
    CompressFile(file);

    return 0;
}