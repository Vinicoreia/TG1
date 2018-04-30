#include "deflate.h"
#include "util.h"
#include "lz77.h"
#include "huff.h"
#include <iostream>

#define USIZE uint16_t
#define SHIFT sizeof(USIZE)/2
#define MASK 0xFF
/*1- Como aplicar Burrows wheeler (antes e depois, só antes ou só depois)*/
/*2- Como splitar, quantos blocos, a partir de qual tamanho*/
/*3- lembrar do rle definido*/



/*Chamar LZ77 ENCODE com flag encode = 1*/

void DeflateEncode(std::string filenameIn, std::string filenameOut, int encode){
    /*LZ77 PART*/
    /*This will put the triples in the strBuffer*/
    EncodeLZ77(filenameIn, filenameOut, 1);

    std::vector<USIZE> bufferOffLen;
    std::string bufferChar;
    std::vector<std::pair<USIZE, long long>> pairOffLenProb; /*-*/
    std::vector<std::pair<char, long long>> paircharProb;    /*-*/

    USIZE aux = 0;
    for(int i =0; i<strBuffer.size(); i+=3){
        aux = strBuffer[i];
        aux = aux << SHIFT;
        aux |= strBuffer[i+1];
        bufferOffLen.push_back(aux);
        bufferChar.push_back(strBuffer[i+2]);
    }

    /*Huffman Part*/
    paircharProb = getFrequencyU8(bufferChar);
    pairOffLenProb = getFrequencyU16(bufferOffLen);
}