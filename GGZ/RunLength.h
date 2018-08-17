#pragma once

#include "BWT.h"
#include <vector>
#include <cstring>

using namespace std;


vector<uint8_t> RunLengthEncode(vector<uint8_t> data) {
	vector<uint8_t> bytes;
	uint8_t current = data[0];
	long long counter = 0;


	for (long long i = 0; i < data.size(); i++) {
		if (current == data[i]) {
			counter++;
			if (counter == 255) {
				bytes.emplace_back(current);
				bytes.emplace_back((uint8_t)counter);
				counter = 0;
			}
		}
		else {
			if (counter != 0) {
				bytes.emplace_back(current);
				bytes.emplace_back((uint8_t)counter);

			}
			current = data[i];
			counter = 1;
		}
	}

	if (counter != 0) {
		bytes.emplace_back(current);
		bytes.emplace_back((uint8_t)counter);
	}

	return bytes;
}

vector<uint8_t> RunLengthDecode(vector<uint8_t> toDecode) {
	vector<uint8_t> result;

	for (int i = 0; i < toDecode.size(); i+= 2) {
		uint8_t current;
		current = toDecode[i];
		for (int j = 0; j < toDecode[i + 1]; j++) {
			result.emplace_back(current);
		}
	}

	return result;
}