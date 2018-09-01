#pragma once
#include "BitIO.h"
#include <cstdint>
#include <vector>

typedef uint64_t uint64;


class Arithmetic {

	void WriteBits(bool bit) {
		//Write bits to file, if there are pending bits, these are added as well
		bw.WriteBit((bit) ? (uint8_t)1 : (uint8_t)0);
		while (pendingBits > 0) {
			bw.WriteBit((bit) ? (uint8_t)0 : (uint8_t)1);
			pendingBits--;
		}
	}

	uint64 precision;
	uint64 msbMask;
	uint64 alphabetSize;

public:
	BitWriter bw;
	BitReader br;

	Arithmetic(uint64 precision = 0xffffffffUL, uint64 msb = 0x80000000UL, uint64 alphabetSize = 256);

	int pendingBits;

	int ReadBit() {
		int temp = (int)br.GetBit();
		if (temp == -1)
			temp = 0;
		return temp;
	}

	void StaticEncode(fstream* output, std::vector<uint8_t>* input);
	void StaticDecode(fstream* input, std::vector<uint8_t>* output);

	void AdaptiveEncode(fstream* output, std::vector<uint8_t>* input);
	void AdaptiveDecode(fstream* input, std::vector<uint8_t>* output);
};