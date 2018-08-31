#include "deflate.h"
#include <cstdlib>
#include <algorithm>
#include <queue>
#include <bitset>

/*1- Como aplicar Burrows wheeler (antes e depois, só antes ou só depois)*/
/*2- Como splitar, quantos blocos, a partir de qual tamanho*/
/*3- lembrar do rle definido*/
#define MAXCODESIZE 10
std::string USIZEToBin(USIZE c)
{
    std::string charBin;
    charBin.clear();
    for (int i = 8 * sizeof(USIZE) - 1; i >= 0; --i)
    {
        (c & (1 << i)) ? charBin += '1' : charBin += '0';
    }
    return charBin;
}

void mapCodesUSIZE(struct nodeChar *root, int len, std::vector<std::pair<USIZE, int>> &pairCodeLength)
{
    if (!root)
        return;

    if (root->leaf)
    {
        if(len==0)
            len=1;
        pairCodeLength.push_back(std::make_pair(root->code, len));
    }
    mapCodesUSIZE(root->left, len + 1, pairCodeLength);
    mapCodesUSIZE(root->right, len + 1, pairCodeLength);
}

void mapCodesU16(struct nodeU16 *root, int len, std::vector<std::pair<uint16_t, int>> &pairCodeLength)
{
    if (!root)
        return;

    if (root->leaf)
    {
        if(len==0)
            len=1;
        pairCodeLength.push_back(std::make_pair(root->code, len));
    }
    mapCodesU16(root->left, len + 1, pairCodeLength);
    mapCodesU16(root->right, len + 1, pairCodeLength);
}

