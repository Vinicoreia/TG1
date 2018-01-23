#include "adaptativeDeflate.h"
#define DICTSIZE 255
#define LOOKAHEADSIZE 255
#define WINDOWSIZE LOOKAHEADSIZE + DICTSIZE
#define DICTBITS 8
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
string out;

std::vector<USIZE> bufferOffLen;
std::vector<char> bufferChar;
unordered_map<char, long long> mapcharAmount;    /*-*/
vector<pair<char, long long>> paircharProb;      /*-*/
vector<pair<USIZE, long long>> pairOffLenProb;   /*-*/
unordered_map<USIZE, long long> mapOffLenAmount; /*-*/
priority_queue<node1 *, vector<node1 *>, compare1> heapChar;
priority_queue<node2 *, vector<node2 *>, compare2> heapOffLen;
unordered_map<char, pair<string, int>> mapCharCodeLength;
unordered_map<USIZE, pair<string, int>> mapOffLenCodeLength;
deque<code> codeTriples;

/*****************************************************************/
unordered_map<char, long long> mapcharAmount1; /*-*/
unordered_map<char, long long> mapcharAmount2; /*-*/
unordered_map<char, long long> mapcharAmount3; /*-*/
unordered_map<char, long long> mapcharAmount4; /*-*/

vector<pair<char, long long>> paircharProb1; /*-*/
vector<pair<char, long long>> paircharProb2; /*-*/
vector<pair<char, long long>> paircharProb3; /*-*/
vector<pair<char, long long>> paircharProb4; /*-*/

unordered_map<USIZE, long long> mapOffLenAmount1; /*-*/
unordered_map<USIZE, long long> mapOffLenAmount2; /*-*/
unordered_map<USIZE, long long> mapOffLenAmount3; /*-*/
unordered_map<USIZE, long long> mapOffLenAmount4; /*-*/

vector<pair<USIZE, long long>> pairOffLenProb1; /*-*/
vector<pair<USIZE, long long>> pairOffLenProb2; /*-*/
vector<pair<USIZE, long long>> pairOffLenProb3; /*-*/
vector<pair<USIZE, long long>> pairOffLenProb4; /*-*/

priority_queue<node1 *, vector<node1 *>, compare1> heapChar1;
priority_queue<node2 *, vector<node2 *>, compare2> heapOffLen1;

priority_queue<node1 *, vector<node1 *>, compare1> heapChar2;
priority_queue<node2 *, vector<node2 *>, compare2> heapOffLen2;

priority_queue<node1 *, vector<node1 *>, compare1> heapChar3;
priority_queue<node2 *, vector<node2 *>, compare2> heapOffLen3;

priority_queue<node1 *, vector<node1 *>, compare1> heapChar4;
priority_queue<node2 *, vector<node2 *>, compare2> heapOffLen4;

unordered_map<char, pair<string, int>> mapCharCodeLength1;
unordered_map<USIZE, pair<string, int>> mapOffLenCodeLength1;

unordered_map<char, pair<string, int>> mapCharCodeLength2;
unordered_map<USIZE, pair<string, int>> mapOffLenCodeLength2;

unordered_map<char, pair<string, int>> mapCharCodeLength3;
unordered_map<USIZE, pair<string, int>> mapOffLenCodeLength3;

unordered_map<char, pair<string, int>> mapCharCodeLength4;
unordered_map<USIZE, pair<string, int>> mapOffLenCodeLength4;

vector<pair<char, int>> pairCharCodeLength1;
vector<pair<USIZE, int>> pairOffLenCodeLength1;
vector<int> charCodeLengths1 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
vector<int> offLenCodeLengths1 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

vector<pair<char, int>> pairCharCodeLength2;
vector<pair<USIZE, int>> pairOffLenCodeLength2;
vector<int> charCodeLengths2 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
vector<int> offLenCodeLengths2 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

vector<pair<char, int>> pairCharCodeLength3;
vector<pair<USIZE, int>> pairOffLenCodeLength3;
vector<int> charCodeLengths3 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
vector<int> offLenCodeLengths3 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

vector<pair<char, int>> pairCharCodeLength4;
vector<pair<USIZE, int>> pairOffLenCodeLength4;
vector<int> charCodeLengths4 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
vector<int> offLenCodeLengths4 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/*****************************************************************/
void getBlockFrequency(std::unordered_map<char, long long> &mapcharAmountN, std::vector<pair<char, long long>> &paircharProbN, unordered_map<USIZE, long long> &mapOffLenAmountN,
                       vector<pair<USIZE, long long>> &pairOffLenProbN);

