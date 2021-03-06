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


	return 0;
}

void LZ78::Encode(string filename, string outputfile) {
	/*
	The dictionary consists of an index and a pointer to a node in the tree.
	*/
	unordered_map<long long, Tree*> dictionary;

	uint8_t read;
	vector<uint8_t> aux;

	Tree base(NULL, aux, 0);
	Tree* currentPos = &base;

	dictionary[0] = &base;

	FILE *inFileP, *outFileP;

	int load = 0;

	unsigned long long size = 0;

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
	do {

		fread(&read, sizeof(uint8_t), 1, inFileP);

		//If feof flag is set write the index or write the last read index
		if (feof(inFileP)) {
			if (dictionary.size() <= 255) {
				index8 = (uint8_t)currentPos->index;
				if (dictionary[currentPos->index] != currentPos)
					index8 = 0;
				
				if (index8 == 0)
					fwrite(&aux[0], sizeof(uint8_t), 1, outFileP);
			}

			else if (dictionary.size() > 255 && dictionary.size() <= 65535) {
				index16 = (uint16_t)currentPos->index;
				
				if (dictionary[currentPos->index] != currentPos)
					index16 = 0;

				fwrite(&index16, sizeof(uint16_t), 1, outFileP);

				if (index16 == 0)
					fwrite(&aux[0], sizeof(uint8_t), 1, outFileP);

				size += 3;
			}

			else if (dictionary.size() > 65535 && dictionary.size() <= (4294967295)) {
				index32 = (uint32_t)currentPos->index;
				fwrite(&index32, sizeof(uint32_t), 1, outFileP);
			}

			else {
				fwrite(&currentPos->index, sizeof(long long), 1, outFileP);
			}
		}
		else if (currentPos->childs[read] != NULL) { //If the current string was already seen, then just add the current read byte to the string
			currentPos = currentPos->childs[read];
			aux.emplace_back(read);
		}
		//Else the string read is a new string, then it should be added to the dictionary;
		else {
			aux.emplace_back(read);

			currentPos->childs[read] = new Tree(currentPos, aux, counter);

			if (dictionary.size() <= 255) {
				index8 = (uint8_t)currentPos->index;
				fwrite(&index8, sizeof(uint8_t), 1, outFileP);

				fwrite(&read, sizeof(uint8_t), 1, outFileP);
			}

			else if (dictionary.size() > 255 && dictionary.size() <= 65535) {
				index16 = (uint16_t)currentPos->index;
				fwrite(&index16, sizeof(uint16_t), 1, outFileP);

				fwrite(&read, sizeof(uint8_t), 1, outFileP);
			}

			else if (dictionary.size() > 65535 && dictionary.size() <= 4294967295) {
				index32 = (uint32_t)currentPos->index;
				fwrite(&index32, sizeof(uint16_t), 1, outFileP);

				fwrite(&read, sizeof(uint8_t), 1, outFileP);
			}

			//If the dictionary is no full
			if (dictionary.size() < MAX) {
				dictionary[counter++] = currentPos->childs[read];
			}
			else {
				//Find a leaf node to be replaced
				while (!dictionary[counter]->IsLeaf()) {
					counter++;
					if (counter%MAX == 0) {
						counter = 1;
					}
				}
				//cout << counter << " ";
				Tree* parent = dictionary[counter]->parent;

				
				parent->RemoveChild(dictionary[counter]->bytes.back());

				currentPos->childs[read]->index = counter;
				dictionary[counter++] = currentPos->childs[read];
			}
			
			if (counter%MAX == 0) {
				counter = 1;
			}

			aux.clear();
			currentPos = &base;

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

	dictionary[0] = &base;

	FILE *inFileP, *outFileP;

	int load = 0;

	long long counter = 1;

	long long indexSize = 1;
	long long index = 0;
	uint8_t index8 = 0;
	uint16_t index16 = 0;
	uint32_t index32 = 0;

	inFileP = fopen(filename.c_str(), "rb");
	outFileP = fopen(outputfile.c_str(), "wb");

	std::cout << "Writing...\n";

	while (!feof(inFileP)){
		aux.clear();
		currentPos = &base;
		index8 = 0;
		index16 = 0;
		index32 = 0;

		if (dictionary.size() < 256) {
			fread(&index8, sizeof(uint8_t), 1, inFileP);
			fread(&read, sizeof(uint8_t), 1, inFileP);


			//First time appearing
			if (index8 == 0) {
				aux.emplace_back(read);

				currentPos->childs[read] = new Tree(currentPos, aux, counter);
				dictionary[counter++] = currentPos->childs[read];

				fwrite(&read, sizeof(uint8_t), 1, outFileP);
			}
			else {
				currentPos = dictionary[(uint64_t)index8];

				aux = currentPos->bytes;

				if (!feof(inFileP)) {
					aux.emplace_back(read);
					currentPos->childs[read] = new Tree(currentPos, aux, counter);
					if (dictionary.size() < MAX) {
						dictionary[counter++] = currentPos->childs[read];
					}
				}

				for (int i = 0; i < aux.size(); i++) {
					fwrite(&aux[i], sizeof(uint8_t), 1, outFileP);
				}
			}
		}
        else if (dictionary.size() >= 256 && dictionary.size() < 65536) {
			fread(&index16, sizeof(uint16_t), 1, inFileP);
			fread(&read, sizeof(uint8_t), 1, inFileP);

			//First time appearing
			if (index16 == 0) {
				aux.emplace_back(read);

				currentPos->childs[read] = new Tree(currentPos, aux, counter);

				if(dictionary.size() < MAX)
					dictionary[counter++] = currentPos->childs[read];
				else {
					while (!dictionary[counter]->IsLeaf()) {
						counter++;
						if (counter%MAX == 0) {
							counter = 1;
						}
					}

					//Remove child and set new index
					Tree* parent = dictionary[counter]->parent;
					parent->RemoveChild(dictionary[counter]->bytes.back());
					//cout << counter << " ";
					currentPos->childs[read]->index = counter;
					dictionary[counter++] = currentPos->childs[read];
				}

				fwrite(&read, sizeof(uint8_t), 1, outFileP);
			}
			else {
				currentPos = dictionary[(uint64_t)index16];

				aux = currentPos->bytes;

				if (!feof(inFileP)) {
					aux.emplace_back(read);
					currentPos->childs[read] = new Tree(currentPos, aux, counter);

					if (dictionary.size() < MAX) {
						dictionary[counter++] = currentPos->childs[read];
					}
					else {
						while (!dictionary[counter]->IsLeaf()) {
							counter++;
							if (counter%MAX == 0) {
								counter = 1;
							}
						}
						//Remove child and set new index
						Tree* parent = dictionary[counter]->parent;
						parent->RemoveChild(dictionary[counter]->bytes.back());

						currentPos->childs[read]->index = counter;
						dictionary[counter++] = currentPos->childs[read];
					}
				}
				for (int i = 0; i < aux.size(); i++) {
					fwrite(&aux[i], sizeof(uint8_t), 1, outFileP);
				}
			}
		}

		if (counter%MAX == 0) {
			counter = 1;
		}
	}

	fclose(inFileP);
	fclose(outFileP);
}

#undef ll
#undef P
#undef UM
#undef V8
#undef MAX