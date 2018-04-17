#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include "lz77.h"

#define LOOKAHEADSIZE 255
#define DICTSIZE 255
#define WINDOWSIZE LOOKAHEADSIZE + DICTSIZE

#define LOOKBITS floor(log2(LOOKAHEADSIZE) + 1)
#define DICTBITS floor(log2(DICTSIZE) + 1)


/*O LZ77 vai receber os dados já tratados, cada .cpp deve funcionar atomicamente*/

std::string getChars(int position, int len, std::string buffer){
    std::string chars;
    for(int i=0; i<len; i++){
        chars += buffer[position+i];
    }
    return chars;
}

class Data{
    public:
        Data();
        ~Data();
        size_t offset;
        std::string match;
        char nextChar;
};
class Dictionary{
    public:
        Dictionary();
        ~Dictionary();
        int updateWindow();
        int getNextWindow();
        size_t dpb = 0; /*Dictionary pointer to begin of Dictionary*/
        size_t dpe = 0; /*Dictionary pointer to end of Dictionary*/

        std::unordered_map<char, std::vector<int>> hash;
};
class Lookahead{
    public:
        Lookahead(int filesize);
        ~Lookahead();
        
        size_t lpb=0; /*lookahead pointer to begin of lookahead*/
        size_t lpe = 0; /*lookahead pointer to end of lookahead*/

        std::deque<Data> triplas;
};
Lookahead::Lookahead(int filesize){

    if (filesize > LOOKAHEADSIZE)
    {
        windowPointer += LOOKAHEADSIZE;
    }
    else
    {
        windowPointer += filesize;
    }
};

Dictionary::Dictionary(){

};
