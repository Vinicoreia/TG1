#pragma once

#include <cstdint>
#include <utility>
#include <cstdlib>


class Data {
public:
	uint8_t * data;
	long long size;

	Data(int size) {
		this->size = size;
		data = (uint8_t*)malloc(size * sizeof(uint8_t));
	}
	Data() {
		data = 0;
		size = 0;
	}

	~Data() {
		delete[] data;
	}

	Data(const Data& copy) {
		data = (uint8_t*)malloc(copy.size * sizeof(uint8_t));
		size = copy.size;

		for (int i = 0; i < size; i++) {
			data[i] = copy.data[i];
		}
	}
};


class BWT {
public:
	//static pair<string, int> Transform(string toTransform);
	//static std::pair<Data, short> Transform(uint8_t* toTransform, int size);
	static Data Reverse(Data toReverse, long long index);
	static void Reverse(Data *toReverse, long long index);
	static long long Transform(Data* toTransform);

};
