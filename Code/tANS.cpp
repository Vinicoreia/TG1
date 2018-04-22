/*
	This implementation of ANS encoder and decoder is based on the code made by Andrew Polar in
	http://www.ezcodesample.com/abs/ANSCoder.txt.

*/

#include <fstream>
#include <iostream>
#include <vector>
#include <utility>
#include <queue>
#include <string>

#define ALPHABETSIZE 256
#define PRECISION 4

#define PROBABILITYPRECISION (sizeof(char) * 8 + PRECISION) 

//this will make 4096 states
#define NOFSTATES ALPHABETSIZE<<PRECISION

//The max state will be 4095
#define MAXSTATE ((NOFSTATES) << 1) - 1

using namespace std;

//Gets the rearranged order of the number system.
void GetOrder(vector<int> &frequencies, vector<int> &order) {
	int min = 0, max = 0;
	//find the minimum and maximum frequencies
	for (int i = 0; i < ALPHABETSIZE; i++) {
		if (min > frequencies[i])
			min = frequencies[i];
		if (max < frequencies[i])
			max = frequencies[i];
	}

	//if frequencies are all equal, so we use the normal organization.
	if (min == max) {
		for (int i = 0; i < ALPHABETSIZE; i++) {
			order[i] = i;
		}
		return;
	}

	//makes a priority queue so that the smallest value is first
	priority_queue<pair<int, int>> freq;
	for (int i = 0; i < ALPHABETSIZE; i++) {
		freq.push(make_pair(-frequencies[i], i));
	}

	//writes the 
	for (int i = 0; i < ALPHABETSIZE; i++) {
		order[i] = freq.top().second;
		freq.pop();
	}

	return;
}

//Makes the new number system. With the states and their 
void MakeChart(vector<int> &chart, vector<int>& frequencies, vector<int> &sizes) {
	for (int i = 0; i < ALPHABETSIZE; i++) {
		sizes[i] = 1; // table starts from 1
	}

	vector<int> order(ALPHABETSIZE, 0);
	GetOrder(frequencies, order);

	int denominator = 1 << PROBABILITYPRECISION;

	//Initialize chart with a not achievable state as initial state
	for (int i = 0; i <= ((NOFSTATES) << 1) - 1; i++) {
		chart[i] = denominator;
	}

	//Redistribute the alphabet
	int state;
	for (int i = 1; i <= MAXSTATE; i++) {
		for (int j = 0; j < ALPHABETSIZE; j++) {
			state = (i << PROBABILITYPRECISION) / frequencies[j];
			if (state < 2)
				state = 2; //State 1 is where the states starts.

			if (state <= MAXSTATE) {
				if (chart[state] == denominator) { //value at the chart not initialized
					chart[state] = order[j];	
					++sizes[order[j]];	//Increments the counter of appearance
				}
				else {	//this chart position is taken
					for (;;) {
						//find a new chart position
						++state;
						if (state > MAXSTATE)
							break;
						if (chart[state] == denominator) {	
							chart[state] = order[j];
							++sizes[order[j]];
							break;
						}
					}
				}
			}
		}
	}
}

void MakeEncodingTable(vector<vector<int>> &encodingTable, vector<int> &chart, vector<int> &sizes) {
	int denominator = 1 << PROBABILITYPRECISION;
	vector<int> order(ALPHABETSIZE);

	//states start at 1
	for (int i = 0; i < ALPHABETSIZE; i++) {
		order[i] = 1;
	}

	//make the encoding table
	for (int i = 2; i < MAXSTATE; i++) {
		if (chart[i] != denominator)
			encodingTable[chart[i]][order[chart[i]]++] = i; //gives the appearence of the values at chart
	}
}


vector<unsigned char> Encode(vector<unsigned char> data) {
	vector<int> frequencies(ALPHABETSIZE, 0);

	//Get data's distribution
	for (int i = 0; i < data.size(); i++) {
		frequencies[data[i]]++;
	}

	//Get the normalize factor to make the data distribution fit in the states
	double normalize = (double)(NOFSTATES) / (double)data.size();

	//Normalize the data
	for(int i = 0; i < ALPHABETSIZE; i++){
		frequencies[i] = (int)(frequencies[i] * normalize);
	}

	//Check total sum of the distribution
	int total = 0;
	for (int i = 0; i < ALPHABETSIZE; i++) {
		total += frequencies[i];
	}

	//Get the error of the distribution
	int e = (NOFSTATES)-total;

	//Find the max position and add the error so the total is the number of states
	int pos = 0;
	int max = 0;
	for (int i = 0; i < ALPHABETSIZE; i++) {
		if (max < frequencies[i]) {
			max = frequencies[i];
			pos = i;
		}
	}
	frequencies[pos] += e;

	//Frequencies can't be 0
	for (int i = 0; i < ALPHABETSIZE; i++) {
		if (frequencies[i] == 0) {
			frequencies[i] = 1;
			frequencies[pos]--;
		}
	}

	//allocate space to write probabilities
	vector<unsigned char> output((frequencies.size() + 1) * sizeof(frequencies[0]));
	memcpy(output.data(), frequencies.data(), frequencies.size() * sizeof(frequencies[0]));
	int dataSize = data.size();
	memcpy(output.data() + frequencies.size() * sizeof(frequencies[0]), &dataSize, sizeof(int));

	//allocate 2*L as in the Jarek Duda's paper
	vector<int> chart((NOFSTATES)<<1, 0), sizes(ALPHABETSIZE, 0);
	MakeChart(chart, frequencies, sizes);

	//Make the state encoding table
	vector<vector<int>>encodingTable(ALPHABETSIZE);
	for (int i = 0; i < ALPHABETSIZE; i++) {
		encodingTable[i].resize(sizes[i], 0);
	}
	
	MakeEncodingTable(encodingTable, chart, sizes);

	//Start encoding
	int state = MAXSTATE;
	unsigned char byte = 0;
	unsigned char currentBit = 0;

	int mask = 1 << (PROBABILITYPRECISION);
	for (int i = 0; i < data.size(); i++) {
		while (state > sizes[data[i]] - 1) {	//write bits to renormalize the state
			++currentBit;
			byte |= state & 1;
			if (currentBit == 8) {
				output.emplace_back(byte);
				currentBit = 0;
				byte = 0;
			}
			else {
				byte <<= 1;
			}
			state >>= 1;
		}
		state = encodingTable[data[i]][state]; //Get the new state based on the current byte read and current state.

		if (state < mask) {
			cout << "Error in symbol "<< i << " "<< data[i]<<" "<< state <<" \n";
			system("PAUSE");
			break;
		}
	}

	output.resize(output.size() + sizeof(int));

	memcpy(output.data() + output.size() - sizeof(int), &state, sizeof(int));

	return output;
}


int main(int argc, char** argv) {
	string filename, outfile;
	system("DIR");
	cin >> filename >>outfile;
	
	vector<unsigned char> data;

	fstream file(filename, fstream::in | fstream::binary);
	if (file.is_open()) {
		unsigned char aux;
		while (!file.eof()) {
			file.read((char*)&aux, 1);
			data.emplace_back(aux);
		}

		file.close();

		vector<unsigned char> encoded = Encode(data);

		fstream out(outfile, fstream::out | fstream::binary);
		out.write((char*)encoded.data(), encoded.size());
		out.close();

		return 0;
	}
	return -1;
}