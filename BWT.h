#pragma once

#include <cstdint>
#include <utility>

class Data {
public:
	uint8_t *data;
	int size;

	Data(int size) {
		this->size = size;
		data = new uint8_t[size];
	}
	Data() {
		size = 0;
	}
};

class BWT {
public:
	//static pair<string, int> Transform(string toTransform);
	static std::pair<Data, int> Transform(uint8_t* toTransform, int size);
	static Data Reverse(Data toReverse, int index);

};