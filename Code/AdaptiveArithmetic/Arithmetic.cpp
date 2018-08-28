#include "Arithmetic.h"
#include "CumulativeCountTable.h"

#include <iostream>
#include <cstdint>
#include <stack>

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
	long long currentCode = 0;

	int precisionBits = 0;

	for (size_t i = 0; i < (sizeof(unsigned long long) << 3); i++) {
		if (precision >> i)
			precisionBits++;
	}

	for (int i = 0; i < precisionBits; i++) {
		currentCode = (currentCode << 1) | ReadBit();
	}

	int symbol = 0;

	for (;;) {
		unsigned long long range = high - low + 1;
		unsigned long long value = ((currentCode - low + 1) * freq.count - 1) / range;

		//Get current symbol.
		symbol = freq.Search((long long)value);

		//Update the range
		unsigned long long nLow = low + (range * freq.GetLow(symbol)) / freq.count;
		high = low + range * freq.GetHigh(symbol) / freq.count - 1;
		low = nLow;

		//While MSB are equal write bit on file
		while (((low^high) & msbMask) == 0) {
			currentCode = ((currentCode << 1) & precision) | ReadBit();
			low = (low << 1) & precision;
			high = ((high << 1)& precision) | 1;
		}
		//E3, when [low, high] has 0.5
		while ((low & ~high & msbMask>>1) != 0) {
			currentCode = (currentCode & msbMask) | ((currentCode << 1) & (precision >> 1)) | ReadBit();
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
	long long currentCode = 0;

	int precisionBits = 0;

	for (size_t i = 0; i < (sizeof(unsigned long long) << 3); i++) {
		if (precision >> i)
			precisionBits++;
	}

	for (int i = 0; i < precisionBits; i++) {
		currentCode = (currentCode << 1) | ReadBit();
	}

	int symbol = 0;

	for (;;) {
		unsigned long long range = high - low + 1;
		unsigned long long value = ((currentCode - low + 1) * freq.count - 1) / range;

		//Get current symbol.
		symbol = freq.Search((long long)value);

		//Update the range
		unsigned long long nLow = low + (range * freq.GetLow(symbol)) / freq.count;
		high = low + range * freq.GetHigh(symbol) / freq.count - 1;
		low = nLow;

		//While MSB are equal write bit on file
		while (((low^high) & msbMask) == 0) {
			currentCode = ((currentCode << 1) & precision) | ReadBit();
			low = (low << 1) & precision;
			high = ((high << 1)& precision) | 1;
		}
		//E3, when [low, high] has 0.5
		while ((low & ~high & msbMask >> 1) != 0) {
			currentCode = (currentCode & msbMask) | ((currentCode << 1) & (precision >> 1)) | ReadBit();
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

void Arithmetic::PPMEncode(fstream * output, vector<uint8_t>* input)
{
	CumulativeCountTable orderM1(vector<long long>(alphabetSize + 2, 1));
	CumulativeCountTable order0(alphabetSize + 2);
	order0.AddFrequency((size_t)alphabetSize);
	order0.AddFrequency((size_t)alphabetSize + 1);

	int currentOrder = 1;
	int nofOrders = 2;

	if (!input) {
		std::cout << "Could not open input file!\n";
		exit(-1);
	}

	bw.SetFile(output);

	high = precision;
	low = 0x0;

	pendingBits = 0;

	uint8_t read;
	int readAux;

	for (size_t i = 0; i <= input->size(); i++) {
		if (i < input->size())
			readAux = input->at(i);
		//readAux = read;
		/*if (i == input->size())*/else {
			readAux = (int)alphabetSize + 1;
		}

		//Search for longest context
		for (;;) {
			CumulativeCountTable* currentTable = (currentOrder-1 == 0) ? &order0 : &orderM1;
			if (currentTable->GetFrequency(readAux) == 0) {
				//not in current context, 
				//write escape signal, update table, go to a lower context
				WriteSymbol((int)alphabetSize, *currentTable);
				currentOrder--;
				currentTable->AddFrequency(readAux);
			}
			else {
				//In contex, so write symbol, add to the frequency and update the context
				WriteSymbol(readAux, *currentTable);
				if (currentOrder != 0) {
					currentTable->AddFrequency(readAux);
				}
				currentOrder += (currentOrder + 1 == nofOrders)?0:1;
				break;
			}
		}

		//WriteSymbol(readAux, order0);
	}

	WriteBits(1);
	bw.End();

}

void Arithmetic::PPMDecode(fstream * input, vector<uint8_t>* output)
{
	CumulativeCountTable orderM1(vector<long long>(alphabetSize + 2, 1));
	CumulativeCountTable order0(alphabetSize + 2);
	order0.AddFrequency((size_t)alphabetSize);
	order0.AddFrequency((size_t)alphabetSize + 1);

	stack<CumulativeCountTable*> previousContexts;

	int currentOrder = 1;
	int nofOrders = 2;

	CumulativeCountTable* currentContext = &order0;

	output->clear();

	br.SetFile(input);

	unsigned long long high = precision;
	unsigned long long low = 0x0;
	long long currentCode = 0;

	int precisionBits = 0;

	for (size_t i = 0; i < (sizeof(unsigned long long) << 3); i++) {
		if (precision >> i)
			precisionBits++;
	}

	for (int i = 0; i < precisionBits; i++) {
		currentCode = (currentCode << 1) | ReadBit();
	}

	int symbol = 0;

	for (;;) {
		currentContext = (currentOrder - 1 == 0) ? &order0 : &orderM1;

		unsigned long long range = high - low + 1;
		unsigned long long value = ((currentCode - low + 1) * currentContext->count - 1) / range;

		//Get current symbol.
		symbol = currentContext->Search((long long)value);

		//Update the range
		unsigned long long nLow = low + (range * currentContext->GetLow(symbol)) / currentContext->count;
		high = low + range * currentContext->GetHigh(symbol) / currentContext->count - 1;
		low = nLow;

		//While MSB are equal write bit on file
		while (((low^high) & msbMask) == 0) {
			currentCode = ((currentCode << 1) & precision) | ReadBit();
			low = (low << 1) & precision;
			high = ((high << 1)& precision) | 1;
		}
		//E3, when [low, high] has 0.5
		while ((low & ~high & msbMask >> 1) != 0) {
			currentCode = (currentCode & msbMask) | ((currentCode << 1) & (precision >> 1)) | ReadBit();
			low = (low << 1) & (precision >> 1);
			high = ((high << 1) & (precision >> 1)) | msbMask | 1;
		}

		if (symbol == alphabetSize + 1)
			break;
		if (symbol == alphabetSize) {
			//If escape symbol then decrement the current order
			currentOrder--;
			previousContexts.push(currentContext);
			continue;
		}
		//else the current symbol is in the current order, then add to the order and increment the order
		uint8_t toWrite = (uint8_t)symbol;

		//Add thew appearance of this symbol in the higher orders
		while (!previousContexts.empty()) {
			previousContexts.top()->AddFrequency(toWrite);
			previousContexts.pop();
		}
		//Add appearence to current order if the order isn't the order -1
		if (currentOrder > 0) {
			currentContext->AddFrequency(toWrite);
		}
		currentOrder += (currentOrder + 1 == nofOrders) ? 0 : 1;
		output->emplace_back(toWrite);
	}
}

void Arithmetic::WriteSymbol(int readAux, CumulativeCountTable freq)
{
	//Update the range of high and low
	unsigned long long range = high - low + 1;
	unsigned long long nLow = low + (range * freq.GetLow(readAux)) / freq.count;
	high = (low + (range * freq.GetHigh(readAux)) / freq.count) - 1;
	low = nLow;

	//While MSB are equal write bit on file
	while (((low^high) & msbMask) == 0) {
		WriteBits((low >> 31) & 1);
		low = (low << 1) & precision;
		high = ((high << 1) & precision) | 1;
	}
	//E3, when [low, high] has 0.5
	while (((low & ~high) & (msbMask >> 1)) != 0) {
		pendingBits++;
		low = (low << 1) & (precision >> 1);
		high = ((high << 1) & (precision >> 1)) | msbMask | 1;
	}
}