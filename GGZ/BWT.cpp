#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <unordered_map>
#include <cstdint>
#include <algorithm>
#include <cstring>

#include "BWT.h"

using namespace std;

vector<uint8_t> original;

int SortCompare(const vector<uint8_t> &a, const vector<uint8_t> &b) {
	/*for (int i = 0; i < a.size(); i++) {
	if (a[i] != b[i]) {
	return a[i] < b[i];
	}
	}*/

	return a < b;
}

int SortCompare1(const long long a, const long long b) {
	vector<uint8_t> vecA, vecB;

	vecA.resize(original.size());
	vecB.resize(original.size());

	memcpy(vecA.data(), original.data() + a, original.size() - a);
	memcpy(vecA.data() + original.size() - a, original.data(), a);

	memcpy(vecB.data(), original.data() + b, original.size() - b);
	memcpy(vecB.data() + original.size() - b, original.data(), b);

	return vecA < vecB;

	/*for (long long i = 0; i < original.size(); i++) {
		if (vecA[i] != vecB[i]) {
			return vecA[i] - vecB[i];
		}
	}
	return 0;*/
}

int PairCompare(pair<uint8_t, int> &a, pair<uint8_t, int> &b) {
	return a.first < b.first;
}

int Compare(const vector<uint8_t> &a, const vector<uint8_t> &b) {
	for (int i = 0; i < a.size(); i++) {
		if (a[i] != b[i]) {
			return a[i] - b[i];
		}
	}
	return 0;
}

unsigned short Find(vector<vector<uint8_t>> *vec, vector<uint8_t> *find) {
	int a, b;
	a = 0;
	b = vec->size();

	while (b - a > 1) {
		int mid = (a + b) >> 1;
		if (*find < vec->at(mid)) {
			b = mid;
		}
		else /*if(find > vec[mid])*/ {
			a = mid;
		}/*
		 else
		 return mid;*/
	}
	return (unsigned short)a;
}

Data BWT::Reverse(Data toReverse, long long index) {
	Data reverse(toReverse.size);

	unordered_map<uint8_t, vector<int>> seen;
	vector<pair<uint8_t, int>> l;

	for (int i = 0; i < toReverse.size; i++) {
		l.emplace_back(make_pair(toReverse.data[i], seen[toReverse.data[i]].size()));
		seen[toReverse.data[i]].emplace_back(i);
	}

	vector<pair<char, int>> f(l.begin(), l.end());

	sort(f.begin(), f.end());

	for (long long i = 0; i < toReverse.size; i++) {
		//Add the character to the reverse transform string
		reverse.data[i] = f[index].first;
		//update the index using the character at f[index] and the weight of this character. Described in Data Compression book page 155.
		index = seen[f[index].first][f[index].second];

		cout << f[index].first;
	}


	return reverse;
}

void BWT::Reverse(Data * toReverse, long long index)
{
	unordered_map<uint8_t, vector<int>> seen;
	vector<pair<uint8_t, int>> l;

	for (int i = 0; i < toReverse->size; i++) {
		l.emplace_back(make_pair(toReverse->data[i], seen[toReverse->data[i]].size()));
		seen[toReverse->data[i]].emplace_back(i);
	}

	vector<pair<uint8_t, int>> f(l.begin(), l.end());

	sort(f.begin(), f.end());

	vector<uint8_t> ret(toReverse->size);

	for (int i = 0; i < toReverse->size; i++) {
		//Add the character to the reverse transform string
		toReverse->data[i] = f[index].first;
		ret[i] = (f[index].first);
		//update the index using the character at f[index] and the weight of this character. Described in Data Compression book page 155.
		index = seen[f[index].first][f[index].second];
	}

	return;
}



long long BWT::Transform(Data* toTransform) {
	//vector<vector<uint8_t>> rotations(toTransform->size, vector<uint8_t>(toTransform->size));
	vector<long long> rotations;
	original.resize(toTransform->size);

	for (long long i = 0; i < toTransform->size; i++) {
		original[i] = toTransform->data[i];
		rotations.emplace_back(i);
	}

	/*for (int i = 0; i < toTransform->size; i++) {
		/*for (int j = 0, k = i; j < toTransform->size; j++, k++) {
		rotations[i][j] = toTransform->data[k%toTransform->size];
		}///
		memcpy(rotations[i].data(), original.data() + i, original.size() - i);
		memcpy((rotations[i].data() + toTransform->size - i), toTransform->data, i);
	}*/

	sort(rotations.begin(), rotations.end(), SortCompare1);

	long long index;

	for (long long i = 0; i < rotations.size(); i++) {
		if (rotations[i] == 0) {
			index = i;
			break;
		}
	}

	for (long long i = 0; i < rotations.size(); i++) {
		toTransform->data[i] = original[(rotations[i] - 1 + rotations.size())%rotations.size()];
	}

	return index;
}

/*
int main() {
	fstream file;
	string filename;
	vector<uint8_t> read;
	getline(cin, filename);

	file.open(filename, fstream::in | fstream::binary);

	for(;;) {
		uint8_t byte;
		file.read((char*)&byte, 1);
		if (file.eof()) {
			break;
		}
		read.emplace_back(byte);
	}

	file.close();

	Data data(read.size());
	//uint8_t* toTransform = new uint8_t[read.size()];

	for (int i = 0; i < read.size(); i++) {
		data.data[i] = read[i];
	}


	//pair<Data, int> index;
	unsigned short index = BWT::Transform(&data);
	
	getline(cin, filename);

	file.open(filename, fstream::out | fstream::binary);

	file.write((char*)(data.data), data.size);

	file.close();

	BWT::Reverse(&data, index);

	cin >> filename;
	file.open(filename, fstream::out | fstream::binary);

	file.write((char*)data.data, data.size);

	file.close();
	system("PAUSE");
	return 0;
}
*/