#pragma once

#include <string>

class RLE{
public:
    static void Encode(std::string file1, std::string file2);
    static void Decode(std::string file1, std::string file2);
};