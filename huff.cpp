
#include "huff.h"

using namespace std;
/*Simple implementation of the huffman compression algorithm*/

int filesize;
std::string buffer;
std::unordered_map<char, long long> mapSymbAmount;/*-*/
std::unordered_map<char, string> mapSymbCode;

std::vector<std::pair<char, long long>> pairSymbProb; /*-*/
std::priority_queue<node *, vector<node *>, compare> heap;
std::vector<std::pair<char, int>> pairSymbCodeLength;
std::unordered_map<char, std::pair<std::string, int>> mapSymbCodeLength;
std::string out;

void getFileSize(ifstream &file);
void getFrequency();
void huffmanEncode();
void huffmanDecode();
void mapCodes(struct node *root, string str);
string readFromFile(ifstream &fileIn);
std::string WriteOutString();
std::string charToBin(char c);
void buildCodes();
int main(int argc, char *argv[])
{
    ifstream file("teste.txt", ios::in | ios::binary | ios::ate);
    getFileSize(file);
    buffer = readFromFile(file);
    huffmanEncode();
    huffmanDecode();
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
    mapSymbAmount.clear();
    mapSymbCode.clear();
    pairSymbProb.clear();
    pairSymbCodeLength.clear();
    mapSymbCodeLength.clear();
    pairSymbCodeLength.clear();
    /*read Header*/
    std::unordered_map<string, char> mapCodeSymb;

    int addedZeros = stoi(out.substr(0,3), 0, 2);
    out.resize(out.size()-addedZeros);
    std::cout<<endl<<"ENTRANDO NO Decoder"<<endl<<endl;
    std::vector<int> codes = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int k =3;
    for(int i =0; i<16; i++){
        if(out[k] == '1'){
            codes[i] = stoi(out.substr(k+1, 5), 0, 2);
            k+=6;
        }else{
            k+=1;
        }
    }
    char c;
    for(int j = 0; j<codes.size(); j++){
        for(int i =0; i< codes[j]; i++){
           c = stol(out.substr(k, 8), 0, 2);
           pairSymbCodeLength.push_back(make_pair(c, j+1));
           k+=8;
        }
    }
    buildCodes();
    std::cout<<out.substr(k)<<endl<<endl;
    /*Até aqui tudo certo*/

    string decoding;
    string decoded;
    string fin;
    for (unordered_map<char, std::pair<std::string, int>>::iterator i = mapSymbCodeLength.begin(); i != mapSymbCodeLength.end(); ++i){
        mapCodeSymb[i->second.first] = i->first;
        }

    for (std::string::iterator it = out.begin() + k; it != out.end(); it++)
    {
        decoding += *it;
        std::unordered_map<string, char>::const_iterator g = mapCodeSymb.find(decoding);
        if (g!=mapCodeSymb.end()){
            fin+=g->second;
            decoded.clear();
            decoding.clear();
        }
    }
    std::cout<<fin;
    /*Lê o arquivo binario*/
    //std::string bitString;
    // while (fileBuffer.size() > 0)
    // {
    //     bitString.append(charToBin(fileBuffer.at(0)));
    //     fileBuffer.erase(0, 1);
    // }

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

    mapCodes(heap.top(), ""); /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
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
    /*Essa função pode ser melhorada pois só precisamos calcular o tamanho do código de cada elemento*/
    if (!root)
        return;

    if (root->leaf){
        pairSymbCodeLength.push_back(make_pair(root->code, str.length()));
    }

    mapCodes(root->left, str + "0");
    mapCodes(root->right, str + "1");
}

std::string WriteOutString(){
    char c, lastChar;
    std::vector<int> codeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int count;
    lastChar = (char)0x1f;

    std::sort(pairSymbCodeLength.begin(), pairSymbCodeLength.end(), [](auto &left, auto &right) {
            return left.second < right.second;
    });

    for (int i = 0; i < pairSymbCodeLength.size(); i++)
    {
        int index = pairSymbCodeLength[i].second;
        codeLengths[index - 1] += 1;
    }
    /*HEADER*/
    
    for (int i = 0; i < codeLengths.size(); i++)
    {
        if (codeLengths[i] == 0)
        {
            out += "0";
        }
        else
        {
            out += "1";
            bitset<5> bs = codeLengths[i];
            out.append(bs.to_string());
        }
    }

    for (auto it: pairSymbCodeLength){
        out.append(charToBin(it.first));
    }
    /*FIM DO HEADER*/
    buildCodes();
    std::cout<<buffer.size()<<endl;
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

    std::cout<<out<<endl<<endl;
    return out;
}

void buildCodes(){
    int count;
    int code;
    int last_code;
    std::vector<int> codes = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    for (int i = 0; i < pairSymbCodeLength.size(); i++)
    {
        int index = pairSymbCodeLength[i].second;
        codes[index - 1] += 1;
    }

    for (int i = static_cast<int>(codes.size()) - 1; i >= 0; --i)
    {
        if (codes.at(i) != 0)
        {
            count = i + 1;
            break;
        }
    }

    code =0;
    std::vector<int> start_code;
    start_code.resize(count);
    start_code[count-1] = code;
    last_code = codes[count - 1];
    count--;
    for (int i = count-1; i >= 0; i--)
    {
        code = code + last_code;
        code = code >> 1;
        start_code[i] = code;
        last_code = codes[i];
    }

    std::sort(pairSymbCodeLength.begin(), pairSymbCodeLength.end(), [](auto &left, auto &right) {
        return left.first < right.first;
    });

    int tamanho;
    string codeStr;
    for(int i =0; i<pairSymbCodeLength.size(); i++){
        tamanho = pairSymbCodeLength[i].second -1;
        bitset<16> bs = start_code[tamanho];
        codeStr = bs.to_string().substr(16-pairSymbCodeLength[i].second);

        mapSymbCodeLength[pairSymbCodeLength[i].first] = make_pair(codeStr, pairSymbCodeLength[i].second);
        start_code[tamanho] += 1;
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
