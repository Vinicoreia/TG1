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

	/*bool addEscape;

	addEscape = (frequencies[pos] == 0);

	frequencies[pos]++;
	count++;

	for (auto i = pos + 1; i < frequencies.size() - 1; i++) {
		limits[i]++;
	}

	if (addEscape) {
		count++;
		frequencies.back()++;
		limits.back()++;
	}*/

	count = 0;
	int escapeSize = 0;

	frequencies[pos]++;

	for (size_t i = 0; i < frequencies.size() - 1; i++) {
		limits[i] = count;
		count += frequencies[i];
		if (frequencies[i])
			escapeSize++;
	}

	limits[frequencies.size() - 1] = count;
	frequencies[frequencies.size() - 1] = escapeSize;
	count += escapeSize;

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

long long CumulativeCountTable::GetFrequency(size_t index)
{
	return frequencies[index];
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

CumulativeCountTable CumulativeCountTable::Exclusion(CumulativeCountTable * table)
{
	vector<long long> freq(frequencies.size());
	int escapeCount = 1;

	for (size_t i = 0; i < frequencies.size(); i++) {
		if (!table->GetFrequency(i) && frequencies[i]) {
			freq[i] = frequencies[i];
			escapeCount++;
		}
	}

	freq[frequencies.size() - 2]++; //Add eof
	freq[frequencies.size() - 1] = escapeCount;

	return CumulativeCountTable(freq);
}
