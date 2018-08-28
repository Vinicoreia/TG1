#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

#include "Arithmetic.h"


int main(int argc, char** argv) {
	Arithmetic art;
	clock_t time;

	std::string filename, output;

	std::cin >> filename >> output;

	fstream infile(filename, fstream::in | fstream::binary);
	fstream outfile(output, fstream::out | fstream::binary);

	vector<uint8_t> data;

	while (1) {
		uint8_t read;

		infile.read((char*)&read, 1);
		if (infile.eof())
			break;

		data.emplace_back(read);
	}

	time = clock();
	art.PPMEncode(&outfile, &data);
	cout << "Time elapsed in Encoding: " << ((double)(clock() - time) / CLOCKS_PER_SEC) << endl;
	infile.close();
	outfile.close();

	cin >> filename >> output;

	infile.open(filename, fstream::in | fstream::binary);
	outfile.open(output, fstream::out | fstream::binary);

	data.clear();

	art.PPMDecode(&infile, &data);

	outfile.write((char*)data.data(), data.size() * sizeof(uint8_t));

	infile.close();
	outfile.close();
	
	system("PAUSE");
	return 0;
}