void createBlockHeap(priority_queue<node1 *, vector<node1 *>, compare1> &heapCharN, vector<pair<char, long long>> &paircharProbN, priority_queue<node2 *, vector<node2 *>, compare2> &heapOffLenN, vector<pair<USIZE, long long>> &pairOffLenProbN, vector<pair<char, int>> &pairCharCodeLengthN, vector<pair<USIZE, int>> &pairOffLenCodeLengthN);

vector<pair<char, int>> pairCharCodeLength;
vector<int> charCodeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
vector<pair<USIZE, int>> pairOffLenCodeLength;
vector<int> offLenCodeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void splitBlocks();
void getFrequency();
string charToBin(char c);
string readFileToBuffer(ifstream &in);
void getFileSize(ifstream &file);
void fillBuffer(fstream &file);
void CompressFile(ifstream &file);
void getNextWindow(size_t matchSize = 0, size_t jump = 0);
code getBiggestSubstring();
void decompressFile(ifstream &file);
void startWindow();

void huffmanEncode();
void huffmanDecode();
void calcCodeLengths1(vector<pair<char, int>> &pairCharCodeLengthN, vector<int> &charCodeLengthsN);
void calcCodeLengths2(vector<pair<USIZE, int>> &pairOffLenCodeLengthN, vector<int> &OffLenCodeLengthsN);
void mapCodes1(struct node1 *root, vector<pair<char, int>> &pairCharCodeLength, int len = 0);
void mapCodes2(struct node2 *root, vector<pair<USIZE, int>> &pairOffLenCodeLength, int len = 0);
void buildCodes1(vector<pair<char, int>> &pairCharCodeLengthN, vector<int> &charCodeLengthsN, unordered_map<char, pair<string, int>> &mapCharCodeLengthN);
void buildCodes2(vector<pair<USIZE, int>> &pairOffLenCodeLengthN, vector<int> &OffLenCodeLengthsN, unordered_map<USIZE, pair<string, int>> &mapOffLenCodeLengthN);
void decompCode(unordered_map<string, char> &mapCharCodeN, unordered_map<string, USIZE> &mapOffLenCodeN, unordered_map<char, pair<string, int>> &mapCharCodeLengthN, unordered_map<USIZE, pair<string, int>> &mapOffLenCodeLengthN);
void writeBitString();
string WriteOutString();

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
    /*Aqui eu tenho todas as triplas do tipo offset, length, nextchar*/
    USIZE aux = 0;
    for (auto it : codeTriples)
    {
        aux = it.offset;
        aux = aux << SHIFT;
        aux |= it.foundString.length();
        bufferOffLen.push_back(aux);
        bufferChar.push_back(it.nextChar);
    }
    huffmanEncode();
}
void writeBitString()
{
    for (int i = 0; i < fileBuffer.size(); i++)
    {
        bitset<8> bin(fileBuffer.c_str()[i]);
        bitString += bin.to_string();
    }
}

