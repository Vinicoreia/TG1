#pragma once

#include <string>

class MTF{
    public:
    static void Encode(std::string filename, std::string outfilename);
    static void Decode(std::string filename, std::string outfilename);
};

