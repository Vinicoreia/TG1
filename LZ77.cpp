#include<bits/stdc++.h>
#include <fstream>
#define BUFFERSIZE 2048
#define DICTSIZE 12
#define LOOKAHEADSIZE 6 
#define WINDOWSIZE LOOKAHEADSIZE+DICTSIZE
// This is a simple implementation of the lempel-ziv algorithm
using namespace std;

typedef struct code{
    string foundString;
    size_t offset;
    char nextChar;
} code;

string lookahead;
string buffer;
string dicionario;
string window;
int dictPointer=0;
int windowPointer=0;
int filesize=0;
int bufferPointer=0;
/*offset, length, char*/
tuple<size_t, size_t, char> pegaMaiorSubstring(); /*Deve ser alterada pra retornar 1bit de flag 4 bits de deslocamento 3 bits de tamanho*/

void advanceWindow(int comprimento=1);
void getNextBuffer(fstream &file);
void getFileSize(fstream &file);

int main(int argc, char *argv[]){
    fstream file("teste.txt", ios::binary | ios::in | ios::out); // lida com o arquivo, abre em modo binario
    if (file.is_open())
    {
        getFileSize(file); // pega o tamanho do arquivo
        cout << "reading binary file to buffer\n\n";
        string procura;        
        tuple<size_t, size_t, char> tuplaElementos;
        vector<tuple<size_t,size_t,char>> codigo;
        while(bufferPointer < filesize){
            getNextBuffer(file);
            while (lookahead.size() > 0)
            {
                tuplaElementos = pegaMaiorSubstring();
                codigo.push_back(tuplaElementos);

                cout << get<0>(tuplaElementos) << " " << get<1>(tuplaElementos) << " " << get<2>(tuplaElementos) << endl;

                if (lookahead.size() <= 1)
                {
                    break;
                }
                advanceWindow(get<1>(tuplaElementos) + 1);
            }
        }
        }
    file.close();
    return 0;
}
void getFileSize(fstream &file){
        file.seekg(0, ios::end);
        filesize = file.tellg();
        file.clear();
        file.seekg(0, ios::beg);

        
    }
    void advanceWindow(int comprimento){
        /*Avança a window offset+1*/
        dictPointer += comprimento;
        if (dictPointer>DICTSIZE)
            dictPointer=DICTSIZE;
    if(dictPointer==DICTSIZE && windowPointer== buffer.size() && window.size()>DICTSIZE){
        rotate(window.begin(), window.begin() + comprimento, window.end());
        window.erase(window.size()-comprimento,comprimento);
        dicionario.replace(dicionario.begin(), dicionario.end(), window.substr(0, DICTSIZE));
        lookahead.replace(lookahead.begin(), lookahead.end(), window.substr(dicionario.size(), dicionario.size()-window.size())); 
    }else{
        rotate(window.begin(), window.begin() + comprimento, window.end());
        if (comprimento <= 16 && windowPointer < buffer.size())
        {
            window.replace(WINDOWSIZE - comprimento, comprimento, buffer.substr(windowPointer, comprimento));
            windowPointer += comprimento;
        }
        else if (comprimento > 16 && windowPointer < buffer.size())
        {

            window.replace(0, WINDOWSIZE, buffer.substr(comprimento - WINDOWSIZE, WINDOWSIZE));

            windowPointer += comprimento;
        }
        else
        {
            window.replace(window.size() - comprimento, comprimento, "\0");
        }
        lookahead.replace(lookahead.begin(), lookahead.end(), window.substr(DICTSIZE, LOOKAHEADSIZE));
        dicionario.replace(dicionario.begin(), dicionario.end(), window.substr(DICTSIZE - dictPointer, dictPointer));
    }
}