void decompressFile(ifstream &file)
{
    writeBitString();
    string decoding;
    string decoded;
    unordered_map<string, char> mapCharCode1;
    unordered_map<string, USIZE> mapOffLenCode1;

    unordered_map<string, char> mapCharCode2;
    unordered_map<string, USIZE> mapOffLenCode2;

    unordered_map<string, char> mapCharCode3;
    unordered_map<string, USIZE> mapOffLenCode3;

    unordered_map<string, char> mapCharCode4;
    unordered_map<string, USIZE> mapOffLenCode4;
    char c;
    int strPointer = 3;
    int addedZeros = stoi(bitString.substr(0, 3), 0, 2);
    bitString.resize(bitString.size() - addedZeros);

    for (int i = 0; i < 20; i++)
    {
        if (bitString[strPointer] == '1')
        {
            charCodeLengths[i] = stoi(bitString.substr(strPointer + 1, 8), 0, 2);
            strPointer += 9;
        }
        else
        {
            strPointer += 1;
        }
    }

    for (int j = 0; j < charCodeLengths.size(); j++)
    {
        for (int i = 0; i < charCodeLengths[j]; i++)
        {
            c = stol(bitString.substr(strPointer, 8), 0, 2);
            pairCharCodeLength.push_back(make_pair(c, j + 1));
            strPointer += 8;
        }
    }
    /*Agora pra Uint*/

    for (int i = 0; i < 20; i++)
    {
        if (bitString[strPointer] == '1')
        {
            offLenCodeLengths[i] = stoi(bitString.substr(strPointer + 1, 12), 0, 2);
            strPointer += 13;
        }
        else
        {
            strPointer += 1;
        }
    }
    USIZE aux;
    for (int j = 0; j < offLenCodeLengths.size(); j++)
    {
        for (int i = 0; i < offLenCodeLengths[j]; i++)
        {
            aux = stoll(bitString.substr(strPointer, 8 * sizeof(USIZE)), 0, 2);
            pairOffLenCodeLength.push_back(make_pair(aux, j + 1));
            strPointer += 8 * sizeof(USIZE);
        }
    }
    splitBlocks();


    buildCodes1(pairCharCodeLength1, charCodeLengths1, mapCharCodeLength1);
    buildCodes2(pairOffLenCodeLength1, offLenCodeLengths1, mapOffLenCodeLength1);

    buildCodes1(pairCharCodeLength2, charCodeLengths2, mapCharCodeLength2);
    buildCodes2(pairOffLenCodeLength2, offLenCodeLengths2, mapOffLenCodeLength2);

    buildCodes1(pairCharCodeLength3, charCodeLengths3, mapCharCodeLength3);
    buildCodes2(pairOffLenCodeLength3, offLenCodeLengths3, mapOffLenCodeLength3);

    buildCodes1(pairCharCodeLength4, charCodeLengths4, mapCharCodeLength4);
    buildCodes2(pairOffLenCodeLength4, offLenCodeLengths4, mapOffLenCodeLength4);

    decompCode(mapCharCode1, mapOffLenCode1, mapCharCodeLength1, mapOffLenCodeLength1);
    decompCode(mapCharCode2, mapOffLenCode2, mapCharCodeLength2, mapOffLenCodeLength2);
    decompCode(mapCharCode3, mapOffLenCode3, mapCharCodeLength3, mapOffLenCodeLength3);
    decompCode(mapCharCode4, mapOffLenCode4, mapCharCodeLength4, mapOffLenCodeLength4);

    std::deque<char> ch;
    std::deque<USIZE> offLen;
    
    bitString.erase(0, strPointer);
    string state = bitString.substr(0,2);
    
    
    for(int i=2; i<bitString.size(); i++){
        decoding += bitString[i];
        try
        {   
            if(state=="00"){
                offLen.push_back(mapOffLenCode1.at(decoding));
                state = bitString[i+1];
                state += bitString[i+2];
                i+=2;
                decoding.clear();

                for (int j = i + 1; j != bitString.size(); j++)
                {
                    decoding += bitString[j];
                    try
                    {
                        if(state=="00"){
                            ch.push_back(mapCharCode1.at(decoding));
                            decoding.clear();
                            i = j;
                            state = bitString[i+1];
                            state += bitString[i+2];
                            i += 2;
                            break;
                        }else if(state =="01"){
                            ch.push_back(mapCharCode2.at(decoding));
                            decoding.clear();
                            i = j;
                            state = bitString[i+1];
                            state += bitString[i+2];
                            i += 2;
                            break;
                        }
                        else if (state == "10")
                        {
                            ch.push_back(mapCharCode3.at(decoding));
                            decoding.clear();
                            i = j;
                            state = bitString[i+1];
                            state += bitString[i+2];
                            i += 2;
                            break;
                        }
                        else if (state == "11")
                        {
                            ch.push_back(mapCharCode4.at(decoding));
                            decoding.clear();
                            i = j;
                            state = bitString[i+1];
                            state += bitString[i+2];
                            i += 2;
                            break;
                        }
                        
                    }
                    catch (const out_of_range &e)
                    {
                    }
                }
            }
            else if (state == "01")
            {
                offLen.push_back(mapOffLenCode2.at(decoding));
                state = bitString[i+1];
                state += bitString[i+2];
                i += 2;
                decoding.clear();

                for (int j = i + 1; j != bitString.size(); j++)
                {
                    decoding += bitString[j];
                    try
                    {
                        if (state == "00")
                        {
                            ch.push_back(mapCharCode1.at(decoding));
                            decoding.clear();
                            i = j;
                            state = bitString[i+1];
                            state += bitString[i+2];
                            i += 2;
                            break;
                        }
                        else if (state == "01")
                        {
                            ch.push_back(mapCharCode2.at(decoding));
                            decoding.clear();
                            i = j;
                            state = bitString[i+1];
                            state += bitString[i+2];
                            i += 2;
                            break;
                        }
                        else if (state == "10")
                        {
                            ch.push_back(mapCharCode3.at(decoding));
                            decoding.clear();
                            i = j;
                            state = bitString[i+1];
                            state += bitString[i+2];
                            i += 2;
                            break;
                        }
                        else if (state == "11")
                        {
                            ch.push_back(mapCharCode4.at(decoding));
                            decoding.clear();
                            i = j;
                            state = bitString[i+1];
                            state += bitString[i+2];
                            i += 2;
                            break;
                        }
                    }
                    catch (const out_of_range &e)
                    {
                    }
                }
            }
            else if (state == "10")
            {
                offLen.push_back(mapOffLenCode3.at(decoding));
                state = bitString[i+1];
                state += bitString[i+2];
                i += 2;
                decoding.clear();

                for (int j = i + 1; j != bitString.size(); j++)
                {
                    decoding += bitString[j];
                    try
                    {
                        if (state == "00")
                        {
                            ch.push_back(mapCharCode1.at(decoding));
                            decoding.clear();
                            i = j;
                            state = bitString[i+1];
                            state += bitString[i+2];
                            i += 2;
                            break;
                        }
                        else if (state == "01")
                        {
                            ch.push_back(mapCharCode2.at(decoding));
                            decoding.clear();
                            i = j;
                            state = bitString[i+1];
                            state += bitString[i+2];
                            i += 2;
                            break;
                        }
                        else if (state == "10")
                        {
                            ch.push_back(mapCharCode3.at(decoding));
                            decoding.clear();
                            i = j;
                            state = bitString[i+1];
                            state += bitString[i+2];
                            i += 2;
                            break;
                        }
                        else if (state == "11")
                        {
                            ch.push_back(mapCharCode4.at(decoding));
                            decoding.clear();
                            i = j;
                            state = bitString[i+1];
                            state += bitString[i+2];
                            i += 2;
                            break;
                        }
                    }
                    catch (const out_of_range &e)
                    {
                    }
                }
            }
            else if (state == "11")
            {
                offLen.push_back(mapOffLenCode4.at(decoding));
                state = bitString[i+1];
                state += bitString[i+2];
                i += 2;
                decoding.clear();

                for (int j = i + 1; j != bitString.size(); j++)
                {
                    decoding += bitString[j];
                    try
                    {
                        if (state == "00")
                        {
                            ch.push_back(mapCharCode1.at(decoding));
                            decoding.clear();
                            i = j;
                            state = bitString[i+1];
                            state += bitString[i+2];
                            i += 2;
                            break;
                        }
                        else if (state == "01")
                        {
                            ch.push_back(mapCharCode2.at(decoding));
                            decoding.clear();
                            i = j;
                            state = bitString[i+1];
                            state += bitString[i+2];
                            i += 2;
                            break;
                        }
                        else if (state == "10")
                        {
                            ch.push_back(mapCharCode3.at(decoding));
                            decoding.clear();
                            i = j;
                            state = bitString[i+1];
                            state += bitString[i+2];
                            i += 2;
                            break;
                        }
                        else if (state == "11")
                        {
                            ch.push_back(mapCharCode4.at(decoding));
                            decoding.clear();
                            i = j;
                            state = bitString[i+1];
                            state += bitString[i+2];
                            i += 2;
                            break;
                        }
                    }
                    catch (const out_of_range &e)
                    {
                    }
                }
            }
        }
        catch (const out_of_range &e)
        {
        }
    }

    decoding.clear();

    string bitChar;
    string lookaheadBits;
    string dictBits;
    string outString;
    int jump;
    int len;
    char nextChar;
    for (int k = 0; k < ch.size(); k++)
    {
        nextChar = ch[k];
        jump = offLen[k] >> SHIFT;
        len = offLen[k] & MASK;
        if (jump == 0)
        {
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
    cout << "Final filesize after decompressing: " << outString.size() << " bytes" << endl;
    output.close();
}
void decompCode(unordered_map<string, char> &mapCharCodeN, unordered_map<string, USIZE> &mapOffLenCodeN, unordered_map<char, pair<string, int>> &mapCharCodeLengthN, unordered_map<USIZE, pair<string, int>> &mapOffLenCodeLengthN)
{
    for (unordered_map<char, pair<string, int>>::iterator i = mapCharCodeLengthN.begin(); i != mapCharCodeLengthN.end(); ++i)
    {
        mapCharCodeN[i->second.first] = i->first;
    }

    for (unordered_map<USIZE, pair<string, int>>::iterator i = mapOffLenCodeLengthN.begin(); i != mapOffLenCodeLengthN.end(); ++i)
    {
        mapOffLenCodeN[i->second.first] = i->first;
    }
}
string offLenToBin(USIZE c)
{
    string charBin;
    charBin.clear();
    for (int i = 8 * sizeof(USIZE) - 1; i >= 0; --i)
    {
        (c & (1 << i)) ? charBin += '1' : charBin += '0';
    }
    return charBin;
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
    if (windowPointer >= filesize)
    {
        windowPointer = filesize;
        lookahead.resize(windowPointer - lookaheadPointer);
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
            if (i == lookahead.size() - 1)
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
void getBlockFrequency(std::unordered_map<char, long long> &mapcharAmountN, std::vector<pair<char, long long>> &paircharProbN, unordered_map<USIZE, long long> &mapOffLenAmountN,
                       vector<pair<USIZE, long long>> &pairOffLenProbN)
{
    for (unordered_map<char, long long>::iterator it = mapcharAmountN.begin(); it != mapcharAmountN.end(); ++it)
    {
        paircharProbN.push_back(make_pair((char)(it->first), (long long)(it->second)));
    }

    sort(paircharProbN.begin(), paircharProbN.end(), [](auto &left, auto &right) {
        return left.second < right.second;
    });
    sort(paircharProbN.begin(), paircharProbN.end(), [](auto &left, auto &right) {
        return left.first < right.first;
    });

    for (unordered_map<USIZE, long long>::iterator it = mapOffLenAmountN.begin(); it != mapOffLenAmountN.end(); ++it)
    {
        pairOffLenProbN.push_back(make_pair((it->first), (long long)(it->second)));
    }

    sort(pairOffLenProbN.begin(), pairOffLenProbN.end(), [](auto &left, auto &right) {
        return left.second < right.second;
    });
    sort(pairOffLenProbN.begin(), pairOffLenProbN.end(), [](auto &left, auto &right) {
        return left.first < right.first;
    });
}
void getFrequency()
{
    for (long long i = 0; i < bufferChar.size(); i++)
    {
        mapcharAmount[bufferChar[i]] += 1;
    }

    for (unordered_map<char, long long>::iterator it = mapcharAmount.begin(); it != mapcharAmount.end(); ++it)
    {
        paircharProb.push_back(make_pair((char)(it->first), (long long)(it->second)));
    }

    sort(paircharProb.begin(), paircharProb.end(), [](auto &left, auto &right) {
        return left.second < right.second;
    });
    sort(paircharProb.begin(), paircharProb.end(), [](auto &left, auto &right) {
        return left.first < right.first;
    });

    for (long long i = 0; i < bufferOffLen.size(); i++)
    {
        mapOffLenAmount[bufferOffLen[i]] += 1;
    }

    for (unordered_map<USIZE, long long>::iterator it = mapOffLenAmount.begin(); it != mapOffLenAmount.end(); ++it)
    {
        pairOffLenProb.push_back(make_pair((it->first), (long long)(it->second)));
    }

    sort(pairOffLenProb.begin(), pairOffLenProb.end(), [](auto &left, auto &right) {
        return left.second < right.second;
    });
    sort(pairOffLenProb.begin(), pairOffLenProb.end(), [](auto &left, auto &right) {
        return left.first < right.first;
    });
}

void huffmanEncode()
{
    getFrequency();
    string outbuffer;

    struct node1 *nLeft1, *nRight1, *nTop1;
    for (int i = 0; i < paircharProb.size(); i++)
    {
        heapChar.push(new node1(paircharProb[i].first, paircharProb[i].second, true));
    }

    struct node2 *nLeft2, *nRight2, *nTop2;
    for (int i = 0; i < pairOffLenProb.size(); i++)
    {
        heapOffLen.push(new node2(pairOffLenProb[i].first, pairOffLenProb[i].second, true));
    }

    while (heapChar.size() != 1)
    {
        nLeft1 = heapChar.top();
        heapChar.pop();
        nRight1 = heapChar.top();
        heapChar.pop();
        nTop1 = new node1(char(0x1f), nLeft1->key_value + nRight1->key_value, false);
        nTop1->left = nLeft1;
        nTop1->right = nRight1;
        heapChar.push(nTop1);
    }

    while (heapOffLen.size() != 1)
    {
        nLeft2 = heapOffLen.top();
        heapOffLen.pop();
        nRight2 = heapOffLen.top();
        heapOffLen.pop();
        nTop2 = new node2((USIZE)0x1f, nLeft2->key_value + nRight2->key_value, false);
        nTop2->left = nLeft2;
        nTop2->right = nRight2;
        heapOffLen.push(nTop2);
    }

    mapCodes1(heapChar.top(), pairCharCodeLength); /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
    mapCodes2(heapOffLen.top(), pairOffLenCodeLength); /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
    calcCodeLengths1(pairCharCodeLength, charCodeLengths);
    calcCodeLengths2(pairOffLenCodeLength, offLenCodeLengths);
    splitBlocks();
    outbuffer = WriteOutString();
    cout << "Final filesize after compressing: " << outbuffer.size() / 8 << " bytes" << endl;

    ofstream output(fileNameOut, ios::out | ios::binary);
    unsigned long c;
    int counter = 0;
    while (counter < outbuffer.size())
    {
        bitset<8> b(outbuffer.substr(counter, 8));
        c = b.to_ulong();
        output.write(reinterpret_cast<const char *>(&c), 1);
        counter += 8;
    }
    output.close();
}

void splitBlocks(){
    for (auto it : pairOffLenCodeLength){
        if(it.second<=5){
            mapOffLenAmount1[it.first] += 1;
        }
        else if (it.second > 5 and it.second <=10)
        {
            mapOffLenAmount2[it.first] += 1;
        }
        else if (it.second >10 and it.second <=15)
        {
            mapOffLenAmount3[it.first] += 1;
        }
        else if (it.second > 15 and it.second <=20)
        {
            mapOffLenAmount4[it.first] += 1;
        }
        else
        {
            std::cout << "codigo maior que 20 bits ";
            std::cout<<it.second<<endl;
        }
    }
    for (auto it : pairCharCodeLength)
    {
        if (it.second <= 5)
        {
            mapcharAmount1[it.first] += 1;
        }
        else if (it.second > 5 and it.second <= 10)
        {
            mapcharAmount2[it.first] += 1;
        }
        else if (it.second > 10 and it.second <= 15)
        {
            mapcharAmount3[it.first] += 1;
        }
        else if (it.second > 15 and it.second <= 20)
        {
            mapcharAmount4[it.first] += 1;
        }
        else
        {
            std::cout << "codigo maior que 20 bits";
            std::cout << it.second << endl;
        }
    }

    getBlockFrequency(mapcharAmount1, paircharProb1, mapOffLenAmount1, pairOffLenProb1);
    getBlockFrequency(mapcharAmount2, paircharProb2, mapOffLenAmount2, pairOffLenProb2);
    getBlockFrequency(mapcharAmount3, paircharProb3, mapOffLenAmount3, pairOffLenProb3);
    getBlockFrequency(mapcharAmount4, paircharProb4, mapOffLenAmount4, pairOffLenProb4);
    
    createBlockHeap(heapChar1, paircharProb1, heapOffLen1, pairOffLenProb1, pairCharCodeLength1, pairOffLenCodeLength1);
    createBlockHeap(heapChar2, paircharProb2, heapOffLen2, pairOffLenProb2, pairCharCodeLength2, pairOffLenCodeLength2);
    createBlockHeap(heapChar3, paircharProb3, heapOffLen3, pairOffLenProb3, pairCharCodeLength3, pairOffLenCodeLength3);
    createBlockHeap(heapChar4, paircharProb4, heapOffLen4, pairOffLenProb4, pairCharCodeLength4, pairOffLenCodeLength4);
    
    calcCodeLengths1(pairCharCodeLength1, charCodeLengths1);
    calcCodeLengths2(pairOffLenCodeLength1, offLenCodeLengths1);

    calcCodeLengths1(pairCharCodeLength2, charCodeLengths2);
    calcCodeLengths2(pairOffLenCodeLength2, offLenCodeLengths2);

    calcCodeLengths1(pairCharCodeLength3, charCodeLengths3);
    calcCodeLengths2(pairOffLenCodeLength3, offLenCodeLengths3);

    calcCodeLengths1(pairCharCodeLength4, charCodeLengths4);
    calcCodeLengths2(pairOffLenCodeLength4, offLenCodeLengths4);
}

void createBlockHeap(priority_queue<node1 *, vector<node1 *>, compare1> &heapCharN, vector<pair<char, long long>> &paircharProbN, priority_queue<node2 *, vector<node2 *>, compare2> &heapOffLenN, vector<pair<USIZE, long long>> &pairOffLenProbN, vector<pair<char, int>> &pairCharCodeLengthN, vector<pair<USIZE, int>> &pairOffLenCodeLengthN)
{
    struct node2 *nLeft2, *nRight2, *nTop2;
    struct node1 *nLeft1, *nRight1, *nTop1;

    for (int i = 0; i < paircharProbN.size(); i++)
    {
        heapCharN.push(new node1(paircharProbN[i].first, paircharProbN[i].second, true));
    }
    for (int i = 0; i < pairOffLenProbN.size(); i++)
    {
        heapOffLenN.push(new node2(pairOffLenProbN[i].first, pairOffLenProbN[i].second, true));
    }

    while (heapCharN.size() > 1)
    {
        nLeft1 = heapCharN.top();
        heapCharN.pop();
        nRight1 = heapCharN.top();
        heapCharN.pop();
        nTop1 = new node1(char(0x1f), nLeft1->key_value + nRight1->key_value, false);
        nTop1->left = nLeft1;
        nTop1->right = nRight1;
        heapCharN.push(nTop1);
    }

    while (heapOffLenN.size() > 1)
    {
        nLeft2 = heapOffLenN.top();
        heapOffLenN.pop();
        nRight2 = heapOffLenN.top();
        heapOffLenN.pop();
        nTop2 = new node2((USIZE)0x1f, nLeft2->key_value + nRight2->key_value, false);
        nTop2->left = nLeft2;
        nTop2->right = nRight2;
        heapOffLenN.push(nTop2);
    }
    if (!heapCharN.empty())
    {
        mapCodes1(heapCharN.top(), pairCharCodeLengthN); /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
    }
    if (!heapOffLenN.empty())
    {
        mapCodes2(heapOffLenN.top(), pairOffLenCodeLengthN); /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
    }
}
void mapCodes1(struct node1 *root, vector<pair<char, int>> &pairCharCodeLengthN, int len)
{   
    if (!root)
        return;

    if (root->leaf)
    {
        pairCharCodeLengthN.push_back(make_pair(root->code, len));
    }
    mapCodes1(root->left, pairCharCodeLengthN, len + 1);
    mapCodes1(root->right, pairCharCodeLengthN, len + 1);
}

void mapCodes2(struct node2 *root, vector<pair<USIZE, int>> &pairOffLenCodeLengthN, int len)
{
    if (!root)
        return;

    if (root->leaf)
    {
        pairOffLenCodeLengthN.push_back(make_pair(root->code, len));
    }
    mapCodes2(root->left, pairOffLenCodeLengthN, len + 1);
    mapCodes2(root->right, pairOffLenCodeLengthN, len + 1);
}

void calcCodeLengths1(vector<pair<char, int>> &pairCharCodeLengthN, vector<int> &charCodeLengthsN)
{
    sort(pairCharCodeLengthN.begin(), pairCharCodeLengthN.end(), [](auto &left, auto &right) {
        if (left.second == right.second)
        {
            return left.first < right.first;
        }
        return left.second < right.second;
    });

    for (int i = 0; i < pairCharCodeLengthN.size(); i++)
    {
        int index = pairCharCodeLengthN[i].second;
        charCodeLengthsN[index - 1] += 1;
    }
}

void calcCodeLengths2(vector<pair<USIZE, int>> &pairOffLenCodeLengthN, vector<int> &offLenCodeLengthsN)
{
    sort(pairOffLenCodeLengthN.begin(), pairOffLenCodeLengthN.end(), [](auto &left, auto &right) {
        if (left.second == right.second)
        {
            return left.first < right.first;
        }
        return left.second < right.second;
    });

    for (int i = 0; i < pairOffLenCodeLengthN.size(); i++)
    {
        int index = pairOffLenCodeLengthN[i].second;
        offLenCodeLengthsN[index - 1] += 1;
    }
}

string WriteOutString()
{
    int count = 0;
    /*HEADER*/

    for (int i = 0; i < charCodeLengths.size(); i++)
    {
        if (charCodeLengths[i] == 0)
        {
            out += "0";
        }
        else
        {
            out += "1";
            bitset<8> bs = charCodeLengths[i];
            out.append(bs.to_string());
        }
    }

    for (auto it : pairCharCodeLength)
    {
        out.append(charToBin(it.first));
    }

    for (int i = 0; i < offLenCodeLengths.size(); i++)
    {
        if (offLenCodeLengths[i] == 0)
        {
            out += "0";
        }
        else
        {
            out += "1";
            bitset<12> bs = offLenCodeLengths[i];
            out.append(bs.to_string());
        }
    }

    for (auto it : pairOffLenCodeLength)
    {
        out.append(offLenToBin(it.first));
    }

    /*FIM DO HEADER*/

    buildCodes1(pairCharCodeLength, charCodeLengths, mapCharCodeLength);
    buildCodes2(pairOffLenCodeLength, offLenCodeLengths, mapOffLenCodeLength);

    buildCodes1(pairCharCodeLength1,charCodeLengths1,mapCharCodeLength1);
    buildCodes2(pairOffLenCodeLength1, offLenCodeLengths1, mapOffLenCodeLength1);

    buildCodes1(pairCharCodeLength2, charCodeLengths2, mapCharCodeLength2);
    buildCodes2(pairOffLenCodeLength2, offLenCodeLengths2, mapOffLenCodeLength2);

    buildCodes1(pairCharCodeLength3, charCodeLengths3, mapCharCodeLength3);
    buildCodes2(pairOffLenCodeLength3, offLenCodeLengths3, mapOffLenCodeLength3);

    buildCodes1(pairCharCodeLength4, charCodeLengths4, mapCharCodeLength4);
    buildCodes2(pairOffLenCodeLength4, offLenCodeLengths4, mapOffLenCodeLength4);

    USIZE aux;

    for(auto it:offLenCodeLengths4){
        std::cout<<it<<", ";
    }

    for (auto it : codeTriples)
    {
        aux = it.offset;
        aux = aux << SHIFT;
        aux |= it.foundString.length();
        if (mapOffLenCodeLength[aux].first.length() <= 5)
        {
            out.append("00");
            out.append(mapOffLenCodeLength1[aux].first);
        }
        else if (mapOffLenCodeLength[aux].first.length() > 5 and mapOffLenCodeLength[aux].first.length() <= 10)
        {
            out.append("01");
            out.append(mapOffLenCodeLength2[aux].first);
        }
        else if (mapOffLenCodeLength[aux].first.length() > 10 and mapOffLenCodeLength[aux].first.length() <= 15)
        {
            out.append("10");
            out.append(mapOffLenCodeLength3[aux].first);
        }
        else if (mapOffLenCodeLength[aux].first.length() > 15 and mapOffLenCodeLength[aux].first.length() <= 20)
        {
            out.append("11");
            out.append(mapOffLenCodeLength4[aux].first);
        }
        else
        {
            std::cout << "codigo maior que 20 bits";
        }

        if (mapCharCodeLength[it.nextChar].first.length() <= 5)
        {
            out.append("00");
            out.append(mapCharCodeLength1[it.nextChar].first);
        }
        else if (mapCharCodeLength[it.nextChar].first.length() > 5 and mapCharCodeLength[it.nextChar].first.length() <= 10)
        {
            out.append("01");
            out.append(mapCharCodeLength2[it.nextChar].first);
        }
        else if (mapCharCodeLength[it.nextChar].first.length() > 10 and mapCharCodeLength[it.nextChar].first.length() <= 15)
        {
            out.append("10");
            out.append(mapCharCodeLength3[it.nextChar].first);
        }
        else if (mapCharCodeLength[it.nextChar].first.length() > 15 and mapCharCodeLength[it.nextChar].first.length() <= 20)
        {
            out.append("11");
            out.append(mapCharCodeLength4[it.nextChar].first);
        }
        else
        {
            std::cout << "codigo maior que 20 bits";
        }
    }

    while (((out.size() + 3) % 8) != 0)
    {
        out += "0";
        count += 1;
    }
    bitset<3> bs = count;
    out.insert(0, bs.to_string());
    return out;
}

void buildCodes1(vector<pair<char, int>> &pairCharCodeLengthN, vector<int> &charCodeLengthsN, unordered_map<char, pair<string, int>> &mapCharCodeLengthN)
{
    vector<int> start_code;
    int count, code, nCodes;
    code = 0;
    bool zeros = std::all_of(charCodeLengthsN.begin(), charCodeLengthsN.end(), [](int i) { return i == 0; });
    if(zeros==1){
        return;
    }

    for (int i = static_cast<int>(charCodeLengthsN.size()) - 1; i >= 0; --i)
    {
        if (charCodeLengthsN.at(i) != 0)
        {
            count = i + 1;
            break;
        }
    }

    start_code.resize(count);
    start_code[count - 1] = code;
    nCodes = charCodeLengthsN[count - 1];
    count--;
    for (int i = count - 1; i >= 0; i--)
    {
        code = code + nCodes;
        code = code >> 1;
        start_code[i] = code;
        nCodes = charCodeLengthsN[i];
    }

    int codeLen;
    string codeStr;

    for (int i = 0; i < pairCharCodeLengthN.size(); i++)
    {
        codeLen = pairCharCodeLengthN[i].second - 1;
        bitset<20> bs = start_code[codeLen];
        codeStr = bs.to_string().substr(20 - pairCharCodeLengthN[i].second);

        mapCharCodeLengthN[pairCharCodeLengthN[i].first] = make_pair(codeStr, pairCharCodeLengthN[i].second);
        start_code[codeLen] += 1;
    }
}

void buildCodes2(vector<pair<USIZE, int>> &pairOffLenCodeLengthN, vector<int> &offLenCodeLengthsN, unordered_map<USIZE, pair<string, int>> &mapOffLenCodeLengthN)
{
    vector<int> start_code;
    int count, code, nCodes;
    code = 0;

    bool zeros = std::all_of(offLenCodeLengthsN.begin(), offLenCodeLengthsN.end(), [](int i) { return i == 0; });
    if (zeros == 1)
    {
        return;
    }

    for (int i = static_cast<int>(offLenCodeLengthsN.size()) - 1; i >= 0; --i)
    {
        if (offLenCodeLengthsN.at(i) != 0)
        {
            count = i + 1;
            break;
        }
    }

    start_code.resize(count);
    start_code[count - 1] = code;
    nCodes = offLenCodeLengthsN[count - 1];
    count--;
    for (int i = count - 1; i >= 0; i--)
    {
        code = code + nCodes;
        code = code >> 1;
        start_code[i] = code;
        nCodes = offLenCodeLengthsN[i];
    }

    int codeLen;
    string codeStr;
    for (int i = 0; i < pairOffLenCodeLengthN.size(); i++)
    {
        codeLen = pairOffLenCodeLengthN[i].second - 1;
        bitset<20> bs = start_code[codeLen];
        codeStr = bs.to_string().substr(20 - pairOffLenCodeLengthN[i].second);

        mapOffLenCodeLengthN[pairOffLenCodeLengthN[i].first] = make_pair(codeStr, pairOffLenCodeLengthN[i].second);
        start_code[codeLen] += 1;
    }
}