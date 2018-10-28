
#include <string>
#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <cstring>

#include "GGZ.h"

#include "BWT.h"
#include "MTF.h"
#include "RunLength.h"
#include "Arithmetic.h"

#define BLOCKSIZE 1048576	/*1024 | 2048 | 4096 | 8192 | 16384 | 32768 | 65536 
						| 131072 | 262144| 1048576
	
	
//*/

using namespace std;

int GGZEncode(fstream* file1, fstream* file2) {
	fstream* input = file1;
	fstream* output = file2;

	vector<uint8_t> toWrite;
	vector<uint8_t> readvector;
	
	long long blockn = 1;
	
	Arithmetic art;

	while (input->peek() != EOF) {
		uint8_t read;
		input->read((char*)&read, 1);

		readvector.emplace_back(read);

		if (readvector.size() == BLOCKSIZE) { //Read blocksize bytes
			Data data(readvector.size());

			for (int i = 0; i < readvector.size(); i++) {
				data.data[i] = readvector[i];
			}
			//Apply BWT

			long long index = BWT::Transform(&data);
			//Apply MTF
			MTF::Encode(&data);

			//Aply RLE
			readvector = vector<uint8_t>(data.size);
			memcpy(readvector.data(), data.data, data.size);

			//Write out
			long long last = toWrite.size();
			toWrite.resize(toWrite.size() + sizeof(long long));
			memcpy(toWrite.data() + last, &index, sizeof(long long));
			toWrite.insert(toWrite.end(), readvector.begin(), readvector.end());
			
			
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

		//Aply RLE
		readvector.resize(data.size);
		memcpy(readvector.data(), data.data, data.size);

		//Write out
		long long last = toWrite.size();

		toWrite.resize(toWrite.size() + sizeof(long long));
		memcpy(toWrite.data() + last, &index, sizeof(long long));
		toWrite.insert(toWrite.end(), readvector.begin(), readvector.end());

		readvector.clear();
	}

	art.AdaptiveEncode(output, &toWrite);

	input->close();
	output->close();

	return 0;
}

int GGZDecode(fstream* file1, fstream* file2) {
	fstream *input = file1, *output = file2;

	vector<uint8_t> readvector;
	vector<uint8_t> data;

	int counter = 0;
	
	Arithmetic art;
	
	art.AdaptiveDecode(input, &data);

	for (int i = 0; i < data.size();) {
		long long index;
		//Read index for BWT reverse and blocksize

		memcpy(&index, data.data() + i, sizeof(long long));
		i+= sizeof(long long) ;
		
		//Read bytes in MTF form
		for (int j = 0; (j < BLOCKSIZE) && (i < data.size()); j++, i++) {
			readvector.emplace_back(data[i]);
		}

		if (readvector.size() > 0) {

			Data aux(readvector.size());
			memcpy(aux.data, readvector.data(), readvector.size());

			MTF::Decode(&aux);
			BWT::Reverse(&aux, index);

			readvector.resize(aux.size);
			memcpy(readvector.data(), aux.data, aux.size);

			output->write((char*)readvector.data(), readvector.size());
		}
		readvector.clear();
	}

	input->close();
	output->close();

	return 0;
}

