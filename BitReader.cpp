#include <cstdio>
#include <iostream>
#include <utility>
#include <vector>

using namespace std;

class BitReader{
	uint64_t buffer;
	int numberOfBits;
	FILE* input;
	
public:
	BitReader(FILE* file){
		input = file;
		buffer = 0;
		numberOfBits = 0;
	}

	uint64_t GetBit(){
		uint8_t mask = 0x1, value;
		uint8_t read;
		
		//If there are no bits, read a byte from input.
		if(numberOfBits == 0){
			fread(&read, sizeof(uint8_t), 1, input);
			if(feof(input))
				return -1;
			buffer = (uint64_t)read;
			numberOfBits = 8;
		}
		
		numberOfBits--;
		//Read the nth bit.
		return (buffer >> numberOfBits)&1;
	}
	
	uint8_t Read8(){
		uint8_t value = 0;
		for(int i = 0; i < 8; i++){
			value = (value<<1) | GetBit();
		}
		return value;
	}
	
	uint64_t Read(int bits){
		uint64_t value = 0;
		for(int i = 0; i < bits; i++){
			value = (value<<1) | GetBit();
		}
		return value;
	}
};

class BitWriter{
	int bitPosition;
	uint8_t toWrite;
	FILE* output;
	
public:
	BitWriter(FILE* filename){
		bitPosition = 0;
		toWrite = 0;
		output = filename;
	}
	
	void WriteBit(uint8_t bit){
		toWrite = (toWrite<<1)|bit;
		bitPosition++;
		
		//Writes if byte is full.
		if(bitPosition == 8){
			fwrite(&toWrite, sizeof(uint8_t), 1, output);
			toWrite = 0;
			bitPosition = 0;
		}
	}
	
	void Write(uint8_t value){
		for(int i = 7; i >= 0; i--){
			uint8_t mask = 1<<i;
			if(value&mask){
				WriteBit(1);
			}else{
				WriteBit(0);
			}
		}
	}
	
	void Write(uint16_t value){
		for(int i = 15; i >= 0; i--){
			uint16_t mask = 1<<i;
			if(value&mask){
				WriteBit(1);
			}else{
				WriteBit(0);
			}
		}
	}
	
	void Write(uint32_t value){
		for(int i = 31; i >= 0; i--){
			uint32_t mask = 1<<i;
			if(value&mask){
				WriteBit(1);
			}else{
				WriteBit(0);
			}
		}
	}
	
	void Write(uint64_t value){
		for(int i = 63; i >= 0; i--){
			uint64_t mask = 1<<i;
			if(value&mask){
				WriteBit(1);
			}else{
				WriteBit(0);
			}
		}
	}
	
	void Write(uint64_t value, uint8_t nOfBits){
		for(int i = nOfBits - 1; i >= 0; i--){
			uint64_t mask = 1<<i;
			if(value&mask){
				WriteBit(1);
			}else{
				WriteBit(0);
			}
		}
	}
	
	void End(){
		if(bitPosition > 0){
			toWrite <<= 8-bitPosition;
			fwrite(&toWrite, sizeof(uint8_t), 1, output);
		}
	}
};

int main(){
	FILE* input = fopen("testfile.bin", "wb");
	
	BitWriter bw(input);
	
	bw.WriteBit(1);
	bw.WriteBit(1);
	bw.WriteBit(1);
	bw.WriteBit(1);
	bw.WriteBit(1);
	bw.WriteBit(1);
	bw.WriteBit(1);
	bw.WriteBit(1);
	
	bw.Write((uint8_t)0xae);
	bw.Write((uint16_t)0xbebe);
	bw.Write((uint32_t)0xcececece);
	bw.Write((uint64_t)0xdededededededede);
	bw.Write((uint64_t)0xff, 8);
	
	bw.WriteBit(1);
	bw.End();
	
	fclose(input);
	
	input = fopen("testfile.bin", "rb");
	
	BitReader br(input);
	
	uint64_t aux;
	cout<<(int)br.Read(1)<<endl;
	cout<<(int)br.Read(2)<<endl;
	cout<<(int)br.Read(5)<<endl;
	cout<<(int)br.Read8()<<endl;
	cout<<(int)br.Read(16)<<endl;
	cout<<(int)br.Read(32)<<endl;
	cout<<(long long)br.Read(64)<<endl;
	cout<<(int)br.Read(8)<<endl;
	cout<<(int)br.Read(8)<<endl;
	cout<<(int)br.Read(1)<<endl;
	
	fclose(input);
	return 0;
}
