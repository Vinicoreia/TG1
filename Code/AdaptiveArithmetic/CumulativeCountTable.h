#pragma once
#include <vector>
#include <cstdint>

typedef uint64_t uint64;


class CumulativeCountTable {
	//maybe take out frequencies and leave only the limits
	std::vector<unsigned int> frequencies;
	std::vector<unsigned int> limits;

public:
	long long count;
	CumulativeCountTable(std::size_t alphabetSize);
	CumulativeCountTable(std::vector<unsigned int> frequencyTable);
	~CumulativeCountTable();
	char AddFrequency(std::size_t pos);
	unsigned int GetLow(std::size_t index);
	unsigned int GetHigh(std::size_t index);
	unsigned int GetFrequency(std::size_t index);
	int Search(long long value);
	std::size_t Search(uint64 value);

	CumulativeCountTable Exclusion(CumulativeCountTable* table);

};
