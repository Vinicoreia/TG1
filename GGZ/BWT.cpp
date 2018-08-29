#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <unordered_map>
#include <cstdint>
#include <algorithm>
#include <queue>
#include <cstring>

#include "BWT.h"

using namespace std;

vector<uint8_t> original;


typedef struct slices {
	int64_t c[2];
	size_t p;
}Slices;

int SliceCmp(Slices a, Slices b) {
	return (a.c[0] == b.c[0]) ? (a.c[1] < b.c[1]) : (a.c[0] < b.c[0]);
}

int SliceEq(Slices a, Slices b) {
	return (a.c[0] == b.c[0] && a.c[1] == b.c[1]);
}

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
}

bool SortCompareF(int& a, int& b) {
	return original[a] < original[b];
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
	vector<int64_t> previous(toTransform->size);
	vector<Slices> slices(toTransform->size);

	original.resize(toTransform->size);
	memcpy(original.data(), toTransform->data, toTransform->size);

	vector<pair<uint8_t, int>> positions;

	vector<queue<size_t>> places(256);

	for (int i = 0; i < toTransform->size; i++) {
		positions.emplace_back(make_pair(toTransform->data[i], i));
	}

	sort(positions.begin(), positions.end());

	int pos[1<<(sizeof(uint8_t)*8)];

	memset(pos, -1, sizeof(int)*(1 << (sizeof(uint8_t) * 8)));

	for (int i = 0, rank = 0; i < positions.size(); i++) {
		if (pos[positions[i].first] == -1) {
			pos[positions[i].first] = rank++;
		}
	}

	//memcpy(previous.data(), toTransform->data, toTransform->size);
	for (size_t i = 0; i <toTransform->size; i++) {
		previous[i] = pos[toTransform->data[i]];
	}




	for (int i = 1; i>>1 < toTransform->size; i<<=1) {
		for (int j = 0; j < toTransform->size; j++) {
			slices[j].c[0] = previous[j];
			
			if (i + j < toTransform->size)
				slices[j].c[1] = previous[i + j];
			else 
				slices[j].c[1] = previous[(i+j)%toTransform->size];

			slices[j].p = j;
		}

		sort(slices.begin(), slices.end(), SliceCmp);

		previous[slices[0].p] = 0;

		for (int j = 1; j < toTransform->size; j++) {
			previous[slices[j].p] = SliceEq(slices[j], slices[j - 1]) ? previous[slices[j - 1].p] : j;
		}
	}

	//vector<vector<uint8_t>> rotations(toTransform->size, vector<uint8_t>(toTransform->size));
	//vector<long long> rotations;
	
	//for (long long i = 0; i < toTransform->size; i++) {
	//	rotations.emplace_back(i);
	//}

	/*for (int i = 0; i < toTransform->size; i++) {
		/*for (int j = 0, k = i; j < toTransform->size; j++, k++) {
		rotations[i][j] = toTransform->data[k%toTransform->size];
		}///
		memcpy(rotations[i].data(), original.data() + i, original.size() - i);
		memcpy((rotations[i].data() + toTransform->size - i), toTransform->data, i);
	}*/

	//sort(rotations.begin(), rotations.end(), SortCompare1);

	long long index;

	/*for (long long i = 0; i < previous.size(); i++) {
		if (previous[i] == 0) {
			index = i;
			break;
		}
	}*/

	for (long long i = 0; i < previous.size(); i++) {
		toTransform->data[previous[i]] = original[(i - 1 + previous.size())%previous.size()];
	}

	return previous[0];
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