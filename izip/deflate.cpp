#include "deflate.h"
#include <cstdlib>
#include <algorithm>
#include <queue>
#include <bitset>

/*1- Como aplicar Burrows wheeler (antes e depois, só antes ou só depois)*/
/*2- Como splitar, quantos blocos, a partir de qual tamanho*/
/*3- lembrar do rle definido*/

std::vector<std::string> historyCharLenTree(4, "");
std::vector<std::string> historyJumpTree(4, "");
std::vector<std::string> historyCharLenCodes(4, "");
std::vector<std::string> historyJumpCodes(4, "");
int histCharLenTreePointer = 0;
int histCharLenCodesPointer = 0;
int histJumpTreePointer = 0;
int histJumpCodesPointer = 0;

#define MAXCODESIZE 180
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
        if (len == 0)
            len = 1;
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
        if (len == 0)
            len = 1;
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
    count = 0;
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
    count = 0;
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

int checkBiggestCode(std::string firstCode, std::string secondCode, std::string thirdCode)
{

    if (firstCode.length() + secondCode.length() > thirdCode.length())
    {
        std::cout << firstCode.length() + secondCode.length() << " " << thirdCode.length() << "\n";
        return 1;
    }
    else
    {
        std::cout << firstCode.length() + secondCode.length() << " " << thirdCode.length() << "\n";
        return 0;
    }
}

std::string compareHistoryStrings(std::string firstString, std::vector<std::string> &historyString, int &pointer)
{
    int cont = 0;
    std::string found;
    for (; cont < historyString.size(); cont++)
    {
        if (firstString.compare(historyString[cont]) == 0)
        {
            break;
        }
    }
    if (cont != 4)
    {
        found += "1";
        found.append(decimalToBitString(cont, 2));
        std::cout << found;
        return (found);
    }
    else
    {

        historyString[pointer % 4] = firstString;
        pointer++;
        std::cout << "0" + firstString;
        return "0" + firstString;
    }
}

