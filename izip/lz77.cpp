#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <deque>
#include "lz77.h"

#define LOOKAHEADSIZE 255
#define DICTSIZE 255
#define WINDOWSIZE LOOKAHEADSIZE + DICTSIZE

#define LOOKBITS floor(log2(LOOKAHEADSIZE) + 1)
#define DICTBITS floor(log2(DICTSIZE) + 1)


/*O LZ77 vai receber os dados já tratados, cada .cpp deve funcionar atomicamente*/


class Data{
    public:
        Data();
        ~Data();
        size_t offset
        std::string match;
        char nextChar;
};
class Dictionary{
    public:
        Dictionary();
        ~Dictionary();
        
        int updateWindow();
        int getNextWindow();
        size_t dictPointer=0;
};
class Lookahead{
    public:
        Lookahead(int filesize);
        ~Lookahead();
        int getChars(int len)
        size_t lookaheadPointer=0;
};

class Window{
    Lookahead *look;
    Dictionary *dict;

    public:
        Window();
        ~Window();
        int moveWindow(int len);
        size_t windowPointer=0;
        deque<Data> triplas;   
};

Lookahead::Lookahead(int filesize){
    if(filesize > LOOKAHEADSIZE){
        lookaheadPointer += LOOKAHEADSIZE;
    }
    else{
        lookaheadPointer = filesize;
    }
};

Dictionary::Dictionary(){
    lookaheadPointer = 1;
};

Window::Window(void){
    look = new Lookahead(filesize);
    dict = new Dictionary();
    windowPointer = 1;
}