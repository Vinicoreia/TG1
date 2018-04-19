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
#define LOOKAHEADSIZE 255
#define DICTSIZE 16383
#define WINDOWSIZE LOOKAHEADSIZE + DICTSIZE

#define LOOKBITS floor(log2(LOOKAHEADSIZE) + 1)
#define DICTBITS floor(log2(DICTSIZE) + 1)
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
        std::unordered_map<char, std::vector<int>> hash;
        std::deque<Data> triplas;
};

void Dictionary::updateDict(size_t offset){
    dpe += offset;
    if(dpe-dpb > DICTSIZE){
        dpb += offset;
    }
    dictionary = filebuffer.substr(dpb, dpe - dpb);
    if(dpe > hpe){
        hashDict();
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
    triplas.emplace_back(0, "", filebuffer[0]);
    hashDict();
};

void Dictionary::hashDict()
{
    

    hash.clear();
    hpe += DICTSIZE;
    if(hpe > filesize){
        hpe=filesize;
    }
    int i = dpb;
    while(i<hpe){
        hash[filebuffer[i]].push_back(i);
        i++;
    }
    
}

void Dictionary::findBestMatch(std::string lookahead)
{
    std::vector<Data> found;
    char a = lookahead[0];
    std::string strMatch;
    std::vector<int> positions;
    int i, inRange;
    inRange = 0; /*flag to get if the position is in the range of the dictionary*/
    matchSz = 1;
    if(lookahead.size()==1){
        triplas.emplace_back(0, "", a);
        return;
    }
    try{
        positions = hash.at(a);/* initial position to search*/
    }catch(const std::out_of_range& e){
        /*Caso não tenha a letra no hash retorna 0,"",letra*/
        triplas.emplace_back(0, "", a);
        return;
    }

    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    if(positions.size()>=dictionary.size()/2){
        /*O problema de tempo se deve ao fato de ter muitos elementos repetidos na hash*/
        return;
    }
    for(auto &pos : positions){
        if(pos>=dpb and pos < dpe)
        {

            inRange = 1;
            strMatch.clear();
            i = 0;
            while (dictionary[(pos + i) % dictionary.size()] == lookahead[i] and i < lookahead.size() - 1) /*we can only go as far as the penultimate position*/
            {
                strMatch += (lookahead[i]);
                i++;
            }
            found.push_back({dpe-pos, strMatch, lookahead[i]}); /* lookahead[i] is now the next char*/
            
        }
        if(strMatch.size()==lookahead.size()-1){
            break;
        }
    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(t2 - t1).count();
    std::cout << "duration: " << duration << std::endl;

    if(inRange == 0){
        /*Caso não tenha a letra no hash retorna 0,"",letra*/
        triplas.emplace_back(0, "", a);
        return;
    }

    std::reverse(found.begin(), found.end()); /*A demora ta aqui*/

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

    // for (int i = 0; i < dict->triplas.size(); i++)
    // {
    //     std::cout << dict->triplas[i].offset << " " << dict->triplas[i].match.size() << " "<< dict->triplas[i].nextChar<<std::endl;
    // }

        for (int i = 0; i < dict->triplas.size(); i++)
        {
            if (4 + dict->triplas[i].match.size() * 8 < (1 + DICTBITS + LOOKBITS + 8))
            {
                bitString += "0"; /*flag that indicates no compression made*/
                bitString.append(std::bitset<3>(dict->triplas[i].match.size()).to_string());
                for (int j = 0; j < dict->triplas[i].match.size(); j++)
                {
                    bitString.append(std::bitset<8>(dict->triplas[i].match[j]).to_string());
                }
            }
            else
            {
                bitString += "1";
                bitString.append(std::bitset<14>(dict->triplas[i].offset).to_string());
                bitString.append(std::bitset<8>(dict->triplas[i].match.size()).to_string());
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
    std::ifstream file("bee.bmp", std::ios::in | std::ios::binary | std::ios::ate);
    CompressFile(file);

    return 0;
}