DeflateData checkLenAndNLen(USIZE matchSize, USIZE offset)
{

    USIZE match = 0;
    USIZE offsetlen = 0;
    USIZE fInMatchRange = 0;
    USIZE fInOffsetRange = 0;
    USIZE matchExtraBits = 0;
    USIZE offsetExtraBits = 0;
    if (matchSize >= 3 and matchSize <= 10)
    {
        match = 254 + matchSize;
        fInMatchRange = 3;
        matchExtraBits = 0;
    }
    else if (matchSize >= 11 and matchSize <= 18)
    {
        matchExtraBits = 1;

        if (matchSize >= 11 and matchSize <= 12)
        {
            match = 265;
            fInMatchRange = 11;
        }
        else if (matchSize >= 13 and matchSize <= 14)
        {
            match = 266;
            fInMatchRange = 13;
        }
        else if (matchSize >= 15 and matchSize <= 16)
        {
            match = 267;
            fInMatchRange = 15;
        }
        else if (matchSize >= 17 and matchSize <= 18)
        {
            match = 268;
            fInMatchRange = 17;
        }
        else
        {
            exit(1);
        }
    }
    else if (matchSize >= 19 and matchSize <= 34)
    {
        matchExtraBits = 2;

        if (matchSize >= 19 and matchSize <= 22)
        {
            match = 269;
            fInMatchRange = 19;
        }
        else if (matchSize >= 23 and matchSize <= 26)
        {
            match = 270;
            fInMatchRange = 23;
        }
        else if (matchSize >= 27 and matchSize <= 30)
        {
            match = 271;
            fInMatchRange = 27;
        }
        else if (matchSize >= 31 and matchSize <= 34)
        {
            match = 272;
            fInMatchRange = 31;
        }
        else
        {
            exit(1);
        }
    }
    else if (matchSize >= 35 and matchSize <= 66)
    {
        matchExtraBits = 3;

        if (matchSize >= 35 and matchSize <= 42)
        {
            match = 273;
            fInMatchRange = 35;
        }
        else if (matchSize >= 43 and matchSize <= 50)
        {
            match = 274;
            fInMatchRange = 43;
        }
        else if (matchSize >= 51 and matchSize <= 58)
        {
            match = 275;
            fInMatchRange = 51;
        }
        else if (matchSize >= 59 and matchSize <= 66)
        {
            match = 276;
            fInMatchRange = 59;
        }
        else
        {
            exit(1);
        }
    }
    else if (matchSize >= 67 and matchSize <= 130)
    {
        matchExtraBits = 4;

        if (matchSize >= 67 and matchSize <= 82)
        {
            match = 277;
            fInMatchRange = 67;
        }
        else if (matchSize >= 83 and matchSize <= 98)
        {
            match = 278;
            fInMatchRange = 83;
        }
        else if (matchSize >= 99 and matchSize <= 114)
        {
            match = 279;
            fInMatchRange = 99;
        }
        else if (matchSize >= 115 and matchSize <= 130)
        {
            match = 280;
            fInMatchRange = 115;
        }
        else
        {
            exit(1);
        }
    }
    else if (matchSize >= 131 and matchSize <= 257)
    {
        matchExtraBits = 5;

        if (matchSize >= 131 and matchSize <= 162)
        {
            match = 281;
            fInMatchRange = 131;
        }
        else if (matchSize >= 163 and matchSize <= 194)
        {
            match = 282;
            fInMatchRange = 163;
        }
        else if (matchSize >= 195 and matchSize <= 226)
        {
            match = 283;
            fInMatchRange = 195;
        }
        else if (matchSize >= 227 and matchSize <= 257)
        {
            match = 284;
            fInMatchRange = 227;
        }
        else
        {
            exit(1);
        }
    }
    else if (matchSize == 258)
    {
        matchExtraBits = 0;
        match = 285;
    }
    else
    {
        exit(1);
    }

    /*Aqui termina os codeLengths das matchs*/

    if (offset >= 1 and offset <= 4)
    {
        offsetlen = offset - 1;
        fInOffsetRange = 1;
    }
    else if (offset >= 5 and offset <= 6)
    {
        offsetlen = 4;
        offsetExtraBits = 1;
        fInOffsetRange = 5;
    }
    else if (offset >= 7 and offset <= 8)
    {
        offsetlen = 5;
        offsetExtraBits = 1;
        fInOffsetRange = 7;
    }
    else if (offset >= 9 and offset <= 12)
    {
        offsetlen = 6;
        offsetExtraBits = 2;

        fInOffsetRange = 9;
    }
    else if (offset >= 13 and offset <= 16)
    {
        offsetlen = 7;
        offsetExtraBits = 2;
        fInOffsetRange = 13;
    }
    else if (offset >= 17 and offset <= 24)
    {
        offsetlen = 8;
        offsetExtraBits = 3;
        fInOffsetRange = 17;
    }
    else if (offset >= 25 and offset <= 32)
    {
        offsetlen = 9;
        offsetExtraBits = 3;
        fInOffsetRange = 25;
    }
    else if (offset >= 33 and offset <= 48)
    {
        offsetlen = 10;
        offsetExtraBits = 4;
        fInOffsetRange = 33;
    }
    else if (offset >= 49 and offset <= 64)
    {
        offsetlen = 11;
        offsetExtraBits = 4;
        fInOffsetRange = 49;
    }
    else if (offset >= 65 and offset <= 96)
    {
        offsetlen = 12;
        offsetExtraBits = 5;
        fInOffsetRange = 65;
    }
    else if (offset >= 97 and offset <= 128)
    {
        offsetlen = 13;
        offsetExtraBits = 5;
        fInOffsetRange = 97;
    }
    else if (offset >= 129 and offset <= 192)
    {
        offsetlen = 14;
        offsetExtraBits = 6;
        fInOffsetRange = 129;
    }
    else if (offset >= 193 and offset <= 256)
    {
        offsetlen = 15;
        offsetExtraBits = 6;
        fInOffsetRange = 193;
    }
    else if (offset >= 257 and offset <= 384)
    {
        offsetlen = 16;
        offsetExtraBits = 7;
        fInOffsetRange = 257;
    }
    else if (offset >= 385 and offset <= 512)
    {
        offsetlen = 17;
        offsetExtraBits = 7;
        fInOffsetRange = 385;
    }
    else if (offset >= 513 and offset <= 768)
    {
        offsetlen = 18;
        offsetExtraBits = 8;
        fInOffsetRange = 513;
    }
    else if (offset >= 769 and offset <= 1024)
    {
        offsetlen = 19;
        offsetExtraBits = 8;
        fInOffsetRange = 769;
    }
    else if (offset >= 1025 and offset <= 1536)
    {
        offsetlen = 20;
        offsetExtraBits = 9;
        fInOffsetRange = 1025;
    }
    else if (offset >= 1537 and offset <= 2048)
    {
        offsetlen = 21;
        offsetExtraBits = 9;
        fInOffsetRange = 1537;
    }
    else if (offset >= 2049 and offset <= 3072)
    {
        offsetlen = 22;
        offsetExtraBits = 10;
        fInOffsetRange = 2049;
    }
    else if (offset >= 3073 and offset <= 4096)
    {
        offsetlen = 23;
        offsetExtraBits = 10;
        fInOffsetRange = 3073;
    }
    else if (offset >= 4097 and offset <= 6144)
    {
        offsetlen = 24;
        offsetExtraBits = 11;
        fInOffsetRange = 4097;
    }
    else if (offset >= 6145 and offset <= 8192)
    {
        offsetlen = 25;
        offsetExtraBits = 11;
        fInOffsetRange = 6145;
    }
    else if (offset >= 8193 and offset <= 12288)
    {
        offsetlen = 26;
        offsetExtraBits = 12;
        fInOffsetRange = 8193;
    }
    else if (offset >= 12289 and offset <= 16384)
    {
        offsetlen = 27;
        offsetExtraBits = 12;
        fInOffsetRange = 12289;
    }
    else if (offset >= 16385 and offset <= 24576)
    {
        offsetlen = 28;
        offsetExtraBits = 13;
        fInOffsetRange = 16385;
    }
    else if (offset >= 24577 and offset <= 32768)
    {
        offsetlen = 29;
        offsetExtraBits = 13;
        fInOffsetRange = 24577;
    }
    else
    {
        exit(1);
    }

    return DeflateData(offsetlen, match, fInMatchRange, fInOffsetRange, matchExtraBits, offsetExtraBits);
}

