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

std::string filebuffer;
/*O LZ77 vai receber os dados já tratados, cada .cpp deve funcionar atomicamente*/

std::string getChars(int position, int len, std::string buffer){
    std::string chars;
    for(int i=0; i<len; i++){
        chars += buffer[position+i];
    }
    return chars;
}
typedef struct Data
{
    size_t offset;
    std::string match;
    char nextChar;

    Data(size_t offset_, std::string match_, char nextChar_) : offset(offset_), match(match_), nextChar(nextChar_){};
} Data;

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
        std::string lookahead;
        Lookahead(int filesize);
        ~Lookahead();
        size_t lpb=0; /*lookahead pointer to begin of lookahead*/
        size_t lpe = 0; /*lookahead pointer to end of lookahead*/

        std::deque<Data> triplas;
};


Lookahead::Lookahead(int filesize){
    if(filesize==1){
        exit(1);
    }

    if (filesize > LOOKAHEADSIZE)
    {
        lpe += LOOKAHEADSIZE;
        lookahead.append(getChars(1, LOOKAHEADSIZE, filebuffer));
        triplas.emplace_back(0,0,filebuffer[0]);
    }
    else
    {
        lpe += filesize;
        lookahead.append(getChars(1, filesize-1, filebuffer));
    }


};


Dictionary::Dictionary(){

};
