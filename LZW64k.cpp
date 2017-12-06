#include <fstream>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <iostream>
#include <string>
#include <utility>
#include <map>
#include <ctime>

//65535|2147483648|4294967295

#define ll long long
#define P(a,b) pair<a,b>
#define UM(a,b) unordered_map<a, b>
#define V8 vector<uint8_t>/*
#define MAX 65535

*/
#define MAX 65535

using namespace std;

/*
*	Tree for checking previously viewed sequences
*/
class Tree {
public:
	vector<Tree*> childs;
	Tree* parent;
	vector<uint8_t> bytes;
	long long index;

	Tree(Tree* parent, vector<uint8_t> bytes, long long index) : bytes(bytes.begin(), bytes.end()), parent(parent),index(index){
		childs.resize(256);
	}

	~Tree() {
	}


	bool IsLeaf() {
		for (int i = 0; i < childs.size(); i++) {
			if (childs[i] != NULL) {
				return false;
			}
		}
		return true;
	}

	void RemoveChild(uint8_t index) {
		delete childs[index];
		childs[index] = NULL;
	}

	bool IsParent(Tree* check) {
		Tree* aux = parent;
		while (aux != NULL) {
			if (check == aux) {
				return true;
			}
			aux = aux->parent;
		}
		return false;
	}
};



class LZ78 {
public:
	static void Encode(string filename, string outputfile);
	static void Decode(string filename, string outputfile);
};

int main(int argc, char** argv) {
	//TODO 
	//MENU
	string filename;
	string outputfile;
	cout << "File to be encoded: ";
	cin >> filename;
	cout << "Output file: ";
	cin >> outputfile;
	clock_t begin = clock();
	
	LZ78::Encode(filename, outputfile);

	cout << "\nEncoded in: " << (clock() - begin) / CLOCKS_PER_SEC << " seconds\n";

	cout << "File to be decoded: ";
	cin >> filename;
	cout << "Output file name: ";
	cin >> outputfile;

	begin = clock();
	LZ78::Decode(filename, outputfile);
	cout << "\nDencoded in: " << (clock() - begin) / CLOCKS_PER_SEC << " seconds\n";

	system("PAUSE");

	return 0;
}

void LZ78::Encode(string filename, string outputfile) {
	//unordered_map<long long, pair<long long, uint8_t>> dictionary;
	unordered_map<long long, Tree*> dictionary;

	uint8_t read;
	vector<uint8_t> aux;

	Tree base(NULL, aux, 0);
	Tree* currentPos = &base;

	//dictionary[0] = &base;

	for (int i = 0; i < 256; i++) {
		aux.clear();
		aux.emplace_back((uint8_t)i);
		base.childs[i] = new Tree(&base, aux, i);
		dictionary[i] = base.childs[i];
	}

	FILE *inFileP, *outFileP;

	int load = 0;

	unsigned long long size = 0;

	long long counter = 256;

	long long indexSize = 1;
	long long index = 0;
	uint8_t index8 = 0;
	uint16_t index16 = 0;
	uint32_t index32 = 0;
	char laod[] = { '|', '\\', '/', '|' };

	inFileP = fopen(filename.c_str(), "rb");
	outFileP = fopen(outputfile.c_str(), "wb");

	std::cout << "Encoding...\n";
	do {
		fread(&read, sizeof(uint8_t), 1, inFileP);

		if (feof(inFileP)) {
			if (dictionary.size() > 255 && dictionary.size() <= 65535) {
				index16 = (uint16_t)currentPos->index;
				
				if (dictionary[currentPos->index] != currentPos)
					index16 = 0;

				fwrite(&index16, sizeof(uint16_t), 1, outFileP);
			}
		}

		else if (currentPos->childs[read] != NULL) {
			currentPos = currentPos->childs[read];
			aux.emplace_back(read);
		}

		else {
			aux.emplace_back(read);

			if (dictionary.size() < MAX)
				currentPos->childs[read] = new Tree(currentPos, aux, counter);

			if (dictionary.size() > 255 && dictionary.size() <= 65535) {
				index16 = (uint16_t)currentPos->index;
				fwrite(&index16, sizeof(uint16_t), 1, outFileP);
			}

			
			if (dictionary.size() < MAX) {
				dictionary[counter++] = currentPos->childs[read];
			}

			aux.clear();
			aux.emplace_back(read);
			currentPos = base.childs[read];
			
			if (counter%MAX == 0) {
				counter = 256;
			}

			

			index8 = 0;
			index16 = 0;
		}
	} while (!feof(inFileP));

	fclose(inFileP);
	fclose(outFileP);
}

void LZ78::Decode(string filename, string outputfile) {
	unordered_map<long long, Tree*> dictionary;

	uint8_t read;
	vector<uint8_t> aux;

	Tree base(NULL, aux, 0);
	Tree* currentPos = &base;
	Tree* nextPos = &base;

	for (int i = 0; i < 256; i++) {
		aux.clear();
		aux.emplace_back((uint8_t)i);
		base.childs[i] = new Tree(&base, aux, i);
		dictionary[i] = base.childs[i];
	}

	FILE *inFileP, *outFileP;

	int load = 0;

	long long counter = 256;

	long long indexSize = 1;
	long long index = 0;
	uint8_t index8 = 0;
	uint16_t index16a = 0;
	uint16_t index16b = 0;
	uint32_t index32 = 0;

	inFileP = fopen(filename.c_str(), "rb");
	outFileP = fopen(outputfile.c_str(), "wb");

	std::cout << "Writing...\n";

	fread(&index16a, sizeof(uint16_t), 1, inFileP);
	currentPos = dictionary[(uint64_t)index16a];

	while (!feof(inFileP)){
		index16b = 0;
		aux.clear();

		if (dictionary.size() >= 256 && dictionary.size() < 65536) {
			fread(&index16b, sizeof(uint16_t), 1, inFileP);
		
			aux = currentPos->bytes;

			if (!feof(inFileP)) {
				nextPos = dictionary[(uint64_t)index16b];

				if (nextPos == NULL) {
					aux.emplace_back(aux[0]);
					if (dictionary.size() < MAX)
						currentPos->childs[aux[0]] = new Tree(currentPos, aux, counter);
					nextPos = currentPos->childs[aux[0]];
				}
				else {
					aux.emplace_back(nextPos->bytes.front());
					if (dictionary.size() < MAX)
						currentPos->childs[nextPos->bytes.front()] = new Tree(currentPos, aux, counter);
				}
				
				if (dictionary.size() < MAX) {
					dictionary[counter++] = currentPos->childs[nextPos->bytes.front()];
				}
			}
		}

		for (int i = 0; i < currentPos->bytes.size(); i++) {
			fwrite(&currentPos->bytes[i], sizeof(uint8_t), 1, outFileP);
		}

		if (counter%MAX == 0) {
			counter = 256;
		}

		currentPos = nextPos;
	}

	fclose(inFileP);
	fclose(outFileP);
}

#undef ll
#undef P
#undef UM
#undef V8
#undef MAX