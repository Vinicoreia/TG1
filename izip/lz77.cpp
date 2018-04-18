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
#include <algorithm>
#define LOOKAHEADSIZE 511
#define DICTSIZE 32767
#define WINDOWSIZE LOOKAHEADSIZE + DICTSIZE

#define LOOKBITS floor(log2(LOOKAHEADSIZE) + 1)
#define DICTBITS floor(log2(DICTSIZE) + 1)

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

class Dictionary{
    /*The dictionary is responsible to get the biggest match in the dictionary*/
    public:
        std::string dictionary;
        Dictionary();
        void updateDict(size_t offset);
        size_t matchSz = 0;
        size_t dpb = 0; /*Dictionary pointer to begin of Dictionary*/
        size_t dpe = 0; /*Dictionary pointer to end of Dictionary*/
        void hashDict(); /*creates an unordered_map of the values in the dict to reduce search for the biggest match*/
        void findBestMatch(std::string lookahead);/*This function has to return the Data to the lookahead*/
        std::unordered_map<char, std::vector<int>> hash;
        std::deque<Data> triplas;
};

void Dictionary::updateDict(size_t offset){
    dpe+= offset;
    if(dpe-dpb > DICTSIZE){
        dpb += offset;
    }
    dictionary.clear();
    for (int i = dpb; i < dpe; i++)
    {
        dictionary += filebuffer[i];
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
    if(lpb == filesize){
        lookahead.clear();
        return;
    }
    if(lpe > filesize){
        lpe = filesize;
    }
    lookahead.clear();
    for(int i = lpb; i<lpe; i++){
        lookahead += filebuffer[i];
    }
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
    triplas.emplace_back(0, "", filebuffer[0]);
    hashDict();
};

void Dictionary::hashDict()
{
    hash.clear();
    for (int i = 0; i < dictionary.size(); i++)
    {
        hash[dictionary[i]].push_back(i);
    }
}

void Dictionary::findBestMatch(std::string lookahead)
{
    std::vector<Data> found;
    char a = lookahead[0];
    std::string strMatch;
    std::vector<int> positions;
    int i;
    try{
        positions = hash.at(a);/* initial position to search*/
    }catch(const std::out_of_range& e){
        /*Caso não tenha a letra no hash retorna 0,"",letra*/
        triplas.emplace_back(0, "", a);
        return;
    }

    for(auto &pos : positions){
        strMatch.clear();
        i=0;
        while (dictionary[(pos + i) % dictionary.size()] == lookahead[i] and i < lookahead.size() - 1) /*we can only go as far as the penultimate position*/
        {
            strMatch += (lookahead[i]);
            i++;
        }
        found.push_back({dictionary.size()- pos, strMatch, lookahead[i]}); /* lookahead[i] is now the next char*/
        
    }

    std::reverse(found.begin(), found.end());
    int index = 0;
    size_t max = 0;
    Data d(0,"",0);
    for (auto vec : found)
    {
        if (vec.match.length() > max)
        {
            d = vec;
            max = vec.match.length();
        }
        index++;
    }
    triplas.emplace_back(d);
    matchSz = d.match.size()+1;
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
    int filesize = getFileSize(file);
    filebuffer = readFileToBuffer(file);
    Lookahead *look = new Lookahead(filesize);
    Dictionary *dict = new Dictionary();
    std::string bitString;
    while (!look->lookahead.empty())
    {
        dict->findBestMatch(look->lookahead);
        look->updateLook(dict->matchSz);
        dict->updateDict(dict->matchSz);
    }

    for(int i = 0; i< dict->triplas.size(); i++){
        if(4 + dict->triplas[i].match.size() * 8 < (1+DICTBITS+LOOKBITS+8)){
            bitString += "0"; /*flag that indicates no compression made*/
            bitString.append(std::bitset<3>(dict->triplas[i].match.size()).to_string());
            for (int j = 0; j < dict->triplas[i].match.size(); j++){
                bitString.append(std::bitset<8>(dict->triplas[i].match[j]).to_string());
            }
        }else{
            bitString += "1";
            bitString.append(std::bitset<15>(dict->triplas[i].offset).to_string());
            bitString.append(std::bitset<9>(dict->triplas[i].match.size()).to_string());
            bitString.append(std::bitset<8>(dict->triplas[i].nextChar).to_string());
        }
    }
    while (bitString.size() % 8 != 0)
    {
        bitString += "0";
    }
    std::cout<<bitString;
    delete look;
    delete dict;
}


int main(){
    std::ifstream file("teste.txt", std::ios::in | std::ios::binary | std::ios::ate);
    CompressFile(file);

    return 0;
}