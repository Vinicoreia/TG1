#include "RLE.h"

#include <string>
#include <fstream>
#include <vector>
#include <cstdint>
#include <iostream>

using namespace std;

int CheckLength(vector<uint8_t>* data, uint8_t value, int index){
    int i = 1;
    while((index < data->size()) && (data->at(index) == value)){
        index++;
        i++;
    }
    return i;
}

void RLE::Encode(string file1, string file2){
    fstream input, output;
    vector<uint8_t> data;

    input.open(file1, fstream::in | fstream::ate | fstream::binary);
    output.open(file2, fstream::out | fstream::binary);

    int filesize = input.tellg();

    input.close();
    input.open(file1, fstream::in | fstream::binary);

    data.resize(filesize);

    if(data.empty()){
        output.close();
        input.close();
        exit(0);
    }

    input.read((char*)data.data(), filesize*sizeof(uint8_t));

    uint8_t previous = data[0] - 1;
    for(int i = 0; i < data.size();){
        if(previous == data[i]){
            int length = CheckLength(&data, previous, i);
            i += length-1;
			bool first = true;
            while(length > 255){
                unsigned char max = 255;
                output.write((char*)&previous, sizeof(uint8_t));
				if (!first) {
					output.write((char*)&previous, sizeof(uint8_t));
				}
				else {
					first = false;
				}
                output.write((char*)&max, sizeof(unsigned char));

                length -= 255;
            }
			if (!first) {
				output.write((char*)&previous, sizeof(uint8_t));
			}
            output.write((char*)&previous, sizeof(uint8_t));
            unsigned char resto = (unsigned char)length;

            output.write((char*)&resto, sizeof(unsigned char));
        }else{
			previous = data[i++];
            output.write((char*)&previous, sizeof(uint8_t));
        }
    }

    input.close();
    output.close();
}

void RLE::Decode(string file1, string file2) {
	fstream input, output;

	input.open(file1, fstream::in | fstream::binary);
	output.open(file2, fstream::out | fstream::binary);

	uint8_t previous;
	input.read((char*)&previous, sizeof(uint8_t));

	while (input.peek() != EOF) {
		uint8_t read;

		input.read((char*)&read, sizeof(uint8_t));
		if (read == previous) {
			uint8_t length;

			input.read((char*)&length, sizeof(uint8_t));

			for (int i = 0; i < length; i++) {
				output.write((char*)&read, sizeof(uint8_t));
			}

			input.read((char*)&previous, sizeof(uint8_t));
		}
		else {
			output.write((char*)&previous, sizeof(uint8_t));
			previous = read;
		}
	}


}

int main(){
    string file1, file2;
    cin>>file1>>file2;

    RLE::Encode(file1, file2);

	cin >> file1 >> file2;

	RLE::Decode(file1, file2);

    return 0;
}