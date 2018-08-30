#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

#include "Arithmetic.h"


int main(int argc, char** argv) {
	string file1(""), file2(""), mode, type;
	int order;

	if (argc == 1 || argc < 5) {
		cout << "-e <arithmetic adaptative ppm> <input file name> <output file name>]\n";
		cout << "-d <arithmetic adaptative ppm> <input file name> <output file name>]\n";
		//cout << "Suggested blocksizes: 1024 | 2048 | 4096 | 8192 | 16384 | 32768 | 65536 | 131072 | 262144\n";
		//return 0;
		cout << "Select mode: ";
		cin >> mode;
		cout << "Select type: ";
		cin >> type;
		cout << "Input file: ";
		cin >> file1;
		cout << "Output file: ";
		cin >> file2;
		//cout << "Block size: ";
		//cin >> BLOCKSIZE;
	}
	else {
		mode.assign(argv[1]);

		if (mode != "-e" && mode != "-d") {
			cout << "Invalid input!\n";
			return -1;
		}

		type.assign(argv[2]);

		if (file1 == "")
			file1.assign(argv[3]);

		if (file2 == "")
			file2.assign(argv[4]);
	}

	Arithmetic art;
	clock_t time;

	fstream infile;
	fstream outfile;

	if (mode == "-e") {
		infile.open(file1, fstream::in | fstream::binary);
		outfile.open(file2, fstream::out | fstream::binary);

		vector<uint8_t> data;

		if (!infile.is_open() | !outfile.is_open()) {
			cout << "Could not open file!\n";
			return -1;
		}

		while (1) {
			uint8_t read;

			infile.read((char*)&read, 1);
			if (infile.eof())
				break;

			data.emplace_back(read);
		}
		time = clock();
		if (type == "arithmetic") {
			art.StaticEncode(&outfile, &data);
		}
		else if (type == "adaptative") {
			art.AdaptiveEncode(&outfile, &data);
		}
		else if (type == "ppm") {
			art.PPMEncode(&outfile, &data);
		}
		cout << "Time elapsed in Encoding: " << ((double)(clock() - time) / CLOCKS_PER_SEC) << endl;
	}
	else if (mode == "-d") {
		infile.open(file1, fstream::in | fstream::binary);
		outfile.open(file2, fstream::out | fstream::binary);

		vector<uint8_t> data;

		if (!infile.is_open() | !outfile.is_open()) {
			cout << "Could not open file!\n";
			return -1;
		}

		time = clock();
		if (type == "arithmetic") {
			art.StaticDecode(&infile, &data);
		}
		else if (type == "adaptative") {
			art.AdaptiveDecode(&infile, &data);
		}
		else if (type == "ppm") {
			art.PPMDecode(&infile, &data);
		}
		cout << "Time elapsed in decoding: " << ((double)(clock() - time) / CLOCKS_PER_SEC) << endl;

		outfile.write((char*)data.data(), data.size() * sizeof(uint8_t));
		infile.close();
		outfile.close();

	}

	return 0;
}