std::string WriteDeflateBitString(
    std::deque<Data> &codeTriples,

    std::vector<std::pair<uint16_t, int>> &pairCharLenCodeLength,
    std::vector<int> &charLenCodeLengths,
    std::unordered_map<uint16_t, std::pair<std::string, int>> &mapCharLenCodeLength,

    std::vector<std::pair<uint16_t, int>> &pairJumpCodeLength,
    std::vector<int> &jumpCodeLengths,
    std::unordered_map<uint16_t, std::pair<std::string, int>> &mapJumpCodeLength)
{
    std::string out = "";

 for (int i = 0; i < charLenCodeLengths.size(); i++)
    {
        if (charLenCodeLengths[i] == 0)
        {
            out += "0";
        }
        else
        {
            out += "1";
            out.append(decimalToBitString(charLenCodeLengths[i], 12));
        }
    }

    for (auto it : pairCharLenCodeLength)
    {
        out.append(USIZEToBin(it.first));
    }

    for (int i = 0; i < jumpCodeLengths.size(); i++)
    {
        if (jumpCodeLengths[i] == 0)
        {
            out += "0";
        }
        else
        {
            out += "1";
            out.append(decimalToBitString(jumpCodeLengths[i], 12));
        }
    }

    for (auto it : pairJumpCodeLength)
    {
        out.append(USIZEToBin(it.first));
    }

    uint16_t aux;
    USIZE diffMatchLen = 0;
    USIZE diffOffset = 0;
    DeflateData lenNLen(0, 0, 0, 0, 0, 0);
    for (auto it : codeTriples)
    {

        if (it.match.size() != 0)
        {
            lenNLen = checkLenAndNLen(it.match.length(), it.offset);
            out.append(mapCharLenCodeLength[lenNLen.matchLen].first);
            if (lenNLen.matchExtraBits > 0)
            {
                out.append(decimalToBitString(it.match.length() - lenNLen.firstInMatchRange, lenNLen.matchExtraBits));
            }
            out.append(mapJumpCodeLength[lenNLen.offset].first);
            if (lenNLen.offsetExtraBits > 0)
            {
                out.append(decimalToBitString(it.offset - lenNLen.firstInOffsetRange, lenNLen.offsetExtraBits));
            }
        }
        out.append(mapCharLenCodeLength[it.nextChar].first);
    }
    // std::cout<<out<<"\n\n";
    return out;
}

