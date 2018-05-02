#include "util.h"
#include "deflate.h"
#include "lz77.h"
#include "huff.h"
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <sstream>
    /*
    This is a simple compressor implementation using some ideas of the folowing compressing methods:
    - Deflate
    - Lz77
    - Huffman
    - BWT
    - MTF
    - RLE
    
    */
int main(int argc, char *argv[]){

    /* vou passar algo como ./main filein fileout -he -de -lz77e */
    std::vector<char *> args;
    std::string filenameIn, filenameOut;
    for(int i =0; i< argc; i++){
        args.push_back(argv[i]);
    }
    if(args.size()>3){
        filenameIn = args[1];
        filenameOut = args[2];
    }

    args.erase(args.begin(),args.begin()+3);
    
    for(auto it: args){
        if(std::string(it) == "-he"){
            filenameOut.append(".huf");
            HuffmanEncode(filenameIn, filenameOut);
        }
        else if(std::string(it) == "-de"){
            filenameOut.append(".def");
            DeflateEncode(filenameIn, filenameOut);
        }
        else if( std::string(it) == "-lz77e"){
            filenameOut.append(".lz77");
            EncodeLZ77(filenameIn, filenameOut);            
        }
    }
    

    return 0;
}