void calcUSIZECodeLengths(std::vector<std::pair<USIZE, int>> &pairOffLenCodeLength, std::vector<int> &offLenCodeLengths)
{

    std::sort(pairOffLenCodeLength.begin(), pairOffLenCodeLength.end(), [](auto &left, auto &right) {
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

void calcU16CodeLengths(std::vector<std::pair<uint16_t, int>> &pairOffLenCodeLength, std::vector<int> &offLenCodeLengths)
{

    std::sort(pairOffLenCodeLength.begin(), pairOffLenCodeLength.end(), [](auto &left, auto &right) {
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

void buildUSIZECodes(std::vector<std::pair<USIZE, int>> &pairOffLenCodeLength, std::vector<int> &offLenCodeLengths, std::unordered_map<USIZE, std::pair<std::string, int>> &mapOffLenCodeLength)
{
    std::vector<int> start_code;
    int count, code, nCodes;
    code = 0;
    count=0;
    nCodes = 0;
    start_code.clear();

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

    int codeLen = 0;
    std::string codeStr = "";
    for (int i = 0; i < pairOffLenCodeLength.size(); i++)
    {
        codeLen = pairOffLenCodeLength[i].second - 1;
        std::bitset<MAXCODESIZE> bs = start_code[codeLen];
        codeStr = bs.to_string().substr(MAXCODESIZE - pairOffLenCodeLength[i].second);

        mapOffLenCodeLength[pairOffLenCodeLength[i].first] = std::make_pair(codeStr, pairOffLenCodeLength[i].second);
        start_code[codeLen] += 1;
    }
    return;
}


void buildU16Codes(std::vector<std::pair<uint16_t, int>> &pairOffLenCodeLength, std::vector<int> &offLenCodeLengths, std::unordered_map<uint16_t, std::pair<std::string, int>> &mapOffLenCodeLength)
{
    std::vector<int> start_code;
    int count, code, nCodes;
    code = 0;
    count=0;
    nCodes = 0;
    start_code.clear();

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

    int codeLen = 0;
    std::string codeStr = "";
    for (int i = 0; i < pairOffLenCodeLength.size(); i++)
    {
        codeLen = pairOffLenCodeLength[i].second - 1;
        std::bitset<MAXCODESIZE> bs = start_code[codeLen];
        codeStr = bs.to_string().substr(MAXCODESIZE - pairOffLenCodeLength[i].second);

        mapOffLenCodeLength[pairOffLenCodeLength[i].first] = std::make_pair(codeStr, pairOffLenCodeLength[i].second);
        start_code[codeLen] += 1;
    }
    return;
}


int checkBiggestCode(std::string firstCode, std::string secondCode, std::string thirdCode){
    // if(firstCode.length() < secondCode.length() and (firstCode.length() > 0)){
        // smallest = firstCode;
        // if(thirdCode.length() < smallest.length()){
            // smallest = thirdCode;
        // }
    // }else if(secondCode.length() < firstCode.length() and secondCode.length > 0){
        // smallest = secondCode;
        // if(thirdCode.length() < smallest.length()){
            // smallest = thirdCode;
        // }
    // }
    if(firstCode.length() + secondCode.length() > thirdCode.length()){
        std::cout<<firstCode.length() + secondCode.length()<<" "<<thirdCode.length()<<"\n";
        return 1;
    }
    else{
        std::cout<<firstCode.length() + secondCode.length()<<" "<<thirdCode.length()<<"\n";
        return 0;
    }
}




std::string WriteDeflateBitString(
    std::deque<Data> &codeTriples,

    std::vector<std::pair<uint16_t, int>> &pairCharLenCodeLength,
    std::vector<int> &charLenCodeLengths,
    std::unordered_map<uint16_t, std::pair<std::string, int>> &mapCharLenCodeLength,
    
    std::vector<std::pair<uint16_t, int>> &pairJumpCodeLength,
    std::vector<int> &jumpCodeLengths,
    std::unordered_map<uint16_t, std::pair<std::string, int>> &mapJumpCodeLength,

    std::vector<std::pair<USIZE, int>> &pairJumpHighCodeLength,
    std::vector<int> &jumpHighCodeLengths,
    std::unordered_map<USIZE, std::pair<std::string, int>> &mapjumpHighCodeLength,

    std::vector<std::pair<USIZE, int>> &pairJumpLowCodeLength,
    std::vector<int> &jumpLowCodeLengths,
    std::unordered_map<USIZE, std::pair<std::string, int>> &mapjumpLowCodeLength,

    std::vector<std::pair<USIZE, int>> &pairCharCodeLength,
    std::vector<int> &charCodeLengths,
    std::unordered_map<USIZE, std::pair<std::string, int>> &mapCharCodeLength,

    std::vector<std::pair<USIZE, int>> &pairLenCodeLength,
    std::vector<int> &lenCodeLengths,
    std::unordered_map<USIZE, std::pair<std::string, int>> &mapLenCodeLength)
{
    int count = 0;
    srand(333);

    /*HEADER*/
    std::string out = "";

//     for (int i = 0; i < jumpHighCodeLengths.size(); i++)
//     {
//         if (jumpHighCodeLengths[i] == 0)
//         {
//             out += "0";
//         }
//         else
//         {
//             out += "1";
//             out.append(decimalToBitString(jumpHighCodeLengths[i], 10));
//         }
//     }

//     for (auto it : pairJumpHighCodeLength)
//     {
//         out.append(USIZEToBin(it.first));
//     }

//    for (int i = 0; i < jumpLowCodeLengths.size(); i++)
//     {
//         if (jumpLowCodeLengths[i] == 0)
//         {
//             out += "0";
//         }
//         else
//         {
//             out += "1";
//             out.append(decimalToBitString(jumpLowCodeLengths[i], 10));
//         }
//     }

//     for (auto it : pairJumpLowCodeLength)
//     {
//         out.append(USIZEToBin(it.first));
//     }


//     for (int i = 0; i < charCodeLengths.size(); i++)
//     {
//         if (charCodeLengths[i] == 0)
//         {
//             out += "0";
//         }
//         else
//         {
//             out += "1";
//             out.append(decimalToBitString(charCodeLengths[i], 8));
//         }
//     }

//     for (auto it : pairCharCodeLength)
//     {
//         out.append(USIZEToBin(it.first));
//     }

//     for (int i = 0; i < lenCodeLengths.size(); i++)
//     {
//         if (lenCodeLengths[i] == 0)
//         {
//             out += "0";
//         }
//         else
//         {
//             out += "1";
//             out.append(decimalToBitString(lenCodeLengths[i], 8));
//         }
//     }

//     for (auto it : pairLenCodeLength)
//     {
//         out.append(USIZEToBin(it.first));
//     }
    /*FIM DO HEADER*/
// for (int i = 0; i < charLenCodeLengths.size(); i++)
//     {
//         if (charLenCodeLengths[i] == 0)
//         {
//             out += "0";
//         }
//         else
//         {
//             out += "1";
//             out.append(decimalToBitString(charLenCodeLengths[i], 10));
//         }
//     }

    // for (auto it : pairCharLenCodeLength)
    // {
    //     out.append(USIZEToBin(it.first));
    // }

    // for (int i = 0; i < jumpCodeLengths.size(); i++)
    // {
    //     if (jumpCodeLengths[i] == 0)
    //     {
    //         out += "0";
    //     }
    //     else
    //     {
    //         out += "1";
    //         out.append(decimalToBitString(jumpCodeLengths[i], 8));
    //     }
    // }

    // for (auto it : pairJumpCodeLength)
    // {
    //     out.append(USIZEToBin(it.first));
    // }
    
    uint16_t aux;
    
    int random=0;

    for (auto it : codeTriples)
    {
        aux = it.nextChar;
        aux = aux << 8;
        random = rand()%7+1; //se o rand acertar o bit 1 estará ativo


    //    if(checkBiggestCode(mapjumpHighCodeLength[(it.offset >>8)].first, mapjumpLowCodeLength[(it.offset&MASK)].first, mapJumpCodeLength[it.offset].first)){
            // out.append("0");
    out.append(mapJumpCodeLength[it.offset].first);
            
    //    }else{
    //         out.append("1");
    //         out.append(mapjumpHighCodeLength[(it.offset >>8)].first);
    //         out.append(mapjumpLowCodeLength[(it.offset&MASK)].first);
    //    }

        if(it.match.length()==0){
            aux |= it.match.length();

            // if(checkBiggestCode(mapCharCodeLength[it.nextChar].first, mapLenCodeLength[it.match.length()].first, mapCharLenCodeLength[aux].first)){
                
                // out.append("1");
                out.append(mapCharLenCodeLength[aux].first);

            // }else{
            //     out.append("0");
            //     out.append(mapLenCodeLength[it.match.length()].first);
            //     out.append(mapCharCodeLength[it.nextChar].first);

            // }
        }else{
            aux |= (it.match.length()-3);

            // if(checkBiggestCode(mapCharCodeLength[it.nextChar].first, mapLenCodeLength[it.match.length()].first, mapCharLenCodeLength[aux].first)){
                // out.append("1");
                out.append(mapCharLenCodeLength[aux].first);
            // }
        //     else{
        //         out.append("0");
        //         out.append(mapLenCodeLength[it.match.length()-3].first);
        //         out.append(mapCharCodeLength[it.nextChar].first);

        // }
        
    }
    }
    std::bitset<3> bs = count;
    out.insert(0, bs.to_string());
    return out;
}


/*Chamar LZ77 ENCODE com flag encode = 1*/


void DeflatePart(   std::deque<Data> codeTriplesAux,
                    std::vector<uint16_t> bufferJump,
                    std::vector<uint16_t> bufferCharLen,
                    std::vector<USIZE> bufferChar, 
                    std::vector<USIZE> bufferLen,
                    std::vector<USIZE> bufferJumpHigh,
                    std::vector<USIZE> bufferJumpLow
                    ){
    /* 16 bits */
    std::vector<std::pair<uint16_t, long long>> pairJumpProb;    /*-*/
    std::vector<std::pair<uint16_t, long long>> pairCharLenProb; /*-*/
    std::priority_queue<nodeU16 *, std::vector<nodeU16 *>, compareU16> heapCharLen;
    std::priority_queue<nodeU16 *, std::vector<nodeU16 *>, compareU16> heapJump;
    std::vector<std::pair<uint16_t, int>> pairCharLenCodeLength;
    std::vector<int> charLenCodeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<std::pair<uint16_t, int>> pairJumpCodeLength;
    std::vector<int> JumpCodeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::unordered_map<uint16_t, std::pair<std::string, int>> mapCharLenCodeLength;
    std::unordered_map<uint16_t, std::pair<std::string, int>> mapJumpCodeLength;

    std::vector<std::pair<USIZE, long long>> pairJumpHighProb;    
    std::vector<std::pair<USIZE, long long>> pairJumpLowProb;    
    std::vector<std::pair<USIZE, long long>> pairCharProb; 
    std::vector<std::pair<USIZE, long long>> pairLenProb; 
    std::priority_queue<nodeChar *, std::vector<nodeChar *>, compareChar> heapChar;
    std::priority_queue<nodeChar *, std::vector<nodeChar *>, compareChar> heapJumpHigh;
    std::priority_queue<nodeChar *, std::vector<nodeChar *>, compareChar> heapJumpLow;
    std::priority_queue<nodeChar *, std::vector<nodeChar *>, compareChar> heapLen;
    std::vector<std::pair<USIZE, int>> pairJumpHighCodeLength;
    std::vector<std::pair<USIZE, int>> pairJumpLowCodeLength;
    std::vector<std::pair<USIZE, int>> pairCharCodeLength;
    std::vector<std::pair<USIZE, int>> pairLenCodeLength;
    std::vector<int> jumpHighCodeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<int> jumpLowCodeLengths  = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<int> charCodeLengths     = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<int> lenCodeLengths      = {0, 0, 0, 0, 0, 0, 0, 0, 0};
   
    std::unordered_map<USIZE, std::pair<std::string, int>> mapJumpHighCodeLength;
    std::unordered_map<USIZE, std::pair<std::string, int>> mapJumpLowCodeLength;
    std::unordered_map<USIZE, std::pair<std::string, int>> mapCharCodeLength;
    std::unordered_map<USIZE, std::pair<std::string, int>> mapLenCodeLength;

    pairCharLenProb = getFrequencyU16(bufferCharLen);
    pairJumpProb = getFrequencyU16(bufferJump);

    pairJumpHighProb = getFrequencyU8Vector(bufferJumpHigh);
    pairJumpLowProb = getFrequencyU8Vector(bufferJumpLow);
    pairCharProb = getFrequencyU8Vector(bufferChar);
    pairLenProb = getFrequencyU8Vector(bufferLen);


    struct nodeU16 *nLeftC16, *nRightC16, *nTopC16;
    for (int i = 0; i < pairCharLenProb.size(); i++)
    {
        heapCharLen.push(new nodeU16(pairCharLenProb[i].first, pairCharLenProb[i].second, true));
    }

    struct nodeU16 *nLeftOL, *nRightOL, *nTopOL;
    for (int i = 0; i < pairJumpProb.size(); i++)
    {
        heapJump.push(new nodeU16(pairJumpProb[i].first, pairJumpProb[i].second, true));
    }


    struct nodeChar *nLeftJH, *nRightJH, *nTopJH;
    for (int i = 0; i < pairJumpHighProb.size(); i++)
    {
        heapJumpHigh.push(new nodeChar(pairJumpHighProb[i].first, pairJumpHighProb[i].second, true));
    }

    struct nodeChar *nLeftJL, *nRightJL, *nTopJL;
    for (int i = 0; i < pairJumpLowProb.size(); i++)
    {
        heapJumpLow.push(new nodeChar(pairJumpLowProb[i].first, pairJumpLowProb[i].second, true));
    }
    
    struct nodeChar *nLeftC, *nRightC, *nTopC;
    for (int i = 0; i < pairCharProb.size(); i++)
    {
        heapChar.push(new nodeChar(pairCharProb[i].first, pairCharProb[i].second, true));
    }

    struct nodeChar *nLeftL, *nRightL, *nTopL;
    for (int i = 0; i < pairLenProb.size(); i++)
    {
        heapLen.push(new nodeChar(pairLenProb[i].first, pairLenProb[i].second, true));
    }



    while (heapCharLen.size() != 1)
    {
        nLeftC16 = heapCharLen.top();
        heapCharLen.pop();
        nRightC16 = heapCharLen.top();
        heapCharLen.pop();
        nTopC16 = new nodeU16((uint16_t)0x1f, nLeftC16->key_value + nRightC16->key_value, false);
        nTopC16->left = nLeftC16;
        nTopC16->right = nRightC16;
        heapCharLen.push(nTopC16);
    }

    while (heapJump.size() != 1)
    {
        nLeftOL = heapJump.top();
        heapJump.pop();
        nRightOL = heapJump.top();
        heapJump.pop();
        nTopOL = new nodeU16((uint16_t)0x1f, nLeftOL->key_value + nRightOL->key_value, false);
        nTopOL->left = nLeftOL;
        nTopOL->right = nRightOL;
        heapJump.push(nTopOL);
    }

    while (heapJumpHigh.size() != 1)
    {
        nLeftJH = heapJumpHigh.top();
        heapJumpHigh.pop();
        nRightJH = heapJumpHigh.top();
        heapJumpHigh.pop();
        nTopJH = new nodeChar((USIZE)0x1f, nLeftJH->key_value + nRightJH->key_value, false);
        nTopJH->left = nLeftJH;
        nTopJH->right = nRightJH;
        heapJumpHigh.push(nTopJH);
    }

    while (heapJumpLow.size() != 1)
    {
        nLeftJL = heapJumpLow.top();
        heapJumpLow.pop();
        nRightJL = heapJumpLow.top();
        heapJumpLow.pop();
        nTopJL = new nodeChar((USIZE)0x1f, nLeftJL->key_value + nRightJL->key_value, false);
        nTopJL->left = nLeftJL;
        nTopJL->right = nRightJL;
        heapJumpLow.push(nTopJL);
    }
    
    while (heapChar.size() != 1)
    {
        nLeftC = heapChar.top();
        heapChar.pop();
        nRightC = heapChar.top();
        heapChar.pop();
        nTopC = new nodeChar((USIZE)0x1f, nLeftC->key_value + nRightC->key_value, false);
        nTopC->left = nLeftC;
        nTopC->right = nRightC;
        heapChar.push(nTopC);
    }

    while (heapLen.size() != 1)
    {
        nLeftL = heapLen.top();
        heapLen.pop();
        nRightL = heapLen.top();
        heapLen.pop();
        nTopL = new nodeChar((USIZE)0x1f, nLeftL->key_value + nRightL->key_value, false);
        nTopL->left = nLeftL;
        nTopL->right = nRightL;
        heapLen.push(nTopL);
    }


    mapCodesU16(heapCharLen.top(), 0, pairCharLenCodeLength); /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
    mapCodesU16(heapJump.top(), 0, pairJumpCodeLength);       /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
    calcU16CodeLengths(pairCharLenCodeLength, charLenCodeLengths);
    calcU16CodeLengths(pairJumpCodeLength, JumpCodeLengths);
    buildU16Codes(pairJumpCodeLength, JumpCodeLengths, mapJumpCodeLength);
    buildU16Codes(pairCharLenCodeLength, charLenCodeLengths, mapCharLenCodeLength);

    mapCodesUSIZE(heapJumpHigh.top(), 0, pairJumpHighCodeLength); /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
    mapCodesUSIZE(heapJumpLow.top(), 0, pairJumpLowCodeLength);       /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
    mapCodesUSIZE(heapChar.top(), 0, pairCharCodeLength); /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
    mapCodesUSIZE(heapLen.top(), 0, pairLenCodeLength);       /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
    
    calcUSIZECodeLengths(pairJumpHighCodeLength, jumpHighCodeLengths);
    calcUSIZECodeLengths(pairJumpLowCodeLength, jumpLowCodeLengths);
    calcUSIZECodeLengths(pairCharCodeLength, charCodeLengths);
    calcUSIZECodeLengths(pairLenCodeLength, lenCodeLengths);

    buildUSIZECodes(pairJumpHighCodeLength, jumpHighCodeLengths, mapJumpHighCodeLength);
    buildUSIZECodes(pairJumpLowCodeLength, jumpLowCodeLengths, mapJumpLowCodeLength);
    buildUSIZECodes(pairCharCodeLength, charCodeLengths, mapCharCodeLength);
    buildUSIZECodes(pairLenCodeLength, lenCodeLengths, mapLenCodeLength);
    
    bitString += WriteDeflateBitString(
    codeTriplesAux,

    pairCharLenCodeLength,
    charLenCodeLengths,
    mapCharLenCodeLength,

    pairJumpCodeLength,
    JumpCodeLengths,
    mapJumpCodeLength,

    pairJumpHighCodeLength,
    jumpHighCodeLengths,
    mapJumpHighCodeLength,
    pairJumpLowCodeLength,
    jumpLowCodeLengths,
    mapJumpLowCodeLength,
    pairCharCodeLength,
    charCodeLengths,
    mapCharCodeLength,
    pairLenCodeLength,
    lenCodeLengths,
    mapLenCodeLength);
    return;
}


void DeflateEncode(std::string filenameIn, std::string filenameOut, int encode)
{
    /*LZ77 PART*/
    /*This will put the triples in the strBuffer*/
    std::deque<Data> codeTriples;
    std::deque<Data> codeTriplesAux;
    codeTriples = EncodeLZ77(filenameIn, filenameOut, 1);
    bitString.clear();
    std::vector<USIZE> bufferJumpHigh;
    std::vector<USIZE> bufferJumpLow;
    std::vector<USIZE> bufferChar;
    std::vector<USIZE> bufferLen;
    std::vector<uint16_t> bufferJump;
    std::vector<uint16_t> bufferCharLen;
    uint16_t aux = 0;
    int i =0;
    int j=0;
    while(i<codeTriples.size()){
        for (j=i; j-i<5; j++)
        {
            if(j==codeTriples.size()){
                break;
            }
            aux = codeTriples[j].nextChar;
            aux = aux << 8;
            if(codeTriples[j].match.length()==0){
                bufferLen.push_back(codeTriples[j].match.length());
                aux |= codeTriples[j].match.length();
            }else{
                bufferLen.push_back(codeTriples[j].match.length()-3);
                aux |= (codeTriples[j].match.length()-3);

            }
            bufferJumpHigh.push_back((codeTriples[j].offset >> 8));
            bufferJumpLow.push_back((codeTriples[j].offset) & MASK);
            bufferChar.push_back(codeTriples[j].nextChar);
            bufferJump.push_back(codeTriples[j].offset);
            bufferCharLen.push_back(aux);
            codeTriplesAux.emplace_back(codeTriples[j]);
        }
        i=j;

        DeflatePart(codeTriplesAux, bufferJump, bufferCharLen, bufferChar, bufferLen, bufferJumpHigh,  bufferJumpLow);
        codeTriplesAux.clear();
        bufferChar.clear();  
        bufferLen.clear();
        bufferJumpHigh.clear();
        bufferJumpLow.clear();
        bufferJump.clear();
        bufferCharLen.clear();

    }
    /*Huffman Part*/
       if (encode == 0)
    {
        writeEncodedFile(filenameOut);
    }
    else
    {
        std::cout << bitString.size() / 8 << std::endl;
    }
}

void DeflateDecode(std::string filenameIn, std::string filenameOut)
{
    readFileAsBinaryString(filenameIn);
    getWindowSize();
    std::string decoding;
    std::string decoded;
    std::vector<std::pair<USIZE, long long>> pairJumpProb;    /*-*/
    std::vector<std::pair<USIZE, long long>> pairCharLenProb; /*-*/
    std::priority_queue<nodeChar *, std::vector<nodeChar *>, compareChar> heapCharLen;
    std::priority_queue<nodeChar *, std::vector<nodeChar *>, compareChar> heapJump;
    std::vector<std::pair<USIZE, int>> pairCharLenCodeLength;
    std::vector<int> charLenCodeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<std::pair<USIZE, int>> pairJumpCodeLength;
    std::vector<int> JumpCodeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::unordered_map<USIZE, std::pair<std::string, int>> mapCharLenCodeLength;
    std::unordered_map<USIZE, std::pair<std::string, int>> mapJumpCodeLength;

    std::unordered_map<std::string, USIZE> mapCharLenCode;
    std::unordered_map<std::string, USIZE> mapJumpCode;

    char c;
    int strPointer = 3;
    int addedZeros = stoi(bitString.substr(0, 3), 0, 2);
    bitString.resize(bitString.size() - addedZeros);

    for (int i = 0; i < MAXCODESIZE; i++)
    {
        if (bitString[strPointer] == '1')
        {
            charLenCodeLengths[i] = stoi(bitString.substr(strPointer + 1, 12), 0, 2);
            strPointer += 13;
        }
        else
        {
            strPointer += 1;
        }
    }
    USIZE auxCharLen = 0;
    for (int j = 0; j < charLenCodeLengths.size(); j++)
    {
        for (int i = 0; i < charLenCodeLengths[j]; i++)
        {

            auxCharLen = stoll(bitString.substr(strPointer, 8 * sizeof(USIZE)), 0, 2);
            pairCharLenCodeLength.push_back(std::make_pair(auxCharLen, j + 1));
            strPointer += 8 * sizeof(USIZE);
        }
    }

    /*Agora pra Uint*/

    for (int i = 0; i < MAXCODESIZE; i++)
    {
        if (bitString[strPointer] == '1')
        {
            JumpCodeLengths[i] = stoi(bitString.substr(strPointer + 1, 12), 0, 2);
            strPointer += 13;
        }
        else
        {
            strPointer += 1;
        }
    }
    USIZE auxJump = 0;

    for (int j = 0; j < JumpCodeLengths.size(); j++)
    {
        for (int i = 0; i < JumpCodeLengths[j]; i++)
        {
            auxJump = stoll(bitString.substr(strPointer, 8 * sizeof(USIZE)), 0, 2);
            pairJumpCodeLength.push_back(std::make_pair(auxJump, j + 1));
            strPointer += 8 * sizeof(USIZE);
        }
    }

    buildUSIZECodes(pairCharLenCodeLength, charLenCodeLengths, mapCharLenCodeLength);
    buildUSIZECodes(pairJumpCodeLength, JumpCodeLengths, mapJumpCodeLength);

    for (std::unordered_map<USIZE, std::pair<std::string, int>>::iterator i = mapCharLenCodeLength.begin(); i != mapCharLenCodeLength.end(); ++i)
    {
        mapCharLenCode[i->second.first] = i->first;
    }

    for (std::unordered_map<USIZE, std::pair<std::string, int>>::iterator i = mapJumpCodeLength.begin(); i != mapJumpCodeLength.end(); ++i)
    {
        mapJumpCode[i->second.first] = i->first;
    }

    std::deque<USIZE> charLenDecoding;
    std::deque<USIZE> jumpdecoding;
    for (std::string::iterator it = bitString.begin() + strPointer; it != bitString.end(); it++)
    {
        decoding += *it;
        try
        {
            jumpdecoding.push_back(mapJumpCode.at(decoding));
            decoding.clear();
            for (std::string::iterator jt = it + 1; jt != bitString.end(); jt++)
            {
                decoding += *jt;
                try
                {
                    charLenDecoding.push_back(mapCharLenCode.at(decoding));
                    decoding.clear();
                    it = jt;
                    break;
                }
                catch (const std::out_of_range &e)
                {
                }
            }
        }
        catch (const std::out_of_range &e)
        {
        }
    }
    decoding.clear();

    std::string outString;
    std::string dict;
    int windowPointer = 0;
    int jump;
    int len;
    char nextChar;

    for (int k = 0; k < charLenDecoding.size(); k++)
    {

        nextChar = charLenDecoding[k] >> SHIFT;
        len = charLenDecoding[k] & MASK;
        jump = jumpdecoding[k];
        if(len!=0){
            len+=3;
        }
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
    writeDecodedFile(filenameOut, outString);
}