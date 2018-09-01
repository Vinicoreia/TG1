#include <iostream>
#include <cstdio>
#include <string>
#include <cstdint>
#include <ctime>
#include <utility>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cstdio>
#include "BitIO.h"
#include "ArithmeticEncoding.h"

#define NOFALPHABET 256

using namespace std;

#define MASK 0xffffffffU
#define TOP_MASK 0x80000000U

class FrequencyTable {
public:
	vector<long long> frequencies;
	vector<long long> limits;
	long long totalBytes;

	FrequencyTable(vector<long long> frequencies) :frequencies(frequencies) {
		long long total = 0;
		for (auto &i : frequencies) {
			limits.emplace_back(total);
			total += i;
		}
		//cout << total<<endl;
		totalBytes = total;
	}

	long long GetMaxSymbol() {
		return frequencies.size();
	}

	long long GetFrequency(long long index) {
		if (index < 0 || index >= frequencies.size()) {
			cout << "Invalid Index\n";
			return -1;
		}
		return frequencies[index];
	}

	long long GetLow(long long index) {
		if (index < 0 || index >= frequencies.size()) {
			cout << "Invalid Index\n";
			return -1;
		}
		return limits[index];
	}

	long long GetHigh(long long index) {
		if (index < 0 || index >= frequencies.size()) {
			cout << "Invalid Index\n";
			return -1;
		}
		return limits[index] + frequencies[index];
	}


	int Search(long long value) {
		long long start = 0;
		long long end = frequencies.size();
		while ((end - start > 1)) {
			long long mid = (start + end)>>1;
			if (GetLow(mid) > value)
				end = mid;
			else
				start = mid;
		}
		return (int)start;
	}
};

BitWriter ArithmeticEncoder::bw;
BitReader ArithmeticEncoder::br;

int ArithmeticEncoder::pendingBits = 0;

void ArithmeticEncoder::WriteBits(bool bit) {
	//Write bits to file, if there are pending bits, these are added as well
	bw.WriteBit((bit) ? (uint8_t)1 : (uint8_t)0);
	while (pendingBits > 0) {
		bw.WriteBit((bit) ? (uint8_t)0 : (uint8_t)1);
		pendingBits--;
	}
}
#ifdef C_IO
void ArithmeticEncoder::Encode(std::string filename, std::string outputfile) {
	FILE *input, *output;

	std::vector<long long> frequencies;
	unsigned long long nread = 0;

	//Initialize the frequencies
	for (int i = 0; i < NOFALPHABET; i++) {
		frequencies.emplace_back(0);
	}

	//Add the EOF to the end
	frequencies.emplace_back(1);

	input = fopen(filename.c_str(), "rb");

	if (!input) {
		std::cout << "Could not open input file!\n";
		exit(-1);
	}

	br.SetFile(input);

	//Get frequencies of each byte.
	do {
		uint8_t read;
		nread++;
		fread(&read, sizeof(uint8_t), 1, input);
		if(!feof(input))
			frequencies[read]++;
	} while (!feof(input));

	fclose(input);
	input = fopen(filename.c_str(), "rb");

	output = fopen(outputfile.c_str(), "wb");
	if (!output) {
		std::cout << "Could not open output file!\n";
		fclose(input);
		exit(-1);
	}

	bw.SetFile(output);

	FrequencyTable freq(frequencies);

	unsigned long long high = 0xffffffffU;
	unsigned long long low = 0x0;

	//write frequencies
	for (int i = 0; i < frequencies.size() - 1; i++) {
		fwrite(&frequencies[i], sizeof(long long), 1, output);
	}

	pendingBits = 0;


	uint8_t read;
	int readAux;

	while (!feof(input)) {
		fread(&read, sizeof(uint8_t), 1, input);
		readAux = read;
		if (feof(input))
			readAux = NOFALPHABET;

		//Update the range of high and low
		unsigned long long range = high - low + 1;
		unsigned long long nLow = low + (range * freq.GetLow(readAux)) / freq.totalBytes;
		high = (low + (range * freq.GetHigh(readAux)) / freq.totalBytes) - 1;
		low = nLow;

		//While MSB are equal write bit on file
		while (((low^high) & 0x80000000U) == 0) {
			WriteBits((low >> 31) & 1);
			low = (low << 1)&MASK;
			high = ((high << 1)&MASK) | 1;
		}
		//E3, when [low, high] has 0.5
		while (((low & ~high) & 0x40000000U) != 0) {
			pendingBits++;
			low = (low << 1) & (MASK >> 1);
			high = ((high << 1) & (MASK >> 1)) | TOP_MASK | 1;
		}
	} 

	WriteBits(1);
	bw.End();

	fclose(input);
	fclose(output);
}


