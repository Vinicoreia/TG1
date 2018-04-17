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
struct Data
{
    size_t offset;
    size_t matchSize;
    char nextChar;
    Data(size_t offset, size_t matchSize, char nextChar) : offset(offset), matchSize(matchSize), nextChar(nextChar){}
};

class Dictionary{
    public:
        std::string dictionary;
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
        size_t lpb=0; /*lookahead pointer to begin of lookahead*/
        size_t lpe = 0; /*lookahead pointer to end of lookahead*/

        std::deque<Data> triplas;
};


Lookahead::Lookahead(int filesize){

    // if(filesize<=LOOKBITS+DICTBITS+8){
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
    triplas.emplace_back(0, 0, filebuffer[0]);
};


Dictionary::Dictionary(){
    dpe+=1;
    dictionary.append(getChars(0, 1, filebuffer));
};




int main(){
    filebuffer = "teste";

    Lookahead * look = new Lookahead(6);
    std::cout << look->triplas.at(0).nextChar;
    delete look;
    return 0;
}