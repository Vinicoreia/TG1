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
#define DICTSIZE 4
#define LOOKAHEADSIZE 3
#define WINDOWSIZE LOOKAHEADSIZE + DICTSIZE

#define DICTBITS 3
#define LOOKBITS 2
using namespace std::chrono;

std::string filebuffer;
int filesize;
/*O LZ77 vai receber os dados já tratados, cada .cpp deve funcionar atomicamente*/

#define REP(i, n) for (int i = 0; i < (int)(n); ++i)

const int MAXN = 2*DICTSIZE;
std::string S;
int N, gap;
int sa[MAXN], pos[MAXN], tmp[MAXN], lcp[MAXN];

bool sufCmp(int i, int j)
{
    if (pos[i] != pos[j])
        return pos[i] < pos[j];
    i += gap;
    j += gap;
    return (i < N && j < N) ? pos[i] < pos[j] : i > j;
}

void buildSA()
{
    N = S.size();
    REP(i, N)
    sa[i] = i,
    pos[i] = S[i];
    for (gap = 1;; gap *= 2)
    {
        std::sort(sa, sa + N, sufCmp);
        REP(i, N - 1)
        tmp[i + 1] = tmp[i] + sufCmp(sa[i], sa[i + 1]);
        REP(i, N)
        pos[sa[i]] = tmp[i];
        if (tmp[N - 1] == N - 1)
            break;
    }
}

void buildLCP()
{
    for (int i = 0, k = 0; i < N; ++i)
        if (pos[i] != N - 1)
        {
            for (int j = sa[pos[i] + 1]; S[i + k] == S[j + k];)
                ++k;
            lcp[pos[i]] = k;
            if (k)
                --k;
        }
}

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
        size_t hpe=0, hpb=0;/*points to end of hash*/
        void hashDict(); /*creates an unordered_map of the values in the dict to reduce search for the biggest match*/
        void findBestMatch(std::string lookahead);/*This function has to return the Data to the lookahead*/
        int P[DICTSIZE+1][LOOKAHEADSIZE], stp;
        std::deque<Data> triplas;
        std::unordered_map<int, int> hash;
};

void Dictionary::updateDict(size_t offset){
    dpe += offset;
    if(dpe-dpb > DICTSIZE){
        dpb += offset;
    }
    dictionary = filebuffer.substr(dpb, dpe - dpb);
    if(dpe >= hpe and dpe<filesize){
        hpb = hpe;
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
    hpe += MAXN;
    if(hpe > filesize){
        hpe = filesize;
    }
    S = filebuffer.substr(hpb, hpe-hpb);
    buildSA();
    // buildLCP();
}

void Dictionary::findBestMatch(std::string lookahead)
{
    matchSz = 1;
    char a = lookahead[0];
    std::string strMatch0, strMatch1;
    int i =0;
    int j=0;
    int pos = 0;
    int flag0 = 0;
    /*Pegar indice do primeiro valor que tem lookahead[0]*/
    /*se i == SUffixArray.size() então não achou no array de suffixos*/
    
    while(i<MAXN){
        if(filebuffer[sa[i]]== a){
            break;
        }
        i++;
    }

    if(i>=MAXN){

        triplas.emplace_back(0, "", lookahead[0]);
        return;
        /*retorna tripla vazia*/
    }

    
    /* Achei um indice*/
    std::cout<<std::endl;
    while(filebuffer[sa[i]]==a){
        std::cout<<"I"<<sa[i]<<" "<<lookahead[0]<<" ";
        if(hpb+sa[i] >= dpb and hpb+sa[i] < dpe){
            strMatch0.clear();
            
            j = 0;
            while(dictionary[(sa[i]+j)%dictionary.size()] == lookahead[j] and j<lookahead.size()-1){
                strMatch0 += lookahead[j];
                j++;
            }
            std::cout<<strMatch0<<" ";
            if (strMatch1 > strMatch0)
            {
                break;
            }
            pos = i;
            strMatch1 = strMatch0;
        }
        i++;
    }

    if (strMatch1.size() ==0)
    {
        triplas.emplace_back(0, "", lookahead[0]);
        return;
        /*retorna tripla vazia*/
    }
    matchSz = strMatch1.size() + 1;
    triplas.emplace_back(dpe-(hpb+sa[pos]), strMatch1, lookahead[matchSz - 1]);

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
        // std::cout<<duration<<std::endl;
    }
    for (int i = 0; i < dict->triplas.size(); i++)
    {
        std::cout <<"TRIPLA: "<< dict->triplas[i].offset << " " << dict->triplas[i].match.size() << " "<< dict->triplas[i].nextChar<<std::endl;
    }
        bitString.clear();
        std::cout<<dict->triplas.size()<<std::endl;
        for (int i = 0; i < dict->triplas.size(); i++)
        {
            if(dict->triplas[i].offset == 0){
                /*nao teve match adiciona flag 0 e o nextchar*/
                bitString+= "0";
                bitString.append(std::bitset<8>(dict->triplas[i].nextChar).to_string());
            }else if((dict->triplas[i].match.size()*9 + 8) < (1+DICTBITS+LOOKBITS)){
                /*representar a match com DICTSIZE+DICTBITS nao vale a pena*/
                for (int j = 0; j < dict->triplas[i].match.size(); i++)
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