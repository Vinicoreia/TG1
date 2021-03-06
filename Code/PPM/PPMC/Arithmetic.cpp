#include "Arithmetic.h"
#include "CumulativeCountTable.h"

#include <iostream>
#include <cstdint>
#include <stack>
#include <unordered_map>

#define MASK 		0xffffffffU
#define TOP_MASK 	0x80000000U

#define MAXMEM 750000
//3500 = 15MB
//250000 = 550MB 
Arithmetic::Arithmetic(uint64 precision, uint64 msb, uint64 alphabetSize):precision(precision), msbMask(msb), alphabetSize(alphabetSize)
{
}

void Arithmetic::StaticEncode(fstream* output, std::vector<uint8_t>* input) {

	std::vector<unsigned int> frequencies((size_t)alphabetSize);

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
	
	int shiftBits = 0;
	
	for(int i = 0;;i++){
		if((msbMask >> i) &1){
			break;
		}else 
			shiftBits++;
	}
	
	cout<<shiftBits<<endl;

	unsigned long long high = precision;
	unsigned long long low = 0x0;

	//write frequencies
	output->write((char*)frequencies.data(), alphabetSize * sizeof(long long));

	pendingBits = 0;

	uint8_t read;
	int readAux;


	for (size_t i = 0; i <= input->size(); i++) {
		//cout << "\rProcessing " << i << " of " << input->size();
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
			WriteBits((low >> shiftBits) & 1);
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
	std::vector<unsigned int> frequencies((size_t)alphabetSize);

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
	std::vector<unsigned int> frequencies((size_t)alphabetSize, 1);

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
		//cout << "\rProcessing " << i << " of " << input->size();
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
	std::vector<unsigned int> frequencies((size_t)alphabetSize, 1);

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

void Arithmetic::PPMEncode(fstream * output, fstream* input)
{
	CumulativeCountTable orderM1(vector<unsigned int>(alphabetSize + 2, 1));
	CumulativeCountTable order0(alphabetSize + 2);
	order0.AddFrequency((size_t)alphabetSize);
	order0.AddFrequency((size_t)alphabetSize + 1);

	long long memory = 0;

	int currentOrder = 0;
	int nofOrders = 5;
	int highestOrder = 0;

	vector<unordered_map<uint64_t, CumulativeCountTable*>> orders(nofOrders);

	stack<CumulativeCountTable*> previousTable;

	uint8_t byteMask = 0xffU;
	uint64_t currentContextMask = 0x0U;
	uint64_t currentContext = 0;

	orders[0][0] = new CumulativeCountTable(order0);

	if (!input) {
		std::cout << "Could not open input file!\n";
		exit(-1);
	}

	bw.SetFile(output);

	high = precision;
	low = 0x0;

	pendingBits = 0;

	uint8_t read;
	unsigned int readAux;

	//for (size_t i = 0; i <= input->size(); i++) {
	while(!input->eof()){
		//cout << "\rProcessing " << i << " of " << input->size();
		//if (i < input->size())
		input->read((char*)&read, sizeof(uint8_t));

		readAux = (unsigned int)read;
			//readAux = input->at(i);
		//readAux = read;
		/*if (i == input->size())*/if(input->eof()) {
			readAux = (int)alphabetSize;
		}

		//Search for longest context
		for (;;) {
			CumulativeCountTable* currentTable = (currentOrder == -1) ? 
				&orderM1:
				orders[currentOrder][currentContext&currentContextMask];
			while (currentTable == NULL) {
				orders[currentOrder].erase(currentContext&currentContextMask);

				currentOrder--;
				currentContextMask >>= 8;

				currentTable = (currentOrder == -1) ?
					&orderM1 :
					orders[currentOrder][currentContext&currentContextMask];
			}


			if (currentTable->GetFrequency(readAux) == 0 ) {
				//not in current context, 
				//write escape signal, update table, go to a lower context

				WriteSymbol((int)alphabetSize + 1, (previousTable.empty() || currentTable == &orderM1)?*currentTable : currentTable->Exclusion(previousTable.top()));

				//currentTable->AddFrequency(readAux);
				
				if (currentOrder < nofOrders - 1 && memory < MAXMEM) {
					uint64_t auxMask = (currentContextMask<<8)| 0xff;
					uint64_t indexAux = ((currentContext<<8)|(uint8_t)readAux) & auxMask;

					if (orders[currentOrder + 1][indexAux] == NULL ) {
						orders[currentOrder + 1][indexAux] = new CumulativeCountTable(order0);
						memory++;
						//orders[currentOrder + 1][indexAux]->AddFrequency(readAux);
						if (currentOrder + 1 > highestOrder)
							highestOrder = currentOrder + 1;
					}
				}
				
				currentTable->AddFrequency(alphabetSize + 1);
				currentOrder--;
				currentContextMask >>= 8;

				previousTable.push(currentTable);
			}
			else {
				//In contex, so write symbol, add to the frequency and update the context
				WriteSymbol(readAux, (previousTable.empty() || currentTable == &orderM1) ? *currentTable : currentTable->Exclusion(previousTable.top()));

				/*if (currentOrder != 0) {
					currentTable->AddFrequency(readAux);
				}*/

				uint64_t auxMask = currentContextMask;
				for (int k = currentOrder; k >= 0; k--) {
					if(orders[k][auxMask&currentContext] != NULL)
						orders[k][auxMask&currentContext]->AddFrequency(readAux);
					auxMask >>= 8;
				}

				currentOrder = highestOrder;
				currentContextMask = 0;
				for (int k = 0; k < currentOrder; k++) {
					currentContextMask <<= 8;
					currentContextMask |= 0xffU;
				}

				while (!previousTable.empty()) {
					previousTable.top()->AddFrequency(readAux);
					previousTable.pop();
				}

				break;
				//previousTable = NULL;
			}
		}

		currentContext <<= 8;
		currentContext |= (uint8_t)readAux;
	}

	WriteBits(1);
	bw.End();

	for (auto i = orders.begin(); i != orders.end(); i++) {
		for (auto j = i->begin(); j != i->end(); j++) {
			delete j->second;
		}
	}
	cout << "mem: " << memory << endl;
}

void Arithmetic::PPMDecode(fstream * input, vector<uint8_t>* output)
{
	CumulativeCountTable orderM1(vector<unsigned int>(alphabetSize + 2, 1));
	CumulativeCountTable order0(alphabetSize + 2);
	order0.AddFrequency((size_t)alphabetSize);
	order0.AddFrequency((size_t)alphabetSize + 1);

	int currentOrder = 0;
	int nofOrders = 5;
	int highestOrder = 0;

	vector<unordered_map<uint64_t, CumulativeCountTable*>> orders(nofOrders);

	uint8_t byteMask = 0xffU;
	uint64_t currentContextMask = 0x0U;
	uint64_t currentContext = 0;

	long long memory = 0;

	orders[0][0] = new CumulativeCountTable(order0);

	CumulativeCountTable* currentTable = orders[0][0];

	stack<CumulativeCountTable*> previousContexts;

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
		CumulativeCountTable* currentTable = (currentOrder == -1) ?
			&orderM1 :
			orders[currentOrder][currentContext&currentContextMask];
		while (currentTable == NULL) {
			orders[currentOrder].erase(currentContext&currentContextMask);

			currentOrder--;
			currentContextMask >>= 8;

			currentTable = (currentOrder == -1) ?
				&orderM1 :
				orders[currentOrder][currentContext&currentContextMask];
		}

		CumulativeCountTable currentTableAux = (previousContexts.empty() || currentTable == &orderM1) ? *currentTable : currentTable->Exclusion(previousContexts.top());


		unsigned long long range = high - low + 1;
		unsigned long long value = ((currentCode - low + 1) * currentTableAux.count - 1) / range;

		symbol = currentTableAux.Search((long long)value);

		//Update the range
		unsigned long long nLow = low + (range * currentTableAux.GetLow(symbol)) / currentTableAux.count;
		high = low + range * currentTableAux.GetHigh(symbol) / currentTableAux.count - 1;
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

		if (symbol == alphabetSize)
			break;

		if (symbol == alphabetSize + 1) {
			//If escape symbol then decrement the current order
			currentOrder--;

			
			currentTable->AddFrequency(alphabetSize + 1);
			currentContextMask >>= 8;
			previousContexts.push(currentTable);
			continue;
		}

		//else the current symbol is in the current order, then add to the order and increment the order
		uint8_t toWrite = (uint8_t)symbol;
		
		//Add thew appearance of this symbol in the higher orders
		while (!previousContexts.empty()) {
			previousContexts.top()->AddFrequency(toWrite);
			previousContexts.pop();
		}
		
		//Add new context if needed
		for (int i = highestOrder; i >= currentOrder; i--) {
			if (i < nofOrders - 1 && memory < MAXMEM) {
				uint64_t auxMask = 0;
				for (int j = 0; j <= i; j++) {
					auxMask = (auxMask << 8) | 0xff;
				}

				uint64_t indexAux = ((currentContext<<8)|toWrite) & auxMask;

				if (orders[i + 1][indexAux] == NULL ) {
					orders[i + 1][indexAux] = new CumulativeCountTable(order0);
					memory++;
					if (i + 1 > highestOrder)
						highestOrder = i + 1;
				}

			}
		}

		//Add appearence to current order if the order isn't the order -1
		uint64_t auxMask = currentContextMask;
		//Add appearances in lower contexts
		for (int k = currentOrder; k >= 0; k--) {
			if (orders[k][auxMask&currentContext] != NULL)
				orders[k][auxMask&currentContext]->AddFrequency(toWrite);
			auxMask >>= 8;
		}

		currentOrder = highestOrder;
		currentContextMask = 0;
		for (int k = 0; k < currentOrder; k++) {
			currentContextMask <<= 8;
			currentContextMask |= 0xffU;
		}

		output->emplace_back(toWrite);

		currentContext <<= 8;
		currentContext |= toWrite;

	}

	for (auto i = orders.begin(); i != orders.end(); i++) {
		for (auto j = i->begin(); j != i->end(); j++) {
			delete j->second;
		}
	}
	cout << "mem: " << memory << endl;
}

void Arithmetic::WriteSymbol(int readAux, CumulativeCountTable freq)
{
	//Update the range of high and low
	if (freq.GetFrequency(readAux) == 0) {
		cout << "WTF!\n";
	}

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