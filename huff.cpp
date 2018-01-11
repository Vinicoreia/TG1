
#include "huff.h"

using namespace std;
/*Simple implementation of the huffman compression algorithm*/

int filesize;

string bitString;
string buffer;
string out;
string fileNameIn;
string fileNameOut;

unordered_map<char, long long> mapSymbAmount;/*-*/
unordered_map<char, string> mapSymbCode;
unordered_map<char, pair<string, int>> mapSymbCodeLength;
priority_queue<node *, vector<node *>, compare> heap;
vector<pair<char, long long>> pairSymbProb; /*-*/
vector<pair<char, int>> pairSymbCodeLength;
vector<int> codeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void getFileSize(ifstream &file);
void getFrequency();
void huffmanEncode();
void huffmanDecode();
void calcCodeLengths();
void mapCodes(struct node *root, int len=0);
void buildCodes();
void writeBitString();
string readFromFile(ifstream &fileIn);
string WriteOutString();
string charToBin(char c);

int main(int argc, char *argv[])
{

    string fileMode;
    if (argc == 4)
    {
        fileNameOut = argv[3];
        fileNameIn  = argv[2];
        fileMode    = argv[1];
        ifstream file(fileNameIn, ios::in | ios::binary | ios::ate);

        if (file.is_open())
        {
            getFileSize(file);
            cout << "Your file has : " << filesize << " bytes" << endl;
            buffer = readFromFile(file);
            if (fileMode == "compress")
            {
                huffmanEncode();
            }
            else if (fileMode == "decompress")
            {
                writeBitString();
                huffmanDecode();
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


void getFrequency()
{
    for (long long i = 0; i < buffer.length(); i++)
    {
        mapSymbAmount[buffer[i]] += 1;
    }

    for (unordered_map<char, long long>::iterator it = mapSymbAmount.begin(); it != mapSymbAmount.end(); ++it)
    {
        pairSymbProb.push_back(make_pair((char)(it->first), (long long)(it->second)));
    }

    sort(pairSymbProb.begin(), pairSymbProb.end(), [](auto &left, auto &right) {
        return left.second < right.second;
    });
    sort(pairSymbProb.begin(), pairSymbProb.end(), [](auto &left, auto &right) {
        return left.first < right.first;
    });
    
}
void writeBitString(){
    for (int i = 0; i < buffer.size(); i++)
    {   
        bitset<8> bin(buffer.c_str()[i]);
        bitString += bin.to_string();
    }
}

void huffmanDecode()
{
    string decoding;
    string decoded;
    unordered_map<string, char> mapCodeSymb;
    char c;
    int strPointer =3;
    int addedZeros = stoi(bitString.substr(0, 3), 0, 2);
    bitString.resize(bitString.size() - addedZeros);
    
    for(int i =0; i<16; i++){
        if (bitString[strPointer] == '1')
        {
            codeLengths[i] = stoi(bitString.substr(strPointer + 1, 7), 0, 2);
            strPointer+=8;
        }else{
            strPointer+=1;
        }
    }
    
    for (int j = 0; j < codeLengths.size(); j++)
    {
        for (int i = 0; i < codeLengths[j]; i++)
        {
            c = stol(bitString.substr(strPointer, 8), 0, 2);
            pairSymbCodeLength.push_back(make_pair(c, j + 1));
            strPointer += 8;
        }
    }
    buildCodes();
    
    for (unordered_map<char, pair<string, int>>::iterator i = mapSymbCodeLength.begin(); i != mapSymbCodeLength.end(); ++i){
        mapCodeSymb[i->second.first] = i->first;
        }

    string dec;
    for (string::iterator it = bitString.begin() + strPointer; it != bitString.end(); it++)
    {
        decoding += *it;
        try{
            decoded += mapCodeSymb.at(decoding);
            decoding.clear();
        }
        catch (const out_of_range& e)
        {
        }
    }
    ofstream output(fileNameOut, ios::out | ios::binary);
    output << decoded; //WRITE TO FILE
    output.close();
    
}


void huffmanEncode(){
    getFrequency();
    string outbuffer;
    
    struct node *nLeft, *nRight, *nTop;    
    for(int i=0; i< pairSymbProb.size(); i++){
        heap.push(new node(pairSymbProb[i].first, pairSymbProb[i].second, true));
    }

    while(heap.size()!=1){
        nLeft = heap.top();
        heap.pop();
        nRight = heap.top();
        heap.pop();
        nTop = new node(char(0x1f), nLeft->key_value + nRight->key_value, false);
        nTop->left = nLeft;
        nTop->right = nRight;
        heap.push(nTop);
    }

    mapCodes(heap.top()); /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
    outbuffer = WriteOutString();

    ofstream output(fileNameOut, ios::out | ios::binary);
    unsigned long c;
    int counter=0;
    while (counter<outbuffer.size())
    {
        bitset<8> b(outbuffer.substr(counter, 8));
        c = b.to_ulong();
        output.write(reinterpret_cast<const char *>(&c), 1);
        counter += 8;
    }
    output.close();
}

void mapCodes(struct node *root, int len)
{
    /*Essa função pode ser melhorada pois só precisamos calcular o tamanho do código de cada elemento*/
    if (!root)
        return;

    if (root->leaf){
        pairSymbCodeLength.push_back(make_pair(root->code, len));
    }
    mapCodes(root->left, len+1);
    mapCodes(root->right, len+1);
}

void calcCodeLengths(){
    sort(pairSymbCodeLength.begin(), pairSymbCodeLength.end(), [](auto &left, auto &right) {
        if(left.second==right.second){
            return left.first<right.first;
        }
        return left.second < right.second;
    });

    for (int i = 0; i < pairSymbCodeLength.size(); i++)
    {
        int index = pairSymbCodeLength[i].second;
        codeLengths[index - 1] += 1;
    }
}
string WriteOutString(){
    int count=0;
    /*HEADER*/
    calcCodeLengths();
    for (int i = 0; i < codeLengths.size(); i++)
    {
        if (codeLengths[i] == 0)
        {
            out += "0";
        }
        else
        {
            out += "1";
            bitset<7> bs = codeLengths[i];
            out.append(bs.to_string());
        }
    }

    for (auto it: pairSymbCodeLength){
        out.append(charToBin(it.first));
    }

    /*FIM DO HEADER*/
    buildCodes();

    for (int i = 0; i < buffer.size(); i++)
    {
        out.append(mapSymbCodeLength[buffer[i]].first);
    }

    while (((out.size()+3) % 8) != 0)
    {
        out += "0";
        count += 1;
    }
    bitset<3> bs = count;

    out.insert(0, bs.to_string());
    return out;
}

void buildCodes(){
    vector<int> start_code;
    int count, code, nCodes;
    code =0;

    for (int i = static_cast<int>(codeLengths.size()) - 1; i >= 0; --i)
    {
        if (codeLengths.at(i) != 0)
        {
            count = i + 1;
            break;
        }
    }

    start_code.resize(count);
    start_code[count-1] = code;
    nCodes = codeLengths[count - 1];
    count--;
    for (int i = count-1; i >= 0; i--)
    {
        code = code + nCodes;
        code = code >> 1;
        start_code[i] = code;
        nCodes = codeLengths[i];
    }

    int codeLen;
    string codeStr;
    
    for(int i =0; i<pairSymbCodeLength.size(); i++){
        codeLen = pairSymbCodeLength[i].second - 1;
        bitset<16> bs = start_code[codeLen];
        codeStr = bs.to_string().substr(16-pairSymbCodeLength[i].second);

        mapSymbCodeLength[pairSymbCodeLength[i].first] = make_pair(codeStr, pairSymbCodeLength[i].second);
        start_code[codeLen] += 1;
    }
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
string readFromFile(ifstream & fileIn)
{
    return static_cast<stringstream const &>(stringstream() << fileIn.rdbuf()).str();
}
void getFileSize(ifstream & file)
{
    filesize = file.tellg();
    file.seekg(0, ios::beg);
    file.clear();
}
