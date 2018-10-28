#include <iostream>
#include <fstream>
#include <string>

#include "PPM.h"
#include "GGZ.h"

using namespace std;

int main (int argc, char** argv){
	string filename1, filename2;
	fstream infile, outfile;
	string op;
	
	if(argc == 1 || argc > 4){
		cout << "-e <input file name> <output file name>\n";
		cout << "-d <input file name> <output file name>\n";
		cout << "Select mode: ";
		cin>>op;
		cout << "Input file: "; 
		cin>>filename1;
		cout << "Output file: ";
		cin>>filename2;
	}
	else{
		op.assign(argv[1]);
		filename1.assign(argv[2]);
		filename2.assign(argv[3]);
	}
	
	
	
	if(op == "-e"){
		size_t filesize;
		
		infile.open(filename1, fstream::in | fstream::ate | fstream::binary);
		outfile.open(filename2, fstream::out | fstream::binary);
	
		if (!infile.is_open() || !outfile.is_open()) {
			cout << "Could not open file!\n";
			return -1;
		}
		
		filesize = infile.tellg();
		infile.seekp(ios_base::beg);
		if(filesize < 32100){
			//Use GGZnoRLE
			outfile.write("GGZG", sizeof(char)*4);
			GGZEncode(&infile, &outfile);
		}else{
			outfile.write("GGZP", sizeof(char)*4);
			PPM ppm;
			ppm.PPMEncode(&outfile, &infile);
		}
	} 
	else if(op == "-d"){
		infile.open(filename1, fstream::in | fstream::binary);
		outfile.open(filename2, fstream::out | fstream::binary);
		if (!infile.is_open() || !outfile.is_open()) {
			cout << "Could not open file!\n";
			return -1;
		}
		
		char header[4];

		infile.read(header, 4);
		
		string head(header, 4);
		cout<<head<<endl;
		
		if(head == "GGZG"){
			GGZDecode(&infile, &outfile);
		}
		else if(head == "GGZP"){
			PPM ppm;
			ppm.PPMDecode(&infile, &outfile);
		}else{
			cout<<"Unrecognized header.\n";	
		}
		
		infile.close();
		outfile.close();
	}
}