/*Chamar LZ77 ENCODE com flag encode = 1*/

void DeflatePart(std::deque<Data> codeTriplesAux,
                 std::vector<uint16_t> bufferJump,
                 std::vector<uint16_t> bufferCharLen)
{
    /* 16 bits */
    std::vector<std::pair<uint16_t, long long>> pairJumpProb;    /*-*/
    std::vector<std::pair<uint16_t, long long>> pairCharLenProb; /*-*/
    std::priority_queue<nodeU16 *, std::vector<nodeU16 *>, compareU16> heapCharLen;
    std::priority_queue<nodeU16 *, std::vector<nodeU16 *>, compareU16> heapJump;
    std::vector<std::pair<uint16_t, int>> pairCharLenCodeLength;
    std::vector<int> charLenCodeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<std::pair<uint16_t, int>> pairJumpCodeLength;
    std::vector<int> JumpCodeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::unordered_map<uint16_t, std::pair<std::string, int>> mapCharLenCodeLength;
    std::unordered_map<uint16_t, std::pair<std::string, int>> mapJumpCodeLength;

    if (bufferCharLen.size() > 0)
    {
        pairCharLenProb = getFrequencyU16(bufferCharLen);
        struct nodeU16 *nLeftC16, *nRightC16, *nTopC16;
        for (int i = 0; i < pairCharLenProb.size(); i++)
        {
            heapCharLen.push(new nodeU16(pairCharLenProb[i].first, pairCharLenProb[i].second, true));
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

        mapCodesU16(heapCharLen.top(), 0, pairCharLenCodeLength); /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
        calcU16CodeLengths(pairCharLenCodeLength, charLenCodeLengths);
        buildU16Codes(pairCharLenCodeLength, charLenCodeLengths, mapCharLenCodeLength);

    //     for(auto it: mapCharLenCodeLength){
    //         std::cout<<it.first << " " <<it.second.first<<"\n";
    //     }
    // std::cout<<"C";
    // for(auto it: charLenCodeLengths){
    //     std::cout<<it<<" ";
    // }
    // std::cout<<"\n\n";
    }
    // std::cout<<"\n\n";

    if (bufferJump.size() > 0)
    {
        pairJumpProb = getFrequencyU16(bufferJump);

        struct nodeU16 *nLeftOL, *nRightOL, *nTopOL;
        for (int i = 0; i < pairJumpProb.size(); i++)
        {
            heapJump.push(new nodeU16(pairJumpProb[i].first, pairJumpProb[i].second, true));
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

        mapCodesU16(heapJump.top(), 0, pairJumpCodeLength); /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
        calcU16CodeLengths(pairJumpCodeLength, JumpCodeLengths);
        buildU16Codes(pairJumpCodeLength, JumpCodeLengths, mapJumpCodeLength);
    // std::cout<<"J";

    // for(auto it: JumpCodeLengths){
    //     std::cout<<it<<" ";
    // }
    // std::cout<<"\n\n";

    }
    bitString += WriteDeflateBitString(
        codeTriplesAux,

        pairCharLenCodeLength,
        charLenCodeLengths,
        mapCharLenCodeLength,

        pairJumpCodeLength,
        JumpCodeLengths,
        mapJumpCodeLength);

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
    std::vector<uint16_t> bufferJump;
    std::vector<uint16_t> bufferCharLen;
    uint16_t aux = 0;
    DeflateData lenNLen(0, 0, 0, 0, 0, 0);
    int i = 0;
    int j = 0;

    while (i < codeTriples.size())
    {
        for (j = i; j - i < 50000000; j++)
        {
            if (j == codeTriples.size())
            {
                break;
            }
            if (codeTriples[j].match.length() != 0)
            {
                lenNLen = checkLenAndNLen(codeTriples[j].match.length(), codeTriples[j].offset);
                bufferCharLen.push_back(lenNLen.matchLen);
                bufferJump.push_back(lenNLen.offset);
            }

            bufferCharLen.push_back(codeTriples[j].nextChar);

            codeTriplesAux.emplace_back(codeTriples[j]);
        }
        i = j;

        DeflatePart(codeTriplesAux, bufferJump, bufferCharLen);
        lenNLen = {0, 0, 0, 0, 0, 0};
        codeTriplesAux.clear();
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
        if (len != 0)
        {
            len += 3;
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