
#include "huff.h"

using namespace std;
/*Simple implementation of the huffman compression algorithm*/

int filesize;
std::string buffer;
std::unordered_map<char, long long> mapSymbAmount;
std::vector<std::pair<char, long long>> pairSymbProb;
std::priority_queue<node *, vector<node *>, compare> heap;
std::unordered_map<char, string> mapSymbCode;

void getFileSize(ifstream &file);
void getFrequency();
void huffmanEncode();
void huffmanDecode();
void mapCodes(struct node *root, string str);
string readFromFile(ifstream &fileIn);
std::string WriteOutString();
std::string charToBin(char c);

int main(int argc, char *argv[])
{
    ifstream file("teste.txt", ios::in | ios::binary | ios::ate);
    getFileSize(file);
    buffer = readFromFile(file);
    huffmanEncode();

    file.close();
    return 0;
    }


void getFrequency()
{
    for (long long i = 0; i < buffer.length(); i++)
    {
        mapSymbAmount[buffer[i]] += 1;
    }

    for (std::unordered_map<char, long long>::iterator it = mapSymbAmount.begin(); it != mapSymbAmount.end(); ++it)
    {
        pairSymbProb.push_back(std::make_pair((char)(it->first), (long long)(it->second)));
    }

    std::sort(pairSymbProb.begin(), pairSymbProb.end(), [](auto &left, auto &right) {
        return left.second < right.second;
    });
    std::sort(pairSymbProb.begin(), pairSymbProb.end(), [](auto &left, auto &right) {
        return left.first < right.first;
    });
    
}

void huffmanDecode()
{
    std::string bitString;
    int count=0;
    while (count<buffer.size())
    {
        bitString.append(charToBin(buffer.at(count)));
        count++;
    }
    bitset<3> b(bitString.substr(0,3));
    count = (int)b.to_ulong();
    bitString.erase(bitString.length()-count);
    bitString.erase(0, 3);
    
    // std::string outbuffer;
    // struct node *nLeft, *nRight, *nTop;
    // for (int i = 0; i < pairSymbProb.size(); i++)
    // {
    //     heap.push(new node(pairSymbProb[i].first, pairSymbProb[i].second, true));
    // }
    // while (heap.size() != 1)
    // {
    //     nLeft = heap.top();
    //     heap.pop();
    //     nRight = heap.top();
    //     heap.pop();
    //     nTop = new node(char(0x1f), nLeft->key_value + nRight->key_value, false);
    //     nTop->left = nLeft;
    //     nTop->right = nRight;
    //     heap.push(nTop);
    // }
    // mapCodes(heap.top(), "");
    // outbuffer = WriteOutString();

    // ofstream output("teste.bin", ios::out | ios::binary);
    // unsigned long c;
    // std::cout << outbuffer.size() / 8;
    // int counter = 0;
    // while (counter < outbuffer.size())
    // {
    //     bitset<8> b(outbuffer.substr(counter, 8));
    //     c = b.to_ulong();
    //     output.write(reinterpret_cast<const char *>(&c), 1);
    //     counter += 8;
    // }
    // output.close();
}


void huffmanEncode(){
    getFrequency();
    std::string outbuffer;
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
    mapCodes(heap.top(), "");
    outbuffer = WriteOutString();

    ofstream output("teste.bin", ios::out | ios::binary);
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

void mapCodes(struct node *root, string str)
{

    if (!root)
        return;

    if (root->leaf){
        std::cout<<root->code;
        mapSymbCode[root->code] = str;
        std::cout << " " << str << endl;
    }

    mapCodes(root->left, str + "0");
    mapCodes(root->right, str + "1");
}

std::string WriteOutString(){
    std::string out;
    char c;
    int count;
    std::vector<std::pair<char, int>> pairSymbLength;

    for (auto it : mapSymbCode)
    {
        pairSymbLength.push_back(make_pair(it.first, it.second.length()));
        
        // out += charToBin(it.first);
        // out.append(it.second);
        // out += charToBin(char(0x1f));
    }

    std::sort(pairSymbLength.begin(), pairSymbLength.end(), [](auto &left, auto &right){
        return left.second < right.second;
    });
    std::sort(pairSymbLength.begin(), pairSymbLength.end(), [](auto &left, auto &right){
        return left.first < right.first;
    });
    std::cout<<std::endl;
    for (auto it : pairSymbLength)
    {
        std::cout<<it.first<<" "<<it.second<<std::endl;
        // out += charToBin(it.first);
        // out.append(it.second);
        // out += charToBin(char(0x1f));
    }
    for (int i = 0; i < buffer.size(); i++)
    {
        out.append(mapSymbCode[buffer[i]]);
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