void ArithmeticEncoder::Decode(std::string filename, std::string outputfile) {
	FILE* input, *output;
	std::vector<long long> frequencies;

	input = fopen(filename.c_str(), "rb");
	if (!input) {
		std::cout << "Could not open input file!\n";
		exit(-1);
	}

	output = fopen(outputfile.c_str(), "wb");
	if (!output) {
		std::cout << "Could not open output file!\n";
		fclose(input);
		exit(-1);
	}

	//Get Frequencies
	for (int i = 0; i < NOFALPHABET; i++) {
		long long read;
		fread(&read, sizeof(long long), 1, input);
		frequencies.emplace_back(read);
	}

	//Add EOF
	frequencies.emplace_back(1);

	br.SetFile(input);
	bw.SetFile(output);

	FrequencyTable freq(frequencies);

	unsigned long long high = 0xffffffffU;
	unsigned long long low = 0x0;
	long long code = 0;

	for (int i = 0; i < 32; i++) {
		code = (code << 1) | br.GetBit();
	}

	int symbol = 0;

	for(;;) {
		unsigned long long range = high - low + 1;
		unsigned long long value = ((code - low + 1) * freq.totalBytes - 1) / range;

		//Get current symbol.
		symbol = freq.Search(value);

		//Update the range
		unsigned long long nLow = low + (range * freq.GetLow(symbol)) / freq.totalBytes;
		high = low + range * freq.GetHigh(symbol) / freq.totalBytes - 1;
		low = nLow;

		//While MSB are equal write bit on file
		while (((low^high) & 0x80000000U) == 0) {
			code = ((code << 1) & MASK) | ReadBit();
			low = (low << 1) & MASK;
			high = ((high << 1)& MASK) | 1;
		}
		//E3, when [low, high] has 0.5
		while ((low & ~high & 0x40000000U) != 0) {
			code = (code & TOP_MASK) | ((code << 1) & (MASK >>1)) | ReadBit();
			low = (low << 1) & (MASK >> 1);
			high = ((high << 1) & (MASK >> 1)) | TOP_MASK | 1;
		}

		if (symbol == 256)
			break;

		uint8_t toWrite = (uint8_t)symbol;
		fwrite(&toWrite, sizeof(uint8_t), 1, output);
	}
}

#else

void ArithmeticEncoder::Encode(fstream* output, vector<uint8_t>* input) {

	std::vector<long long> frequencies;

	//Initialize the frequencies
	for (int i = 0; i < NOFALPHABET; i++) {
		frequencies.emplace_back(0);
	}

	//Add the EOF to the end
	frequencies.emplace_back(1);

	if (!input) {
		std::cout << "Could not open input file!\n";
		exit(-1);
	}

	//Get frequencies of each byte.
	for (int i = 0; i < input->size(); i++) {
		frequencies[input->at(i)]++;
	}

	bw.SetFile(output);

	FrequencyTable freq(frequencies);

	unsigned long long high = 0xffffffffU;
	unsigned long long low = 0x0;

	//write frequencies
	output->write((char*)frequencies.data(), NOFALPHABET * sizeof(long long));

	pendingBits = 0;

	uint8_t read;
	int readAux;


	for (int i = 0; i <= input->size(); i++) {
		if (i < input->size())
			readAux = input->at(i);
			//readAux = read;
		/*if (i == input->size())*/else {
			readAux = NOFALPHABET;
		}

		//Update the range of high and low
		unsigned long long range = high - low + 1;
		unsigned long long nLow = low + (range * freq.GetLow(readAux)) / freq.totalBytes;
		high = (low + (range * freq.GetHigh(readAux)) / freq.totalBytes) - 1;
		low = nLow;

		//While MSB are equal write bit on file
		while (((low^high) & 0x80000000U) == 0) {
			WriteBits((low >> 31) & 1);
			low = (low << 1)&MASK;
			high = ((high << 1)&MASK) | 1;
		}
		//E3, when [low, high] has 0.5
		while (((low & ~high) & 0x40000000U) != 0) {
			pendingBits++;
			low = (low << 1) & (MASK >> 1);
			high = ((high << 1) & (MASK >> 1)) | TOP_MASK | 1;
		}
	}

	WriteBits(1);
	bw.End();
}


void ArithmeticEncoder::Decode(fstream* input, vector<uint8_t>* output) {
	std::vector<long long> frequencies(NOFALPHABET);

	output->clear();

	//Get Frequencies
	input->read((char*)frequencies.data(), NOFALPHABET * sizeof(long long));

	//Add EOF
	frequencies.emplace_back(1);

	br.SetFile(input);
	
	FrequencyTable freq(frequencies);

	unsigned long long high = 0xffffffffU;
	unsigned long long low = 0x0;
	long long code = 0;

	for (int i = 0; i < 32; i++) {
		code = (code << 1) | ReadBit();
	}

	int symbol = 0;

	for (;;) {
		unsigned long long range = high - low + 1;
		unsigned long long value = ((code - low + 1) * freq.totalBytes - 1) / range;

		//Get current symbol.
		symbol = freq.Search(value);

		//Update the range
		unsigned long long nLow = low + (range * freq.GetLow(symbol)) / freq.totalBytes;
		high = low + range * freq.GetHigh(symbol) / freq.totalBytes - 1;
		low = nLow;

		//While MSB are equal write bit on file
		while (((low^high) & 0x80000000U) == 0) {
			code = ((code << 1) & MASK) | ReadBit();
			low = (low << 1) & MASK;
			high = ((high << 1)& MASK) | 1;
		}
		//E3, when [low, high] has 0.5
		while ((low & ~high & 0x40000000U) != 0) {
			code = (code & TOP_MASK) | ((code << 1) & (MASK >> 1)) | ReadBit();
			low = (low << 1) & (MASK >> 1);
			high = ((high << 1) & (MASK >> 1)) | TOP_MASK | 1;
		}

		if (symbol == 256)
			break;

		uint8_t toWrite = (uint8_t)symbol;
		output->emplace_back(toWrite);
	}
}


#endif
#undef NOFALPHABET 
#undef MASK
#undef TOP_MASK