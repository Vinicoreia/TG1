#include<bits/stdc++.h> 
#define DICTSIZE 32767 
#define LOOKAHEADSIZE 255 
#define WINDOWSIZE LOOKAHEADSIZE + DICTSIZE 
#define DICTBITS 15 
#define LOOKBITS 8 

using namespace std;

typedef struct code
{
    size_t offset;
    string foundString;
    char nextChar;
} code;

size_t filesize;
size_t windowPointer = 0;    /*points to end of window*/
size_t dictPointer = 0;      /*points to beg of dict*/
size_t lookaheadPointer = 0; /*points to beg of lookahead*/
string window;
string fileBuffer;
string lookahead;
string dict;
string binaryBuffer;
string bitString;
string fileNameIn;
string fileNameOut;

string charToBin(char c);
string readFileToBuffer(ifstream &in);
void getFileSize(ifstream &file);
void fillBuffer(fstream &file);
void CompressFile(ifstream &file);
void getNextWindow(size_t matchSize = 0, size_t jump = 0);
code getBiggestSubstring();
void decompressFile(ifstream &file);
void startWindow();

int main(int argc, char *argv[])
{
    string fileMode;
    if (argc == 4)
    {
        fileNameOut = argv[3];
        fileNameIn = argv[2];
        fileMode = argv[1];
        ifstream file(fileNameIn, ios::in | ios::binary | ios::ate);
        if (file.is_open())
        {
            getFileSize(file);
            cout << "Your file has : " << filesize << " bytes" << endl;
            fileBuffer = readFileToBuffer(file);
            if (fileMode == "compress")
            {
                CompressFile(file);
            }
            else if (fileMode == "decompress")
            {
                decompressFile(file);
            }
            else
            {
                cout << "No option named " << fileMode << endl;
            }
            file.close();
            }
    }
    else
    {
        cout << "Wrong format" << endl;
        cout << "The format is <compress|decompress filenameIn filenameOut>" << endl;
    }
    return 0;
}

void CompressFile(ifstream &file)
{
    code elementTuple;
    deque<code> codeTriples;
    int jump;
    int len;
    char next;
    string matchString;
    string maxString;

    codeTriples.push_back({0, fileBuffer.substr(0, 1), fileBuffer[0]});
    startWindow();

    while (!lookahead.empty())
    {
        elementTuple = getBiggestSubstring();
        codeTriples.push_back(elementTuple);
        getNextWindow(elementTuple.foundString.length(), elementTuple.offset);
    }

    fileBuffer.clear();
    while (codeTriples.size() > 0)
    {
        jump = codeTriples[0].offset;
        len = codeTriples[0].foundString.length();
        matchString = codeTriples[0].foundString;
        next = codeTriples[0].nextChar;
        if (jump == 0)
        {
            bitString += "0";
            bitString.append(charToBin(next));
        }
        else if ((len * 9 + 8) < (1 + DICTBITS + LOOKBITS + 8) and jump > 0) /*times 9 because we have to apend 'len' bits of flag*/
        {
            for (int i = 0; i < len; i++)
            {

                bitString += "0"; /*FLAG*/
                bitString.append(charToBin(codeTriples[0].foundString[i]));
            }
            bitString += "0"; /*FLAG*/
            bitString.append(charToBin(next));
        }
        else
        {
            bitString += "1"; /*FLAG*/
            bitString.append(bitset<DICTBITS>(jump).to_string());
            bitString.append(bitset<LOOKBITS>(len).to_string());
            bitString.append(charToBin(next));
        }
        codeTriples.pop_front();
    }
    while (bitString.size() % 8 != 0)
    {
        bitString += "0";
    }
    cout<<"Final filesize after compressing: "<<bitString.size()/8<<" bytes"<<endl;
    ofstream output(fileNameOut, ios::out | ios::binary);
    unsigned long c;
    while (!bitString.empty())
    {
        bitset<8> b(bitString);
        c = b.to_ulong();
        output.write(reinterpret_cast<const char *>(&c), 1);
        bitString.erase(0, 8);
    }
    output.close();
}

