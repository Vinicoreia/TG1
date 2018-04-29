
#include <unordered_map>
#include <vector>
#include <queue>
#include "huff.h"
#include "util.h"
void huffmanEncode(){
    std::vector<std::pair<char, long long>> pairSymbProb = getFrequency(strBuffer);
    std::priority_queue<node *, std::vector<node *>, compare> heap;

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
