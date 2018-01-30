#pragma once

#include "BitIO.h"
#include <vector>
#include <fstream>

using namespace std;

class ArithmeticEncoder {
	static void WriteBits(bool bit);
public:
	static BitWriter bw;
	static BitReader br;
	static int pendingBits;
#ifdef C_IO
	static void Encode(std::string filename, std::string outputfile);
	static void Decode(std::string filename, std::string outputfile);
#else
	static void Encode(fstream* output, vector<uint8_t>* input);
	static void Decode(fstream* input, vector<uint8_t>* output);
#endif
	static int ReadBit() {
		int temp = (int)br.GetBit();
		if (temp == -1)
			temp = 0;
		return temp;
	}
};