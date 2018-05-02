#pragma once

#include <cstdint>
#include <utility>

class BWTData {
public:
	uint8_t *data;
	int size;

	BWTData(int size) {
		this->size = size;
		data = new uint8_t[size];
	}
	BWTData() {
		size = 0;
	}
};

class BWT {
public:
	//static pair<string, int> Transform(string toTransform);
	static std::pair<BWTData, int> Transform(uint8_t* toTransform, int size);
	static BWTData Reverse(BWTData toReverse, int index);

};

int Compare(BWTData &a, BWTData &b);