#pragma once
#include <vector>
#include <cstdint>

typedef uint64_t uint64;

class CumulativeCountTable {
	//maybe take out frequencies and leave only the limits
	std::vector<long long> frequencies;
	std::vector<long long> limits;

public:
	long long count;
	CumulativeCountTable(size_t alphabetSize);
	CumulativeCountTable(std::vector<long long> frequencyTable);
	char AddFrequency(size_t pos);
	long long GetLow(size_t index);
	long long GetHigh(size_t index);
	int Search(long long value);
	size_t Search(uint64 value);
};
