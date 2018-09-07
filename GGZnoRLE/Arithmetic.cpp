#include "Arithmetic.h"
#include "CumulativeCountTable.h"

#include <iostream>

#define MASK 0xffffffffU
#define TOP_MASK 0x80000000U

Arithmetic::Arithmetic(uint64 precision, uint64 msb, uint64 alphabetSize):precision(precision), msbMask(msb), alphabetSize(alphabetSize)
{
}

void Arithmetic::StaticEncode(fstream* output, std::vector<uint8_t>* input) {

	std::vector<long long> frequencies((size_t)alphabetSize);

	//Add the EOF to the end
	frequencies.emplace_back(1);

	if (!input) {
		std::cout << "Could not open input file!\n";
		exit(-1);
	}

	//Get frequencies of each byte.
	for (size_t i = 0; i < input->size(); i++) {
		frequencies[input->at(i)]++;
	}

	bw.SetFile(output);

	CumulativeCountTable freq(frequencies);

	unsigned long long high = precision;
	unsigned long long low = 0x0;

	//write frequencies
	output->write((char*)frequencies.data(), alphabetSize * sizeof(long long));

	pendingBits = 0;

	uint8_t read;
	int readAux;


	for (size_t i = 0; i <= input->size(); i++) {
		if (i < input->size())
			readAux = input->at(i);
		//readAux = read;
		/*if (i == input->size())*/else {
			readAux = (int)alphabetSize;
		}

		//Update the range of high and low
		unsigned long long range = high - low + 1;
		unsigned long long nLow = low + (range * freq.GetLow(readAux)) / freq.count;
		high = (low + (range * freq.GetHigh(readAux)) / freq.count) - 1;
		low = nLow;

		//While MSB are equal write bit on file
		while (((low^high) & msbMask) == 0) {
			WriteBits((low >> 31) & 1);
			low = (low << 1)&precision;
			high = ((high << 1)&precision) | 1;
		}
		//E3, when [low, high] has 0.5
		while (((low & ~high) & (msbMask>>1)) != 0) {
			pendingBits++;
			low = (low << 1) & (precision >> 1);
			high = ((high << 1) & (precision >> 1)) | msbMask | 1;
		}
	}

	WriteBits(1);
	bw.End();
}


void Arithmetic::StaticDecode(fstream* input, vector<uint8_t>* output) {
	std::vector<long long> frequencies((size_t)alphabetSize);

	output->clear();

	//Get Frequencies
	input->read((char*)frequencies.data(), alphabetSize * sizeof(uint64));

	//Add EOF
	frequencies.emplace_back(1);

	br.SetFile(input);

    CumulativeCountTable freq(frequencies);

	unsigned long long high = precision;
	unsigned long long low = 0x0;
	long long code = 0;

	int precisionBits = 0;

	for (size_t i = 0; i < (sizeof(unsigned long long) << 3); i++) {
		if (precision >> i)
			precisionBits++;
	}

	for (int i = 0; i < precisionBits; i++) {
		code = (code << 1) | ReadBit();
	}

	int symbol = 0;

	for (;;) {
		unsigned long long range = high - low + 1;
		unsigned long long value = ((code - low + 1) * freq.count - 1) / range;

		//Get current symbol.
		symbol = freq.Search((long long)value);

		//Update the range
		unsigned long long nLow = low + (range * freq.GetLow(symbol)) / freq.count;
		high = low + range * freq.GetHigh(symbol) / freq.count - 1;
		low = nLow;

		//While MSB are equal write bit on file
		while (((low^high) & msbMask) == 0) {
			code = ((code << 1) & precision) | ReadBit();
			low = (low << 1) & precision;
			high = ((high << 1)& precision) | 1;
		}
		//E3, when [low, high] has 0.5
		while ((low & ~high & msbMask>>1) != 0) {
			code = (code & msbMask) | ((code << 1) & (precision >> 1)) | ReadBit();
			low = (low << 1) & (precision >> 1);
			high = ((high << 1) & (precision >> 1)) | msbMask | 1;
		}

		if (symbol == 256)
			break;

		uint8_t toWrite = (uint8_t)symbol;
		output->emplace_back(toWrite);
	}
}

void Arithmetic::AdaptiveEncode(fstream * output, vector<uint8_t>* input)
{
	std::vector<long long> frequencies((size_t)alphabetSize, 1);

	//Add the EOF to the end
	frequencies.emplace_back(1);

	if (!input) {
		std::cout << "Could not open input file!\n";
		exit(-1);
	}

	bw.SetFile(output);

	CumulativeCountTable freq(frequencies);

	unsigned long long high = precision;
	unsigned long long low = 0x0;

	pendingBits = 0;

	uint8_t read;
	int readAux;

	for (size_t i = 0; i <= input->size(); i++) {
		if (i < input->size())
			readAux = input->at(i);
		//readAux = read;
		/*if (i == input->size())*/else {
			readAux = (int)alphabetSize;
		}

		//Update the range of high and low
		unsigned long long range = high - low + 1;
		unsigned long long nLow = low + (range * freq.GetLow(readAux)) / freq.count;
		high = (low + (range * freq.GetHigh(readAux)) / freq.count) - 1;
		low = nLow;

		//While MSB are equal write bit on file
		while (((low^high) & msbMask) == 0) {
			WriteBits((low >> 31) & 1);
			low = (low << 1)&precision;
			high = ((high << 1)&precision) | 1;
		}
		//E3, when [low, high] has 0.5
		while (((low & ~high) & (msbMask >> 1)) != 0) {
			pendingBits++;
			low = (low << 1) & (precision >> 1);
			high = ((high << 1) & (precision >> 1)) | msbMask | 1;
		}

		freq.AddFrequency(readAux);
	}

	WriteBits(1);
	bw.End();
}

void Arithmetic::AdaptiveDecode(fstream * input, vector<uint8_t>* output)
{
	std::vector<long long> frequencies((size_t)alphabetSize, 1);

	output->clear();

	//Add EOF
	frequencies.emplace_back(1);

	br.SetFile(input);

	CumulativeCountTable freq(frequencies);

	unsigned long long high = precision;
	unsigned long long low = 0x0;
	long long code = 0;

	int precisionBits = 0;

	for (size_t i = 0; i < (sizeof(unsigned long long) << 3); i++) {
		if (precision >> i)
			precisionBits++;
	}

	for (int i = 0; i < precisionBits; i++) {
		code = (code << 1) | ReadBit();
	}

	int symbol = 0;

	for (;;) {
		unsigned long long range = high - low + 1;
		unsigned long long value = ((code - low + 1) * freq.count - 1) / range;

		//Get current symbol.
		symbol = freq.Search((long long)value);

		//Update the range
		unsigned long long nLow = low + (range * freq.GetLow(symbol)) / freq.count;
		high = low + range * freq.GetHigh(symbol) / freq.count - 1;
		low = nLow;

		//While MSB are equal write bit on file
		while (((low^high) & msbMask) == 0) {
			code = ((code << 1) & precision) | ReadBit();
			low = (low << 1) & precision;
			high = ((high << 1)& precision) | 1;
		}
		//E3, when [low, high] has 0.5
		while ((low & ~high & msbMask >> 1) != 0) {
			code = (code & msbMask) | ((code << 1) & (precision >> 1)) | ReadBit();
			low = (low << 1) & (precision >> 1);
			high = ((high << 1) & (precision >> 1)) | msbMask | 1;
		}

		if (symbol == 256)
			break;

		uint8_t toWrite = (uint8_t)symbol;
		freq.AddFrequency(toWrite);
		output->emplace_back(toWrite);
	}
}
