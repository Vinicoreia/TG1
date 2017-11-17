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

#define NOFALPHABET 256

using namespace std;

#define MASK 0xffffffffU
#define TOP_MASK 0x80000000U

class BitReader {
	uint64_t buffer;
	int numberOfBits;
	FILE* input;

public:
	BitReader() {
		input = NULL;
		buffer = 0;
		numberOfBits = 0;
	}

	BitReader(FILE* file) {
		input = file;
		buffer = 0;
		numberOfBits = 0;
	}

	void SetFile(FILE* file){
		input = file;
	}

	uint64_t GetBit() {
		uint8_t mask = 0x1, value;
		uint8_t read;

		//If there are no bits, read a byte from input.
		if (numberOfBits == 0) {
			fread(&read, sizeof(uint8_t), 1, input);
			if (feof(input))
				return -1;
			buffer = (uint64_t)read;
			numberOfBits = 8;
		}

		numberOfBits--;
		//Read the nth bit.
		return (buffer >> numberOfBits) & 1;
	}

	uint8_t Read8() {
		uint8_t value = 0;
		for (int i = 0; i < 8; i++) {
			value = (value << 1) | GetBit();
		}
		return value;
	}

	uint64_t Read(int bits) {
		uint64_t value = 0;
		for (int i = 0; i < bits; i++) {
			value = (value << 1) | GetBit();
		}
		return value;
	}
};

class BitWriter {
	int bitPosition;
	uint8_t toWrite;
	FILE* output;

public:
	BitWriter() {
		bitPosition = 0;
		toWrite = 0;
		output = NULL;
	}

	BitWriter(FILE* filename) {
		bitPosition = 0;
		toWrite = 0;
		output = filename;
	}

	void SetFile(FILE* file) {
		output = file;
	}

	void WriteBit(uint8_t bit) {
		toWrite = (toWrite << 1) | bit;
		bitPosition++;

		//Writes if byte is full.
		if (bitPosition == 8) {
			fwrite(&toWrite, sizeof(uint8_t), 1, output);
			toWrite = 0;
			bitPosition = 0;
		}
	}

	void Write(uint8_t value) {
		for (int i = 7; i >= 0; i--) {
			uint8_t mask = 1 << i;
			if (value&mask) {
				WriteBit(1);
			}
			else {
				WriteBit(0);
			}
		}
	}

	void Write(uint16_t value) {
		for (int i = 15; i >= 0; i--) {
			uint16_t mask = 1 << i;
			if (value&mask) {
				WriteBit(1);
			}
			else {
				WriteBit(0);
			}
		}
	}

	void Write(uint32_t value) {
		for (int i = 31; i >= 0; i--) {
			uint32_t mask = 1 << i;
			if (value&mask) {
				WriteBit(1);
			}
			else {
				WriteBit(0);
			}
		}
	}

	void Write(uint64_t value) {
		for (int i = 63; i >= 0; i--) {
			uint64_t mask = 1 << i;
			if (value&mask) {
				WriteBit(1);
			}
			else {
				WriteBit(0);
			}
		}
	}

	void Write(uint64_t value, uint8_t nOfBits) {
		for (int i = nOfBits - 1; i >= 0; i--) {
			uint64_t mask = 1 << i;
			if (value&mask) {
				WriteBit(1);
			}
			else {
				WriteBit(0);
			}
		}
	}

	void End() {
		if (bitPosition > 0) {
			toWrite <<= 8 - bitPosition;
			fwrite(&toWrite, sizeof(uint8_t), 1, output);
		}
	}
};


class FrequencyTable {
public:
	vector<long long> frequencies;
	vector<long long> limits;
	long long totalBytes;

	FrequencyTable(vector<long long> frequencies):frequencies(frequencies) {
		long long total = 0;
		for (auto &i : frequencies) {
			//cout << i << endl;
			limits.emplace_back(total);
			total += i;
		}
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

};


class ArithmeticEncoder {
	static void WriteBits(bool bit);

public:
	static BitWriter bw;
	static BitReader br;
	static int pendingBits;
	static void Encode(std::string filename, std::string outputfile);
	static void Decode(std::string filename, std::string outputfile);
};

BitWriter ArithmeticEncoder::bw;
BitReader ArithmeticEncoder::br;

int ArithmeticEncoder::pendingBits = 0;

void ArithmeticEncoder::WriteBits(bool bit) {
	//printf("%d", (bit) ? 1 : 0);
	bw.WriteBit((bit) ? (uint8_t)1 : (uint8_t)0);
	while (pendingBits > 0) {
		bw.WriteBit((bit) ? (uint8_t)0 : (uint8_t)1);
		pendingBits--;
	}
}

void ArithmeticEncoder::Encode(std::string filename, std::string outputfile) {
	FILE* input, *output;
	
	std::vector<long long> frequencies;
	unsigned long long nread = 0;

	for (int i = 0; i < NOFALPHABET; i++) {
		frequencies.emplace_back(0);
	}

	input = fopen(filename.c_str(), "rb");

	if (!input) {
		std::cout << "Could not open input file!\n";
		exit(-1);
	}

	br.SetFile(input);

	clock_t time = clock();
	do {
		uint8_t read;
		nread++;
		fread(&read, sizeof(uint8_t), 1, input);

		frequencies[read]++;
	} while (!feof(input));

	std::cout << "Read " << nread << " bytes in " << (double)(clock() - time) / CLOCKS_PER_SEC<<std::endl;
	
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


	for (int i = 0; i < frequencies.size(); i++) {
		if (frequencies[i] > 0) {
			fwrite((&i)+ 3 , sizeof(uint8_t), 1, output);
			fwrite(&frequencies[i], sizeof(long long), 1, output);
		}
	}


	pendingBits = 0;

	uint8_t read;

	do {
		fread(&read, sizeof(uint8_t), 1, input);
		unsigned long long range = high - low + 1;
		

		unsigned long long nLow = low + (range * freq.GetLow(read)) / freq.totalBytes;
		high = (low + (range * freq.GetHigh(read)) / freq.totalBytes) - 1;
		low = nLow;

		high &= MASK;
		low &= MASK;


		//While MSB are equal write bit on file
		while (((low^high) & 0x80000000U) == 0) {
			WriteBits((low >> 31)&1);
			low = (low << 1);
			high = (high << 1) | 1;
		}
		//E3, when [low, high] has 0.5
		while (((low & ~high) & 0x40000000U) != 0) {
			pendingBits++;
			low = (low << 1) & (MASK >> 1);
			high = ((high << 1) & (MASK >> 1)) | TOP_MASK | 1;
		}

	} while (!feof(input));

	

	fclose(input);
	fclose(output);
}

void ArithmeticEncoder::Decode(std::string filename, std::string outputfile) {

}




int main(int argc, char** argv) {
	std::string filename, output;

	std::cin >> filename >> output;

	ArithmeticEncoder::Encode(filename, output);

	system("PAUSE");
	return 0;
}