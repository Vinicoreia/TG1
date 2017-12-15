//This program calculates the entropy for a given file.
//It is calculated the entropy of a 255 symbol alphabet and a 2^16 symbol alphabet

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <map>
#include <iostream>

int main(int argc, char** argv){
	char filename[100];
	
	uint8_t data;
	int32_t *probabilities;
	
	float entropy = 0;
	int64_t total = 0;
	
	int i;
	
	FILE *file;
	
	std::map<int32_t, int> map;
	
	probabilities = (int32_t*)calloc(256, sizeof(int32_t));
	
	if(argc > 1){
		strcpy(filename, argv[1]);
	}else{
		printf("File name: ");
		
		while(scanf("%s", filename) != 1){
			printf("Repeat: ");
		}
	}
	
	file = fopen(filename, "rb");
	
	while(!feof(file)){
		fread(&data, sizeof(uint8_t), 1, file);
		probabilities[data]++;
		total++;
	}
	
	for(i = 0; i < 256; i++){
		map[probabilities[i]] = i;
		if(probabilities[i] != 0)
			entropy += (((float)probabilities[i])/(float)total) * log((((float)probabilities[i])/(float)total))*-1;
	}
	
	for(auto itr = map.begin(); itr != map.end(); itr++){
		std::cout<<itr->first<<std::endl;
	}
	
	
	printf("Entropy: %f\n", entropy);
	
	return 0;
}