#include "MTF.h"
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <cstdint>

void MTF::Encode(std::string filename, std::string outfilename){
    std::fstream infile, outfile;

    infile.open(filename, std::fstream::in | std::fstream::binary);

    if(!infile.is_open()){
        std::cout<<"Error while opening file "<<filename<<std::endl;
        exit(-1);
    }

    outfile.open(outfilename, std::fstream::out | std::fstream::binary);

    if(!outfile.is_open()){
        std::cout<<"Error while opening file "<<outfilename<<std::endl;
        infile.close();
        exit(-1);
    }


    std::unordered_map<uint8_t, uint8_t> positions;
    std::vector<uint8_t> vec;

    for(int i = 0; i < 256; i++){
        positions[i] = i;
        vec.emplace_back(i);
    }

    while(infile.peek() != EOF){
        uint8_t read;

        infile.read((char*)&read, 1);
        uint8_t index = positions[read];

        outfile.write((char*)&index, 1);
        
        for(int i = 0; i < index; i++){
            positions[vec[i]]++;
        }

        positions[read] = 0;
        vec.erase(vec.begin() + index);
        vec.insert(vec.begin(), read);
    }

    infile.close();
    outfile.close();
}

void MTF::Decode(std::string filename, std::string outfilename){
    std::fstream infile, outfile;

    infile.open(filename, std::fstream::in | std::fstream::binary);

    if(!infile.is_open()){
        std::cout<<"Error while opening file "<<filename<<std::endl;
        exit(-1);
    }

    outfile.open(outfilename, std::fstream::out | std::fstream::binary);

    if(!outfile.is_open()){
        std::cout<<"Error while opening file "<<outfilename<<std::endl;
        infile.close();
        exit(-1);
    }

    std::vector<uint8_t> vec;

    for(int i = 0; i < 256; i++){
        vec.emplace_back(i);
    }

    while(infile.peek() != EOF){
        uint8_t read;

        infile.read((char*)&read, 1);
        uint8_t index = vec[read];

        outfile.write((char*)&index, 1);
        
        vec.erase(vec.begin() + read);
        vec.insert(vec.begin(), index);
    }

    infile.close();
    outfile.close();
}

int main (){
    std::string file1, file2;
    std::cin>>file1>>file2;

    MTF::Encode(file1, file2);

    std::cin>>file1>>file2;

    MTF::Decode(file1, file2);

    return 0;
}