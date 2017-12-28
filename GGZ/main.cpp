#include "LZ78v2.h"

#include <string>
#include <iostream>
#include <fstream>
#include <utility>
#include <vector>

#include "BWT.h"
#include "MTF.h"

#define BLOCKSIZE 16384	/*1024 | 2048 | 4096 | 8192 | 16384 | 32768 | 65536
Memory needed +-		  1MB  | 4MB  | 16MB | 67MB | 268MB | 1GB   | 4GB  */
/*
	2 - GATHER DATA
	3 - FINISH
*/
using namespace std;

int Encode(string file1, string file2) {
	fstream input, output;

	input.open(file1, fstream::in | fstream::binary);
	if (!input.is_open()) {
		cout << "Could not open input file\n";
		return -1;
	}

	output.open(file2, fstream::out | fstream::binary);
	if (!output.is_open()) {
		cout << "Could not open output file\n";
		input.close();
		return -1;
	}

	//Data data(BLOCKSIZE);
	vector<uint8_t> readvector;

	while (input.peek() != EOF) {
		uint8_t read;
		input.read((char*)&read, 1);

		readvector.emplace_back(read);

		if (readvector.size() == BLOCKSIZE) { //Read blocksize bytes
			Data data(BLOCKSIZE);

			memcpy(data.data, readvector.data(), readvector.size());
			
			//Apply BWT
			unsigned short index = BWT::Transform(&data);
			//Apply MTF
			MTF::Encode(&data);
			//Write out
			output.write((char*)&index, 2);
			output.write((char*)data.data, data.size * sizeof(uint8_t));

			readvector.clear();
		}
	}

	if (readvector.size() > 0) { //Read lesser than blocksize bytes and found eof
		Data data(readvector.size());

		for (int i = 0; i < readvector.size(); i++) {
			data.data[i] = readvector[i];
		}
		//Apply BWT
		unsigned short index = BWT::Transform(&data);
		//Apply MTF
		MTF::Encode(&data);
		//Write out
		output.write((char*)&index, 2);
		output.write((char*)data.data, readvector.size());

		readvector.clear();
	}

	input.close();
	output.close();

	return 0;
}

int Decode(string file1, string file2) {
	fstream input, output;

	input.open(file1, fstream::in | fstream::binary);
	if (!input.is_open()) {
		cout << "Could not open input file\n";
		return -1;
	}

	output.open(file2, fstream::out | fstream::binary);
	if (!output.is_open()) {
		cout << "Could not open output file\n";
		input.close();
		return -1;
	}

	vector<uint8_t> readvector;

	int counter = 0;

	while (input.peek() != EOF) {
		unsigned short index;
		//Read index for BWT reverse
		input.read((char*)&index, sizeof(unsigned short));
		//Read bytes in MTF form
		for (int i = 0; (i < BLOCKSIZE) && (input.peek() != EOF); i++) {
			uint8_t read;

			input.read((char*)&read, sizeof(uint8_t));
			readvector.emplace_back(read);
		}

		if (readvector.size() > 0) {
			//Create the data to be passed to MTF and BWT reverses
			Data aux(readvector.size());
			for (int i = 0; i < readvector.size(); i++) {
				aux.data[i] = readvector[i];
			}

			MTF::Decode(&aux);
			BWT::Reverse(&aux, index);

			output.write((char*)aux.data, aux.size);
		}
		readvector.clear();
	}

	input.close();
	output.close();

	return 0;
}

int main() {
	string file1, file2;

	cin >> file1 >> file2;

	Encode(file1, file2);

	cin >> file1 >> file2;
	Decode(file1, file2);

	return 0;
}
