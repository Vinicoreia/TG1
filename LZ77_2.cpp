#include <bits/stdc++.h>
#include <fstream>
#define BUFFERSIZE 20000
#define DICTSIZE 2047
#define LOOKAHEADSIZE 2047
#define DICTBITS 10
#define LOOKBITS 10
#define WINDOWSIZE LOOKAHEADSIZE + DICTSIZE
// This is a simple implementation of the lempel-ziv algorithm
using namespace std;

typedef struct code
{
    string foundString;
    size_t offset;
    char nextChar;
} code;

deque<char> lookahead;
deque<char> dict;
string buffer;
deque<tuple<size_t, size_t, char>> codigo;
int windowPointer = 0;    /*aponta para o final do lookahead*/
int dictPointer = 0;      /*aponta pro indice no buffer do inicio do dicionario*/
int lookaheadPointer = 0; /*aponta pro indice no buffer do inicio do lookahead*/
int filesize = 0;
int bufferCount = 0;
int bufferPointer = 0;
int numPop = 0;
/*offset, length, char*/
tuple<size_t, size_t, char> pegaMaiorSubstring(); /*Deve ser alterada pra retornar 1bit de flag 4 bits de deslocamento 3 bits de tamanho*/

void advanceWindow(int comprimento = 1);
int getNextBuffer(fstream &file);
void getFileSize(fstream &file);
string charToBin(char c);

void printVector(deque<char> path)
{
    for (auto i = path.begin(); i != path.end(); ++i)
        std::cout << *i;
    cout << endl;
}

void printPointers()
{
    cout << "DP: " << dictPointer << " LP: " << lookaheadPointer << " WP: " << windowPointer << " BP: " << bufferPointer << endl;
}
void printVectors()
{
    cout << "DICT: ";
    printVector(dict);
    cout << "LOOKAHEAD: ";
    printVector(lookahead);
}

