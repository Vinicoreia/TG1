#pragma once
#include <vector>
#include <cstdint>

typedef uint64_t uint64;


class PPMCumulativeCountTable {
	//maybe take out frequencies and leave only the limits
	std::vector<unsigned int> frequencies;
	std::vector<unsigned int> limits;

public:
	long long count;
	PPMCumulativeCountTable(std::size_t alphabetSize);
	PPMCumulativeCountTable(std::vector<unsigned int> frequencyTable);
	~PPMCumulativeCountTable();
	char AddFrequency(std::size_t pos);
	unsigned int GetLow(std::size_t index);
	unsigned int GetHigh(std::size_t index);
	unsigned int GetFrequency(std::size_t index);
	int Search(long long value);
	std::size_t Search(uint64 value);

	PPMCumulativeCountTable Exclusion(PPMCumulativeCountTable* table);

};
