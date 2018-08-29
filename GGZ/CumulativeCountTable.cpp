#include "CumulativeCountTable.h"

using namespace std;

CumulativeCountTable::CumulativeCountTable(size_t alphabetSize) :frequencies(alphabetSize, 0), limits(alphabetSize, 0) {
	count = 0;
}

CumulativeCountTable::CumulativeCountTable(vector<long long> frequencyTable):frequencies(frequencyTable), limits(frequencyTable.size()) {
	count = 0;

	for (size_t i = 0; i < frequencies.size(); i++) {
		limits[i] = count;
		count += frequencies[i];
	}
}

char CumulativeCountTable::AddFrequency(size_t pos) {
	if (pos >= frequencies.size())
		return 0;
	frequencies[pos]++;
	count++;

	for (auto i = pos + 1; i < frequencies.size(); i++) {
		limits[i]++;
	}

	return 1;
}

long long CumulativeCountTable::GetLow(size_t index) {
	if (index >= frequencies.size())
		return -1;
	return limits[index];
}

long long CumulativeCountTable::GetHigh(size_t index) {
	if (index >= frequencies.size())
		return -1;
	return limits[index] + frequencies[index];
}

int CumulativeCountTable::Search(long long value) {
	long long start = 0;
	long long end = frequencies.size();
	while ((end - start > 1)) {
		long long mid = (start + end) >> 1;
		if ((size_t)GetLow(mid) > value)
			end = mid;
		else
			start = mid;
	}
	return (int)start;
}

size_t CumulativeCountTable::Search(uint64 value) {
	size_t start = 0;
	size_t end = frequencies.size();

	while (end - start > 1) {
		size_t mid = (start + end) >> 1;
		if (GetLow(mid) > value)
			end = mid;
		else
			start = mid;
	}

	return start;
}