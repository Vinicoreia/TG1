#ifndef IZIP_LZ77_H_
#define IZIP_LZ77_H_
#include "util.h"

#define DICTSIZE 32767
#define LOOKAHEADSIZE 255
#define WINDOWSIZE LOOKAHEADSIZE + DICTSIZE
#define DICTBITS 15
#define LOOKBITS 8

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

class Lookahead{
    
    public:
        std::string lookahead;
        Lookahead(int filesize);
        size_t lpb=0; /*lookahead pointer to begin of lookahead*/
        size_t lpe = 0; /*lookahead pointer to end of lookahead*/
        void updateLook(size_t offset);
};

#endif
