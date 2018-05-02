#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <unordered_map>
#include <cstdint>
#include <algorithm>

#include "BWT.h"

using namespace std;

int Compare(BWTData& a, BWTData& b) {
	for (int i = 0; i < a.size; i++) {
		if (a.data[i] != b.data[i]) {
			return a.data[i] < b.data[i];
		}
	}
	return 0;
}

BWTData BWT::Reverse(BWTData toReverse, int index) {
	BWTData reverse(toReverse.size);
	
	unordered_map<uint8_t, vector<int>> seen;
	vector<pair<uint8_t, int>> l;

	for (int i = 0; i < toReverse.size; i++) {
		l.emplace_back(make_pair(toReverse.data[i], seen[toReverse.data[i]].size()));
		seen[toReverse.data[i]].emplace_back(i);
	}

	vector<pair<char, int>> f(l.begin(), l.end());

	sort(f.begin(), f.end());

	for (int i = 0; i < toReverse.size; i++) {
		//Add the character to the reverse transform string
		reverse.data[i] = f[index].first;
		//update the index using the character at f[index] and the weight of this character. Described in BWTData Compression book page 155.
		index = seen[f[index].first][f[index].second];

		cout << f[index].first;
	}


	return reverse;
}

pair<BWTData, int> BWT::Transform(uint8_t* toTransform, int size) {
	vector<BWTData> rotations;
	BWTData original(size);
	BWTData* ret = new BWTData(size);

	for (int i = 0; i < size; i++) {
		original.data[i] = toTransform[i];
	}

	clock_t start = clock();

	for (int i = 0; i < size; i++) {
		rotations.emplace_back(BWTData(size));

		for (int j = 0, k = i; j < size; j++, k++) {
			rotations[i].data[j] = toTransform[k%size];
		}
	}

	sort(rotations.begin(), rotations.end(), Compare);

	cout << "Made all rotations in: " << ((double)(clock() - start) / CLOCKS_PER_SEC) << "seconds\n";

	for (int i = 0; i < size; i++) {
		ret->data[i] = rotations[i].data[size-1];
	}

	for (int i = 0; i < rotations.size(); i++) {
		if (!Compare(rotations[i], original) && !Compare(original, rotations[i]))
			return make_pair(*ret, i);
	}
}
