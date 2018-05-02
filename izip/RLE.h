#pragma once

#include <string>
#include <vector>

class RLE{
public:
    static void Encode(std::string file1, std::string file2);
    static void Decode(std::string file1, std::string file2);
};
int CheckLength(std::vector<uint8_t> *data, uint8_t value, int index);