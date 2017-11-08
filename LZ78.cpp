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

class Tree {
public:
	vector<Tree*> childs;
	Tree* parent;
	vector<uint8_t> bytes;
	long long index;

	Tree(Tree* parent, vector<uint8_t> bytes, long long index) : bytes(bytes.begin(), bytes.end()), parent(parent),index(index){
		childs.resize(256);
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

	FILE *inFileP, *outFileP;

	int load = 0;

	long long counter = 1;

	long long indexSize = 1;
	long long index = 0;
	uint8_t index8 = 0;
	uint16_t index16 = 0;
	uint32_t index32 = 0;
	char laod[] = { '|', '\\', '/', '|' };

	inFileP = fopen(filename.c_str(), "rb");
	outFileP = fopen(outputfile.c_str(), "wb");

	std::cout << "Writing ";
	while (!feof(inFileP)) {
		std::cout << laod[load++] << "\b";
		load %= 4;

		cout << dictionary.size()<<endl;

		fread(&read, sizeof(uint8_t), 1, inFileP);
		if (feof(inFileP))
			break;

		if (currentPos->childs[read] != NULL) {
			currentPos = currentPos->childs[read];
			aux.emplace_back(read);
		}

		else {
			currentPos->childs[read] = new Tree(currentPos, aux, counter);

			if (dictionary.size() <= 255) {
				index8 = (uint8_t)currentPos->index;
				fwrite(&index8, sizeof(uint8_t), 1, outFileP);

				if (!feof(inFileP)) {
					fwrite(&read, sizeof(uint8_t), 1, outFileP);
				}
			}

			else if (dictionary.size() > 255 && dictionary.size() <= 65535) {
				index16 = (uint16_t)currentPos->index;
				fwrite(&index16, sizeof(uint8_t), 2, outFileP);

				if (!feof(inFileP)) {
					fwrite(&read, sizeof(uint8_t), 1, outFileP);
				}
			}

			else if (dictionary.size() > 65535 && dictionary.size() <= (4294967295)) {
				index32 = (uint32_t)currentPos->index;
				fwrite(&index32, sizeof(uint32_t), 1, outFileP);

				if (!feof(inFileP)) {
					fwrite(&read, sizeof(uint8_t), 1, outFileP);
				}
			}

			else {
				fwrite(&currentPos->index, sizeof(long long), 1, outFileP);
				if (!feof(inFileP))
					fwrite(&read, sizeof(uint8_t), 1, outFileP);
			}

			dictionary[counter++] = currentPos->childs[read];

			if (counter%MAX == 0) {
				counter = 1;
			}

			aux.clear();
			currentPos = &base;
		}
	}

	fclose(inFileP);
	fclose(outFileP);
}

void LZ78::Decode(string filename, string outputfile) {
	unordered_map<long long, Tree*> dictionary;

	uint8_t read;
	vector<uint8_t> aux;

	Tree base(NULL, aux, 0);
	Tree* currentPos = &base;

	FILE *inFileP, *outFileP;

	int load = 0;

	long long counter = 1;

	long long indexSize = 1;
	long long index = 0;
	uint8_t index8 = 0;
	uint16_t index16 = 0;
	uint32_t index32 = 0;
	char laod[] = { '|', '\\', '/', '|' };

	inFileP = fopen(filename.c_str(), "rb");
	outFileP = fopen(outputfile.c_str(), "wb");

	std::cout << "Writing...\n";

	do {
		aux.clear();
		//std::cout << laod[load++] << "\b";
		//cout << dictionary.size()<< "\r";
		//load %= 4;

		cout << dictionary.size() << endl;

		currentPos = &base;

		index = 0;
		index8 = 0;
		index16 = 0;
		index32 = 0;
		read = 0;

		//Check how many bytes are needed for the index, based on the dictionary size.
		if (dictionary.size() <= 255) {
			fread(&index8, sizeof(uint8_t), 1, inFileP);
			fread(&read, sizeof(uint8_t), 1, inFileP);

			if (index8 == 0) {
				aux.emplace_back(read);
				currentPos->childs[read] = new Tree(currentPos, aux, counter);
				dictionary[counter++] = currentPos->childs[read];

				if (!feof(inFileP))
					fwrite(&read, sizeof(uint8_t), 1, outFileP);
			}
			else {
				currentPos = dictionary[(uint64_t)index8];

				aux = currentPos->bytes;
				if (!feof(inFileP)) {
					aux.emplace_back(read);
				}

				currentPos->childs[read] = new Tree(currentPos, aux, counter);
				dictionary[counter++] = currentPos->childs[read];

				for (int i = 0; i < aux.size(); i++) {
					fwrite(&aux[i], sizeof(uint8_t), 1, outFileP);
				}
			}
		}
		else if (dictionary.size() > 255 && dictionary.size() <= 65535) {
			fread(&index16, sizeof(uint16_t), 1, inFileP);
			fread(&read, sizeof(uint8_t), 1, inFileP);

			if (index16 == 0) {
				aux.emplace_back(read);
				currentPos->childs[read] = new Tree(currentPos, aux, counter);
				dictionary[counter++] = currentPos->childs[read];
				if (!feof(inFileP))
					fwrite(&read, sizeof(uint8_t), 1, outFileP);
			}
			else {
				currentPos = dictionary[(uint64_t)index16];

				aux = currentPos->bytes;
				if (!feof(inFileP)) {
					aux.emplace_back(read);
				}

				currentPos->childs[read] = new Tree(currentPos, aux, counter);
				dictionary[counter++] = currentPos->childs[read];

				for (int i = 0; i < aux.size(); i++) {
					fwrite(&aux[i], sizeof(uint8_t), 1, outFileP);
				}
			}
		}
		else if (dictionary.size() > 65535 && dictionary.size() <= (4294967295)) {
			fread(&index32, sizeof(uint32_t), 1, inFileP);
			fread(&read, sizeof(uint8_t), 1, inFileP);

			if (index32 == 0) {
				aux.emplace_back(read);
				currentPos->childs[read] = new Tree(currentPos, aux, counter);
				dictionary[counter++] = currentPos->childs[read];
				if (!feof(inFileP))
					fwrite(&read, sizeof(uint8_t), 1, outFileP);
			}
			else {
				currentPos = dictionary[(uint64_t)index32];

				aux = currentPos->bytes;
				if (!feof(inFileP)) {
					aux.emplace_back(read);
				}

				currentPos->childs[read] = new Tree(currentPos, aux, counter);
				dictionary[counter++] = currentPos->childs[read];

				for (int i = 0; i < aux.size(); i++) {
					fwrite(&aux[i], sizeof(uint8_t), 1, outFileP);
				}
			}
		}
		else {
			fread(&index, sizeof(uint64_t), 1, inFileP);
			fread(&read, sizeof(uint8_t), 1, inFileP);

			if (index == 0) {
				aux.emplace_back(read);
				currentPos->childs[read] = new Tree(currentPos, aux, counter);
				dictionary[counter++] = currentPos->childs[read];
				if(!feof(inFileP))
					fwrite(&read, sizeof(uint8_t), 1, outFileP);
			}
			else {
				currentPos = dictionary[(uint64_t)index32];

				aux = currentPos->bytes;
				if (!feof(inFileP)) {
					aux.emplace_back(read);
				}

				currentPos->childs[read] = new Tree(currentPos, aux, counter);
				dictionary[counter++] = currentPos->childs[read];

				for (int i = 0; i < aux.size(); i++) {
					fwrite(&aux[i], sizeof(uint8_t), 1, outFileP);
				}
			}
		}

		if ((counter%MAX) == 0) {
			counter = 1;
		}

	} while (!feof(inFileP));

	fclose(inFileP);
	fclose(outFileP);
}

#undef ll
#undef P
#undef UM
#undef V8
#undef MAX