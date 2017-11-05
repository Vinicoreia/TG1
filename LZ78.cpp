#include <fstream>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <iostream>
#include <string>
#include <utility>
#include <map>
#include <ctime>

#define ll long long
#define P(a,b) pair<a,b>
#define UM(a,b) unordered_map<a, b>
#define V8 vector<uint8_t>
#define MAX 65535

using namespace std;

class LZ78 {
public:
	static void Encode(string filename, string outputfile);
	static void Decode(string filename, string outputfile);

private:
	//Writes bytes from an entry recursively, this function is called to save the first call.
	static void WriteFromDictionary(UM(ll, P(ll, uint8_t))* dictionary, uint8_t read, ll index, FILE* outFileP, V8* toMap, map<V8, ll>* memory);
	//Called when the read byte is the last, and called from WriteFromDictionaryFunction.
	static void WriteFromDictionaryRecursion(UM(ll, P(ll, uint8_t))* dictionary, uint8_t read, ll index, FILE* outFileP, V8* toMap, map<V8, ll>* memory);
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
	unordered_map<long long, pair<long long, uint8_t>> dictionary;
	map<vector<uint8_t>, long long> readMemory;
	uint8_t read;
	vector<uint8_t> aux;

	FILE *inFileP, *outFileP;

	int load = 0;

	long long counter = 0;

	long long indexSize = 1;
	long long index = 0;
	uint8_t index8 = 0;
	uint16_t index16 = 0;
	uint32_t index32 = 0;
	char laod[] = { '|', '\\', '/', '|' };

	//dictionary[0] = make_pair(0, 0);

	inFileP = fopen(filename.c_str(), "rb");
	outFileP = fopen(outputfile.c_str(), "wb");

	std::cout << "Writing ";
	while (!feof(inFileP)) {
		std::cout << laod[load++] << "\b";
		load %= 4;

		fread(&read, sizeof(uint8_t), 1, inFileP);

		vector<uint8_t> aux2(aux.begin(), aux.end());
		aux2.emplace_back(read);

		//Checks whether the read bytes are in the dictionary
		if (readMemory.find(aux2) != readMemory.end()) {
			index = readMemory[aux2];
			aux.emplace_back(read);
		}
		else {
			//Create an entry in the map of the string of symbols read
			if (dictionary.size() < MAX)
				readMemory[aux2] = dictionary.size();

			//Starts a new string
			aux.clear();
			aux.emplace_back(read);
			
			//Checks the number of bytes needed to represent the index
			if (dictionary.size() <= 255) {
				index8 = (uint8_t)index;
				fwrite(&index8, sizeof(uint8_t), 1, outFileP);
				if (!feof(inFileP))
					fwrite(&read, sizeof(uint8_t), 1, outFileP);
				counter += 2;
			}
			else if (dictionary.size()> 255 && dictionary.size() <= 65535) {
				index16 = (uint16_t)index;
				fwrite(&index16, sizeof(uint16_t), 1, outFileP);
				if (!feof(inFileP))
					fwrite(&read, sizeof(uint8_t), 1, outFileP);
				counter += 3;
			}
			else if (dictionary.size() > 65535 && dictionary.size() <= (4294967295)) {
				index32 = (uint32_t)index;
				fwrite(&index32, sizeof(uint32_t), 1, outFileP);
				if (!feof(inFileP))
					fwrite(&read, sizeof(uint8_t), 1, outFileP);
				counter += 5;
			}
			else {
				fwrite(&index, sizeof(long long), 1, outFileP);
				if (!feof(inFileP))
					fwrite(&read, sizeof(uint8_t), 1, outFileP);

				counter += 9;
			}

			//If the dictionary size is lower than the max, create a new entry in the dictionary
			if (dictionary.size() < MAX)
				dictionary.emplace(dictionary.size(), make_pair(index, read));

			index = 0;
		}
	}

	fclose(inFileP);
	fclose(outFileP);

	cout << "Wrote " << counter << " Bytes.\n";
}