void fillBuffer(fstream &file){
    file.seekp(bufferPointer);
    if (BUFFERSIZE > filesize-bufferPointer){
        buffer.resize(filesize-bufferPointer); /* Coloca 2048 bytes na memoria*/
        file.read(&buffer[0], buffer.size()); /*Le os bytes*/
        bufferPointer+=filesize;
    }
    else{
        buffer.resize(BUFFERSIZE); /* Coloca 2048 bytes na memoria*/
        file.read(&buffer[0], BUFFERSIZE); /*Le os bytes*/
        bufferPointer+=BUFFERSIZE;
    }
    
}
void clearStreams(){
    dictPointer = 0;
    windowPointer = 0;
    buffer.clear();
    window.clear();
    lookahead.clear();
    dicionario.clear();
}

void getNextBuffer(fstream &file){
    clearStreams();
    fillBuffer(file);

    window.resize(WINDOWSIZE); /*Define o tamanho da janela*/
    lookahead.resize(LOOKAHEADSIZE); /*Define o tamanho do buffer lookahead*/
    dicionario.resize(0);

    lookahead.replace(lookahead.begin(), lookahead.end(), buffer.substr(0, LOOKAHEADSIZE));/* Inicializa o buffer lookahead*/
    window.replace(DICTSIZE, LOOKAHEADSIZE, lookahead);
    dicionario.append(lookahead,0,1); //o primeiro elemento sempre deve ser adicionado no dicionario
    rotate(lookahead.begin(),lookahead.begin()+1,lookahead.end());
    lookahead[LOOKAHEADSIZE-1] = buffer[LOOKAHEADSIZE];
    rotate(window.begin(),window.begin()+1,window.end());
    window.replace(DICTSIZE, LOOKAHEADSIZE, lookahead);
    windowPointer += LOOKAHEADSIZE+1;
    dictPointer += 1; /*Conta quantos caracteres tem no dicionario*/
    }


/*pra otimizar a windowsize basta depois definir que se o numero de bytes que eu uso pra representar <o,l,c> for maior
    do que a quantidade de bytes da string, manda a string e um identificador*/
tuple<size_t, size_t, char> pegaMaiorSubstring(){
    vector<code> substring;
    substring.clear();
    string procura;
    procura.clear();
    size_t posicao;
    size_t max = 0;
    int j=0;
    int i=0;
    int index = 0;
    char charAtual = lookahead[0];
    char charProcurado;
    posicao = dicionario.find_first_of(lookahead[i]);

    while(posicao!= string::npos && i<LOOKAHEADSIZE){
        for(int j=0, k=0;;j++, k++){
            /*lookahead movel*/
            /*Caso a maior substring seja do tamanho do lookahead, move o lookahead pra frente e continua procurando*/
            if(k==LOOKAHEADSIZE){
                rotate(lookahead.begin(),lookahead.begin()+1,lookahead.end());
                lookahead[LOOKAHEADSIZE-1] = buffer[windowPointer];
                windowPointer+=1;
                k--;
            }
            /*PROCURA MAIOR SUBSTRING*/
            if(dicionario[(posicao+j)%(dicionario.size())]==lookahead[k] && j<2^LOOKAHEADSIZE){
                procura.append(lookahead,k,1); /*talvez eu tenha que definir um maximo*/
            }else{
                if(k<LOOKAHEADSIZE){
                    charProcurado = lookahead[k];
                    substring.push_back({procura, posicao, charProcurado});
                }else{
                    charProcurado = lookahead[k + 1];
                    substring.push_back({procura, posicao, charProcurado});
                }
                procura.clear();
                break;
            }
            
        }
        i++;
        posicao = dicionario.find_first_of(lookahead[0],posicao+1);
    }
    if (substring.size()!= 0){
        for(auto vec:substring){
            if (vec.foundString.length() > max)
            {
                max = vec.foundString.length();
                index++;
            }
        }
        if (substring[index-1].foundString.length()>=1){
            return make_tuple((dicionario.size() - substring[index - 1].offset), substring[index - 1].foundString.length(), substring[index - 1].nextChar);
        }
    }
    else{
        return make_tuple(0,0,charAtual);
    }
}