#pragma once
#include "CumulativeCountTable.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

class Context {
	uint64_t currentContext;
	int currentOrder;
	uint64_t contextMask;

	CumulativeCountTable orderM1;
	CumulativeCountTable* order0;
public:
	Context(uint64_t mask = 0xffffffffUL, size_t alphabetSize = 256):orderM1(std::vector<long long>(alphabetSize + 2, 1)) {
		std::vector<long long> aux(alphabetSize, 0);
		aux.emplace_back(1);
		aux.emplace_back(1);

		order0 = new CumulativeCountTable(aux);
	}

	~Context() {
		delete order0;
	}

	uint64_t AddContext(uint8_t next);


};