void LZ78::Decode(string filename, string outputfile) {
	unordered_map<long long, pair<long long, uint8_t>> dictionary;
	map<vector<uint8_t>, long long> readMemory;
	uint8_t read = 0;
	vector<uint8_t> aux;

	int load = 0;

	FILE *inFileP, *outFileP;

	long long counter = 0;

	long long indexSize = 1;
	unsigned long long index = 0;
	uint8_t index8 = 0;
	uint16_t index16 = 0;
	uint32_t index32 = 0;
	char laod[] = { '|', '\\', '/', '|' };

	inFileP = fopen(filename.c_str(), "rb");
	outFileP = fopen(outputfile.c_str(), "wb");

	std::cout << "Writing...\n";

	do {
		aux.clear();
		std::cout << laod[load++] << "\b";
		//cout << dictionary.size()<< "\r";
		load %= 4;
		index = 0;
		index8 = 0;
		index16 = 0;
		index32 = 0;
		read = 0;

		//Check how many bytes are needed for the index, based on the dictionary size.
		if (dictionary.size() <= 255) {
			fread(&index8, sizeof(uint8_t), 1, inFileP);
			fread(&read, sizeof(uint8_t), 1, inFileP);
			if (!feof(inFileP)) {
				WriteFromDictionary(&dictionary, read, index8, outFileP, &aux, &readMemory);
			}
			else {
				WriteFromDictionaryRecursion(&dictionary, dictionary[index8].second, dictionary[index8].first, outFileP, &aux, &readMemory);
			}
		}
		else if (dictionary.size() > 255 && dictionary.size() <= 65535) {
			fread(&index16, sizeof(uint8_t), 2, inFileP);
			fread(&read, sizeof(uint8_t), 1, inFileP);
			if (!feof(inFileP)) {
				WriteFromDictionary(&dictionary, read, index16, outFileP, &aux, &readMemory);
			}
			else {
				WriteFromDictionaryRecursion(&dictionary, dictionary[index16].second, dictionary[index16].first, outFileP, &aux, &readMemory);
			}
		}
		else if (dictionary.size() > 65535 && dictionary.size() <= (4294967295)) {
			fread(&index32, sizeof(uint8_t), 4, inFileP);
			fread(&read, sizeof(uint8_t), 1, inFileP);
			if (!feof(inFileP)) {
				WriteFromDictionary(&dictionary, read, index32, outFileP, &aux, &readMemory);
			}
			else {
				WriteFromDictionaryRecursion(&dictionary, dictionary[index32].second, dictionary[index32].first, outFileP, &aux, &readMemory);
			}
		}
		else {
			fread(&index, sizeof(long long), 1, inFileP);
			fread(&read, sizeof(uint8_t), 1, inFileP);
			if (!feof(inFileP)) {
				WriteFromDictionary(&dictionary, read, index, outFileP, &aux, &readMemory);
			}
			else {
				WriteFromDictionaryRecursion(&dictionary, dictionary[index].second, dictionary[index].first, outFileP, &aux, &readMemory);
			}
		}
	} while (!feof(inFileP));

	fclose(inFileP);
	fclose(outFileP);
}

void LZ78::WriteFromDictionary(UM(ll, P(ll, uint8_t))* dictionary, uint8_t read, ll index, FILE* outFileP, V8* toMap, map<V8, ll>* memory) {
	if (index == 0) {
		//if the index is 0, the this is the value that will be written in the file
		fwrite(&read, sizeof(uint8_t), 1, outFileP);
		toMap->emplace_back(read);
		if (dictionary->size() < MAX) {
			memory->emplace((*toMap), dictionary->size());
			dictionary->emplace(dictionary->size(), make_pair(index, read));
		}
	}
	else {
		//Call the recursive function to write the other indeces
		WriteFromDictionaryRecursion(dictionary, dictionary->at(index).second, dictionary->at(index).first, outFileP, toMap, memory);
		toMap->emplace_back(read);
		//Then create a dictionary entry for this value, if the size is not greater than the MAX
		if (dictionary->size() < MAX) {
			memory->emplace((*toMap), dictionary->size());
			dictionary->emplace(dictionary->size(), make_pair(index, read));
		}
		fwrite(&read, sizeof(uint8_t), 1, outFileP);
	}
}

void LZ78::WriteFromDictionaryRecursion(UM(ll, P(ll, uint8_t))* dictionary, uint8_t read, ll index, FILE* outFileP, V8* toMap, map<V8, ll>* memory) {
	if (index == 0) {
		//Last element
		fwrite(&read, sizeof(uint8_t), 1, outFileP);
		toMap->emplace_back(read);
	}

	else {
		//Calls itself for writting the previous element
		WriteFromDictionaryRecursion(dictionary, dictionary->at(index).second, dictionary->at(index).first, outFileP, toMap, memory);
		toMap->emplace_back(read);
		fwrite(&read, sizeof(uint8_t), 1, outFileP);
	}
}

#undef ll
#undef P
#undef UM
#undef V8
#undef MAX