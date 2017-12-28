#pragma once

#include <string>
#include "BWT.h"

class MTF{
    public:
    static void Encode(std::string filename, std::string outfilename);
    static void Decode(std::string filename, std::string outfilename);

	static void Encode(Data* in);
	static void Decode(Data* in);
};