int main(int argc, char *argv[])
{
    fstream file("teste.txt", ios::binary | ios::in | ios::out); // lida com o arquivo, abre em modo binario
    if (file.is_open())
    {
        getFileSize(file); // pega o tamanho do arquivo
        cout << "reading binary file to buffer\n\n";
        string procura;
        tuple<size_t, size_t, char> tuplaElementos;
        while (getNextBuffer(file))
        {
            while (dictPointer < bufferPointer)
            {
                tuplaElementos = pegaMaiorSubstring();
                if (dictPointer >= buffer.size())
                {
                    break;
                }
                codigo.push_back(tuplaElementos);
                advanceWindow(get<1>(tuplaElementos) + 1);
            }
        }
    }
    file.close();
    // fstream newFile("encode.txt", ios::binary | ios::in | ios::out); // lida com o arquivo, abre em modo binario
    // if (newFile.is_open()){
    size_t lookaheadBits = 3;
    size_t dictBits = 6;
    int jump;
    int len;
    char next;
    char primeiro = get<2>(codigo[0]);
    int max=0;
    // newFile.write(primeiro);
    codigo.pop_front();
    string bitString;
    bitString.clear();
    while (codigo.size() > 0)
    {
        jump = get<0>(codigo[0]);
        len = get<1>(codigo[0]);
        next = get<2>(codigo[0]);
        cout<< jump<< " "<<len<<" "<<next<<endl;
        if(jump>max){
            max = jump;
        }
        codigo.pop_front();
        if (jump == 0)
        {
            bitString += "00";
            bitString.append(charToBin(next));
        }
        else if(jump>0 and len==1){
            bitString += "01"; /*FLAG*/
            bitString.append(charToBin(next));                        }
        else{
            bitString += "10"; /*FLAG*/
            bitString.append(bitset<DICTBITS>(jump).to_string());
            bitString.append(bitset<LOOKBITS>(len).to_string());
            bitString.append(charToBin(next));
        }
    }
    cout<<"MAX"<<max<<endl;
    cout<<bitString.size()/8;
    // }
    return 0;
}
void getFileSize(fstream &file)
{
    file.clear();
    file.seekg(0, ios::beg);
    file.seekg(0, ios::end);
    filesize = file.tellg();
    file.seekg(0, ios::beg);
    file.clear();
}
void advanceWindow(int comprimento)
{
    dictPointer += comprimento;
    dict.clear();
    lookahead.clear();
    if (dictPointer < buffer.size())
    {

        for (int i = lookaheadPointer, j = 0; j < LOOKAHEADSIZE and i < buffer.size(); i++, j++)
        {
            lookahead.push_back(buffer.at(i));
        }
        for (int i = lookaheadPointer - 1, j = 0; j < DICTSIZE and i >= 0; i--, j++)
        {
            dict.push_front(buffer.at(i));
        }
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

void fillBuffer(fstream &file)
{
    file.seekg(0, ios::beg);
    file.seekp(bufferCount * BUFFERSIZE);
    int filePosition = file.tellg();
    if (filesize - filePosition > 0)
    {
        if (filesize - filePosition > 0 and filesize - filePosition < BUFFERSIZE)
        {
            buffer.resize(filesize - filePosition); /* Coloca 2048 bytes na memoria*/
            file.read(&buffer[0], buffer.size());   /*Le os bytes*/
            bufferPointer = buffer.size();
        }
        else
        {

            buffer.resize(BUFFERSIZE);         /* Coloca 2048 bytes na memoria*/
            file.read(&buffer[0], BUFFERSIZE); /*Le os bytes*/
            bufferPointer = BUFFERSIZE;
        }
    }
}
void clearStreams()
{
    dictPointer = 0;
    lookaheadPointer = 0;
    windowPointer = 0;
    buffer.clear();
    lookahead.clear();
    dict.clear();
}

int getNextBuffer(fstream &file)
{
    clearStreams();
    fillBuffer(file);
    if (buffer.size() > 0)
    {
        for (int i = 0; i < LOOKAHEADSIZE + 1 and i < buffer.size(); i++)
        {
            lookahead.push_back(buffer.at(i));
            windowPointer++;
        }
        lookaheadPointer++;
        dict.push_back(lookahead.at(0));
        codigo.push_back({0, 0, lookahead.at(0)});
        lookahead.pop_front();
    }

    bufferCount++;
    if (bufferCount * bufferPointer > filesize or buffer.size() == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*pra otimizar a windowsize basta depois definir que se o numero de bytes que eu uso pra representar <o,l,c> for maior
    do que a quantidade de bytes da string, manda a string e um identificador*/
tuple<size_t, size_t, char> pegaMaiorSubstring()
{
    vector<code> substring;
    deque<char>::iterator charIt;
    string procura;
    size_t posicao;
    size_t max = 0;
    int windowPointerBefore = windowPointer;
    int j = 0;
    numPop = 0;
    int i = 0;
    int index = 0;
    char charProcurado;
    substring.clear();
    procura.clear();

    charIt = find_first_of(dict.begin(), dict.end(), lookahead.begin(), lookahead.begin() + 1);
    posicao = distance(dict.begin(), charIt);
    if (posicao >= dict.size())
    {
        lookaheadPointer++;
        windowPointer++;
        return make_tuple(0, 0, lookahead.at(0));
    }
    else
    {
        /*adiciona sequencias*/
        int k = 1;
        while (posicao < dict.size())
        {
            for (int i = 0; i < lookahead.size(); i++)
            {
                if (lookahead.size() <= 1)
                {
                    procura += lookahead.at(i);
                    substring.push_back({procura, dict.size() - posicao, '\0'});
                    break;
                }
                else if (lookahead.at(i) == dict.at((posicao + i) % dict.size()) and i < LOOKAHEADSIZE)
                {
                    procura += lookahead.at(i);
                    if (i == lookahead.size() - 1)
                    {
                        if (dictPointer >= buffer.size())
                        {
                            substring.push_back({procura, dict.size() - posicao, '\0'});
                            break;
                        }
                        if (windowPointer < buffer.size())
                        {
                            numPop++;
                            /*Pegar prox lookahead*/
                            lookahead.push_back(buffer.at(windowPointer));
                            windowPointer++;
                        }
                    }
                }
                else
                {
                    substring.push_back({procura, dict.size() - posicao, lookahead.at(i)});
                    break;
                }
            }
            if (k >= dict.size())
            {
                break;
            }
            charIt = find_first_of(dict.begin() + k, dict.end(), lookahead.begin(), lookahead.begin() + 1);
            posicao = distance(dict.begin(), charIt);
            k++;
            procura.clear();
        }
    }

    for (int i = 0; i < numPop; i++)
    {
        lookahead.pop_front();
    }
    reverse(substring.begin(), substring.end());
    if (substring.size() != 0)
    {
        for (auto vec : substring)
        {
            if (vec.foundString.length() > max)
            {
                max = vec.foundString.length();
                index++;
            }
        }
        if (substring[index - 1].foundString.length() >= 1)
        {
            lookaheadPointer += substring[index - 1].foundString.length() + 1;
            windowPointer = lookaheadPointer + lookahead.size();
            return make_tuple(substring[index - 1].offset, substring[index - 1].foundString.length(), substring[index - 1].nextChar);
        }
    }
}
