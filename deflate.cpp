#include "deflate.h"
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
priority_queue<node1 *, vector<node1 *>, compare1> heap1;
priority_queue<node2 *, vector<node2 *>, compare2> heap2;
unordered_map<char, pair<string, int>> mapCharCodeLength;
unordered_map<USIZE, pair<string, int>> mapOffLenCodeLength;
deque<code> codeTriples;

vector<pair<char, int>> pairCharCodeLength;
vector<int> charCodeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
vector<pair<USIZE, int>> pairOffLenCodeLength;
vector<int> offLenCodeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
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
void calcCodeLengths1();
void calcCodeLengths2();

void mapCodes1(struct node1 *root, int len = 0);
void mapCodes2(struct node2 *root, int len = 0);

void buildCodes1();
void buildCodes2();

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
    unordered_map<string, char> mapCharCode;
    unordered_map<string, USIZE> mapOffLenCode;
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

    buildCodes1();
    buildCodes2();

    for (unordered_map<char, pair<string, int>>::iterator i = mapCharCodeLength.begin(); i != mapCharCodeLength.end(); ++i)
    {
        mapCharCode[i->second.first] = i->first;
    }

    for (unordered_map<USIZE, pair<string, int>>::iterator i = mapOffLenCodeLength.begin(); i != mapOffLenCodeLength.end(); ++i)
    {
        mapOffLenCode[i->second.first] = i->first;
    }

    std::deque<char> ch;
    std::deque<USIZE> offLen;
    for (string::iterator it = bitString.begin() + strPointer; it != bitString.end(); it++)
    {
        decoding += *it;
        try
        {
            offLen.push_back(mapOffLenCode.at(decoding));
            decoding.clear();
            for (string::iterator jt = it + 1; jt != bitString.end(); jt++)
            {
                decoding += *jt;
                try
                {
                    ch.push_back(mapCharCode.at(decoding));
                    decoding.clear();
                    it = jt;
                    break;
                }
                catch (const out_of_range &e)
                {
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
        heap1.push(new node1(paircharProb[i].first, paircharProb[i].second, true));
    }

    struct node2 *nLeft2, *nRight2, *nTop2;
    for (int i = 0; i < pairOffLenProb.size(); i++)
    {
        heap2.push(new node2(pairOffLenProb[i].first, pairOffLenProb[i].second, true));
    }

    while (heap1.size() != 1)
    {
        nLeft1 = heap1.top();
        heap1.pop();
        nRight1 = heap1.top();
        heap1.pop();
        nTop1 = new node1(char(0x1f), nLeft1->key_value + nRight1->key_value, false);
        nTop1->left = nLeft1;
        nTop1->right = nRight1;
        heap1.push(nTop1);
    }

    while (heap2.size() != 1)
    {
        nLeft2 = heap2.top();
        heap2.pop();
        nRight2 = heap2.top();
        heap2.pop();
        nTop2 = new node2((USIZE)0x1f, nLeft2->key_value + nRight2->key_value, false);
        nTop2->left = nLeft2;
        nTop2->right = nRight2;
        heap2.push(nTop2);
    }
    mapCodes1(heap1.top()); /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
    mapCodes2(heap2.top()); /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
    calcCodeLengths1();
    calcCodeLengths2();
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

void mapCodes1(struct node1 *root, int len)
{
    if (!root)
        return;

    if (root->leaf)
    {
        pairCharCodeLength.push_back(make_pair(root->code, len));
    }
    mapCodes1(root->left, len + 1);
    mapCodes1(root->right, len + 1);
}

void mapCodes2(struct node2 *root, int len)
{
    if (!root)
        return;

    if (root->leaf)
    {
        pairOffLenCodeLength.push_back(make_pair(root->code, len));
    }
    mapCodes2(root->left, len + 1);
    mapCodes2(root->right, len + 1);
}

void calcCodeLengths1()
{
    sort(pairCharCodeLength.begin(), pairCharCodeLength.end(), [](auto &left, auto &right) {
        if (left.second == right.second)
        {
            return left.first < right.first;
        }
        return left.second < right.second;
    });

    for (int i = 0; i < pairCharCodeLength.size(); i++)
    {
        int index = pairCharCodeLength[i].second;
        charCodeLengths[index - 1] += 1;
    }
}

void calcCodeLengths2()
{
    sort(pairOffLenCodeLength.begin(), pairOffLenCodeLength.end(), [](auto &left, auto &right) {
        if (left.second == right.second)
        {
            return left.first < right.first;
        }
        return left.second < right.second;
    });

    for (int i = 0; i < pairOffLenCodeLength.size(); i++)
    {
        int index = pairOffLenCodeLength[i].second;
        offLenCodeLengths[index - 1] += 1;
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
    buildCodes1();
    buildCodes2();

    USIZE aux;

    for (auto it : codeTriples)
    {
        aux = it.offset;
        aux = aux << SHIFT;
        aux |= it.foundString.length();
        out.append(mapOffLenCodeLength[aux].first);
        out.append(mapCharCodeLength[it.nextChar].first);
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

void buildCodes1()
{
    vector<int> start_code;
    int count, code, nCodes;
    code = 0;

    for (int i = static_cast<int>(charCodeLengths.size()) - 1; i >= 0; --i)
    {
        if (charCodeLengths.at(i) != 0)
        {
            count = i + 1;
            break;
        }
    }

    start_code.resize(count);
    start_code[count - 1] = code;
    nCodes = charCodeLengths[count - 1];
    count--;
    for (int i = count - 1; i >= 0; i--)
    {
        code = code + nCodes;
        code = code >> 1;
        start_code[i] = code;
        nCodes = charCodeLengths[i];
    }

    int codeLen;
    string codeStr;

    for (int i = 0; i < pairCharCodeLength.size(); i++)
    {
        codeLen = pairCharCodeLength[i].second - 1;
        bitset<20> bs = start_code[codeLen];
        codeStr = bs.to_string().substr(20 - pairCharCodeLength[i].second);

        mapCharCodeLength[pairCharCodeLength[i].first] = make_pair(codeStr, pairCharCodeLength[i].second);
        start_code[codeLen] += 1;
    }
}

void buildCodes2()
{
    vector<int> start_code;
    int count, code, nCodes;
    code = 0;

    for (int i = static_cast<int>(offLenCodeLengths.size()) - 1; i >= 0; --i)
    {
        if (offLenCodeLengths.at(i) != 0)
        {
            count = i + 1;
            break;
        }
    }

    start_code.resize(count);
    start_code[count - 1] = code;
    nCodes = offLenCodeLengths[count - 1];
    count--;
    for (int i = count - 1; i >= 0; i--)
    {
        code = code + nCodes;
        code = code >> 1;
        start_code[i] = code;
        nCodes = offLenCodeLengths[i];
    }

    int codeLen;
    string codeStr;
    for (int i = 0; i < pairOffLenCodeLength.size(); i++)
    {
        codeLen = pairOffLenCodeLength[i].second - 1;
        bitset<20> bs = start_code[codeLen];
        codeStr = bs.to_string().substr(20 - pairOffLenCodeLength[i].second);

        mapOffLenCodeLength[pairOffLenCodeLength[i].first] = make_pair(codeStr, pairOffLenCodeLength[i].second);
        start_code[codeLen] += 1;
    }
}