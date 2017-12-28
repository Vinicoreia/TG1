#include <bits/stdc++.h>
using namespace std;
/*Simple implementation of the huffman compression algorithm*/
string readFromFile(ifstream &fileIn);
void getFileSize(ifstream &file);
int filesize;

int main(int argc, char *argv[])
{
    ifstream file("LZ77.cpp", ios::in | ios::binary | ios::ate);
    getFileSize(file);
    readFromFile(file);
    std::cout<<filesize;
    return 0;
}

string readFromFile(ifstream &fileIn)
{
    return static_cast<stringstream const &>(stringstream() << fileIn.rdbuf()).str();
}
void getFileSize(ifstream &file)
{
    filesize = file.tellg();
    file.seekg(0, ios::beg);
    file.clear();
}

