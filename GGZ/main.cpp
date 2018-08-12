
#include <string>
#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <cstring>

#include "BWT.h"
#include "MTF.h"

#include "ArithmeticEncoding.h"

//#define BLOCKSIZE 1024	/*1024 | 2048 | 4096 | 8192 | 16384 | 32768 | 65536 | 1048576
	
	
//	2 - GATHER DATA
//	3 - FINISH
//*/

long long BLOCKSIZE;
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

	vector<uint8_t> toWrite;
	vector<uint8_t> readvector;
	
	long long blockn = 1;

	while (input.peek() != EOF) {
		uint8_t read;
		input.read((char*)&read, 1);

		readvector.emplace_back(read);

		if (readvector.size() == BLOCKSIZE) { //Read blocksize bytes
			Data data(BLOCKSIZE);

			memcpy(data.data, readvector.data(), readvector.size());
			
			//Apply BWT
			long long index = BWT::Transform(&data);
			//Apply MTF
			MTF::Encode(&data);
			//Save to be written later
			long long last = toWrite.size();
			toWrite.resize(toWrite.size() + sizeof(long long) + data.size*sizeof(uint8_t));
			memcpy(toWrite.data() + last, &index, sizeof(long long));
			memcpy(toWrite.data() + last + sizeof(long long), data.data, data.size * sizeof(uint8_t));
			//output.write((char*)&index, 2);
			//output.write((char*)data.data, data.size * sizeof(uint8_t));

			readvector.clear();
			cout<<"\rEnd of Block "<<blockn<<"! Processed: "<<blockn*BLOCKSIZE<<"bytes";
			blockn++;
		}
	}

	if (readvector.size() > 0) { //Read less than blocksize bytes and found eof
		Data data(readvector.size());

		for (int i = 0; i < readvector.size(); i++) {
			data.data[i] = readvector[i];
		}
		//Apply BWT
		long long index = BWT::Transform(&data);
		//Apply MTF
		MTF::Encode(&data);
		//Write out
		long long last = toWrite.size();
		toWrite.resize(toWrite.size() + sizeof(long long) + data.size * sizeof(uint8_t));
		memcpy(toWrite.data() + last, &index, sizeof(long long));
		memcpy(toWrite.data() + last + sizeof(long long), data.data, data.size * sizeof(uint8_t));
		//output.write((char*)&index, 2);
		//output.write((char*)data.data, readvector.size());

		readvector.clear();
	}

	//output.write((char*)toWrite.data(), toWrite.size());
	ArithmeticEncoder::Encode(&output, &toWrite);


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
	vector<uint8_t> data;

	int counter = 0;

	ArithmeticEncoder::Decode(&input, &data);

	for (int i = 0; i < data.size();) {
		long long index;

		//Read index for BWT reverse
		memcpy(&index, data.data() + i, sizeof(long long));
		//cout<<index<<endl;
		i+= sizeof(long long);
		//index = (data[i++])|((long long)data[i++]<<8)|((long long)data[i++]<<16)|((long long)data[i++]<<24)|((long long)data[i++]<<32)|((long long)data[i++]<<40)|((long long)data[i++]<<48)|((long long)data[i++]<<56);

		//input.read((char*)&index, sizeof(unsigned short));
		//Read bytes in MTF form
		for (int j = 0; (j < BLOCKSIZE) && (i < data.size()); j++, i++) {
			readvector.emplace_back(data[i]);
		}

		if (readvector.size() > 0) {
			//Create the data to be passed to MTF and BWT reverses
			Data aux(readvector.size());
			for (int k = 0; k < readvector.size(); k++) {
				aux.data[k] = readvector[k];
			}

			MTF::Decode(&aux);
			BWT::Reverse(&aux, index);

			output.write((char*)aux.data, aux.size);
		}
		readvector.clear();
	}

	/*
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
	*/
	input.close();
	output.close();

	return 0;
}

int main(int argc, char** argv) {
	string file1(""), file2(""), mode;
	
	if (argc == 1 || argc < 5) {
		cout << "-e <input file name> <output file name> <BLOCKSIZE>\n";
		cout << "-d <input file name> <output file name> <BLOCKSIZE>\n";
		cout << "Suggested blocksizes: 1024 | 2048 | 4096 | 8192 | 16384 | 32768 | 65536 | 131072 | 262144\n";
		return 0;
		cout << "Select mode: ";
		cin >> mode;
		cout << "Input file: ";
		cin >> file1;
		cout << "Output file: ";
		cin >> file2;
		cout << "Block size: ";
		cin >> BLOCKSIZE;
	}else
		mode.assign(argv[1]);

	if (mode != "-e" && mode != "-d") {
		cout << "Invalid input!\n";
		return -1;
	}

	if (file1 == "")
		file1.assign(argv[2]);
	
	if (file2 == "")
		file2.assign(argv[3]);
	
	BLOCKSIZE = atoi(argv[4]);

	if (mode == "-e") {
		Encode(file1, file2);
	}
	else if (mode == "-d") {
		Decode(file1, file2);
	}

	return 0;
}