void decompressFile(ifstream &file)
{
    while (fileBuffer.size() > 0)
    {
        bitString.append(charToBin(fileBuffer.at(0)));
        fileBuffer.erase(0, 1);
    }
    string bitChar;
    string lookaheadBits;
    string dictBits;
    string outString;
    int jump;
    int len;
    char nextChar;
    while (bitString.size() >= 8)
    {
        if (bitString[0] == '0')
        {
            bitString.erase(0, 1);
            bitChar = bitString.substr(0, 8);
            bitString.erase(0, 8);
            nextChar = static_cast<char>(std::stoi(bitChar, 0, 2));
            
            outString += nextChar;
            dict += nextChar;
            
            windowPointer += 1;
            if (dict.size() > DICTSIZE)
            {
                dict.erase(0, 1);
            }
        }
        else
        {
            bitString.erase(0, 1);
            dictBits = bitString.substr(0, DICTBITS);
            lookaheadBits = bitString.substr(DICTBITS, LOOKBITS);
            bitChar = bitString.substr(DICTBITS + LOOKBITS, 8);
            bitString.erase(0, DICTBITS + LOOKBITS + 8);
            nextChar = static_cast<char>(std::stoi(bitChar, 0, 2));
            jump = stoi(dictBits, 0, 2);
            len = stoi(lookaheadBits, 0, 2);
            for (int i = 0; i < len; i++)
            {
                outString += dict[(dict.size() - jump + i) % dict.size()];
            }
            outString += nextChar;
            dict.append(outString.substr(windowPointer, len + 1));
            windowPointer += len + 1;
            if (dict.size() > DICTSIZE)
            {
                dict.erase(0, len + 1);
            }
        }
    }
    ofstream output(fileNameOut, ios::out | ios::binary);
    output << outString; //WRITE TO FILE
    cout << "Final filesize after decompressing: " << outString.size()<<" bytes"<<endl;

    output.close();
}

string charToBin(char c)
{
    string charBin;
    charBin.clear();
    for (int i = 7; i >= 0; --i)
    {
        (c & (1 << i)) ? charBin += '1' : charBin += '0';
    }
    return charBin;
}

string readFileToBuffer(ifstream &fileIn)
{
    return static_cast<stringstream const &>(stringstream() << fileIn.rdbuf()).str();
}

void getFileSize(ifstream &file)
{
    filesize = file.tellg();
    file.seekg(0, ios::beg);
    file.clear();
}
void startWindow()
{
    lookahead = fileBuffer.substr(1, LOOKAHEADSIZE);
    lookaheadPointer += 1;
    dict = fileBuffer.substr(0, 1);
    windowPointer += lookahead.size() + 1;
}

void getNextWindow(size_t matchSize, size_t jump)
{
    if (jump == 0)
    {
        matchSize = 0;
    }

    matchSize++;
    lookaheadPointer += matchSize;
    windowPointer += matchSize;
    dictPointer += matchSize;

    dict.append(fileBuffer.begin() + lookaheadPointer - matchSize, fileBuffer.begin() + lookaheadPointer);
    lookahead.append(fileBuffer.begin() + windowPointer - matchSize, fileBuffer.begin() + windowPointer);
    lookahead.erase(0, matchSize);

    if (dict.size() > DICTSIZE)
    {
        dict.erase(0, matchSize);
    }
    if(windowPointer >filesize){
        windowPointer = filesize;
        lookahead.resize(windowPointer-lookaheadPointer);
    }
    if (lookaheadPointer >= filesize)
    {
        lookaheadPointer = filesize;
        windowPointer = filesize;
        lookahead.clear();
    }
}

code getBiggestSubstring()
{
    vector<code> substring;
    code sendString;
    char a = lookahead[0];
    size_t pos = dict.find_first_of(a);
    size_t i = 0;
    string strMatch;
    if (pos >= dict.length())
    {
        return {0, lookahead.substr(0, 1), lookahead[0]};
    }
    while (pos != string::npos)
    {
        strMatch.clear();
        i = 0;
        while (dict[(pos + i) % dict.size()] == lookahead[i] and i < lookahead.size())
        {
            strMatch += (lookahead[i]);
            i++;
            a = lookahead[i];
            if (i == lookahead.size()-1)
            {   
                break;
            }
        }
        substring.push_back({dict.size() - pos, strMatch, a});
        a = lookahead[0];
        pos = dict.find_first_of(a, pos + 1);
    }
        reverse(substring.begin(), substring.end());
        int index = 0;
        size_t max = 0;
        for (auto vec : substring)
        {
            if (vec.foundString.length() > max)
            {
                sendString = vec;
                max = vec.foundString.length();
            }
            index++;
        }
    
    return sendString;
}