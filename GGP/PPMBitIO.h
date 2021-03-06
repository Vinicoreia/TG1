#pragma once

#include <cstdio>
#include <cstdint>
#include <fstream>
#include <cstdint>
#include <vector>

using namespace std;

class BitReader {
	uint64_t buffer;
	int numberOfBits;
	fstream* input;
	vector<uint8_t> data;

public:
	BitReader() {
		input = NULL;
		buffer = 0;
		numberOfBits = 0;
	}

	BitReader(fstream* file) {
		input = file;
		buffer = 0;
		numberOfBits = 0;

		while (!file->eof()) {
			uint8_t read;
			file->read((char*)&read, 1);
			data.emplace_back(read);
		}
	}

	BitReader(vector<uint8_t> data) :data(data) {}

	void SetFile(fstream* file) {
		input = file;

		while (!file->eof()) {
			uint8_t read;
			file->read((char*)&read, 1);
			data.emplace_back(read);
		}
	}

	uint64_t GetBit() {
		uint8_t mask = 0x1;
		uint8_t read;

		//If there are no bits, read a byte from input.
		if (numberOfBits == 0) {
			//input->read((char*)&read, sizeof(uint8_t));
			
			if (data.size() == 0) {
				int a = 0;
				return -1;
			}

			read = data[0];
			data.erase(data.begin());

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
			value = (value << 1) | (uint8_t)GetBit();
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
	fstream* output;

public:
	BitWriter() {
		bitPosition = 0;
		toWrite = 0;
		output = NULL;
	}

	BitWriter(fstream* filename) {
		bitPosition = 0;
		toWrite = 0;
		output = filename;
	}

	void SetFile(fstream* file) {
		output = file;
	}

	void WriteBit(uint8_t bit) {
		toWrite = (toWrite << 1) | bit;
		bitPosition++;

		//Writes if byte is full.
		if (bitPosition == 8) {
			output->write((char*)&toWrite, sizeof(uint8_t));
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
			uint64_t mask = (uint64_t)1 << i;
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
			uint64_t mask = (uint64_t)1 << i;
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
			output->write((char*)&toWrite, sizeof(uint8_t));
		}
	}
};

