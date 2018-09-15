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

#define MAXCODESIZE 18
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

std::string WriteDeflateBitString(
    std::deque<Data> &codeTriples,

    std::vector<std::pair<uint16_t, int>> &pairCharLenCodeLength,
    std::vector<int> &charLenCodeLengths,
    std::unordered_map<uint16_t, std::pair<std::string, int>> &mapCharLenCodeLength,

    std::vector<std::pair<uint16_t, int>> &pairJumpCodeLength,
    std::vector<int> &jumpCodeLengths,
    std::unordered_map<uint16_t, std::pair<std::string, int>> &mapJumpCodeLength)
{

    std::string charLenTree = "";
    std::string jumpTree = "";
    std::string charLenCodeLengthsTree = "";
    std::string jumpCodeLengthsTree = "";

    std::string out = "";

    for (int i = 0; i < charLenCodeLengths.size(); i++)
    {
        if (charLenCodeLengths[i] == 0)
        {
            charLenTree += "0";
        }
        else
        {
            charLenTree += "1";
            charLenTree.append(decimalToBitString(charLenCodeLengths[i], 10));
        }
    }
    for (auto it : pairCharLenCodeLength)
    {
        charLenCodeLengthsTree.append(USIZEToBin(it.first));
    }

    for (int i = 0; i < jumpCodeLengths.size(); i++)
    {
        if (jumpCodeLengths[i] == 0)
        {
            jumpTree += "0";
        }
        else
        {
            jumpTree += "1";
            jumpTree.append(decimalToBitString(jumpCodeLengths[i], 10));
        }
    }

    for (auto it : pairJumpCodeLength)
    {
        jumpCodeLengthsTree.append(USIZEToBin(it.first));
    }

    std::cout << "\n\n";
    out.append(compareHistoryStrings(charLenTree, historyCharLenTree, histCharLenTreePointer));
    std::cout << " ";
    out.append(compareHistoryStrings(charLenCodeLengthsTree, historyCharLenCodes, histCharLenCodesPointer));
    std::cout << "\n";
    out.append(compareHistoryStrings(jumpTree, historyJumpTree, histJumpTreePointer));
    std::cout << " ";
    out.append(compareHistoryStrings(jumpCodeLengthsTree, historyJumpCodes, histJumpCodesPointer));
    std::cout << "\n";

    uint16_t aux;

    for (auto it : codeTriples)
    {
        aux = it.nextChar;
        aux = aux << 8;

        out.append(mapJumpCodeLength[it.offset].first);
        if (it.match.length() == 0)
        {
            aux |= it.match.length();
            out.append(mapCharLenCodeLength[aux].first);
        }
        else
        {
            aux |= (it.match.length() - 3);

            out.append(mapCharLenCodeLength[aux].first);
        }
    }
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
    std::vector<int> charLenCodeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::vector<std::pair<uint16_t, int>> pairJumpCodeLength;
    std::vector<int> JumpCodeLengths = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::unordered_map<uint16_t, std::pair<std::string, int>> mapCharLenCodeLength;
    std::unordered_map<uint16_t, std::pair<std::string, int>> mapJumpCodeLength;

    pairCharLenProb = getFrequencyU16(bufferCharLen);
    pairJumpProb = getFrequencyU16(bufferJump);

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

    mapCodesU16(heapCharLen.top(), 0, pairCharLenCodeLength); /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
    mapCodesU16(heapJump.top(), 0, pairJumpCodeLength);       /*Mapeia a arvore de huffman pra calcular o tamanho dos códigos*/
    calcU16CodeLengths(pairCharLenCodeLength, charLenCodeLengths);
    calcU16CodeLengths(pairJumpCodeLength, JumpCodeLengths);
    buildU16Codes(pairJumpCodeLength, JumpCodeLengths, mapJumpCodeLength);
    buildU16Codes(pairCharLenCodeLength, charLenCodeLengths, mapCharLenCodeLength);

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
    int i = 0;
    int j = 0;

    while (i < codeTriples.size())
    {
        for (j = i; j - i < 4; j++)
        {
            if (j == codeTriples.size())
            {
                break;
            }
            if (codeTriples[j].match.length() == 0)
            {
                bufferCharLen.push_back(codeTriples[j].nextChar);
            }
            else
            { /* Aqui separamos os Lengths para extra bits*/
                bufferCharLen.push_back(codeTriples[j].nextChar);
                if (codeTriples[j].match.length() >= 3 and codeTriples[j].match.length() <= 10)
                {
                    bufferCharLen.push_back(254 + codeTriples[j].match.length());
                }
                else if (codeTriples[j].match.length() >= 11 and codeTriples[j].match.length() <= 18)
                {

                    if (codeTriples[j].match.length() >= 11 and codeTriples[j].match.length() <= 12)
                    {
                        bufferCharLen.push_back(265);
                    }
                    else if (codeTriples[j].match.length() >= 13 and codeTriples[j].match.length() <= 14)
                    {
                        bufferCharLen.push_back(266);
                    }
                    else if (codeTriples[j].match.length() >= 15 and codeTriples[j].match.length() <= 16)
                    {
                        bufferCharLen.push_back(267);
                    }
                    else if (codeTriples[j].match.length() >= 17 and codeTriples[j].match.length() <= 18)
                    {
                        bufferCharLen.push_back(268);
                    }
                    else
                    {
                        exit(1);
                    }
                }
                else if (codeTriples[j].match.length() >= 19 and codeTriples[j].match.length() <= 34)
                {

                    if (codeTriples[j].match.length() >= 19 and codeTriples[j].match.length() <= 22)
                    {
                        bufferCharLen.push_back(269);
                    }
                    else if (codeTriples[j].match.length() >= 23 and codeTriples[j].match.length() <= 26)
                    {
                        bufferCharLen.push_back(270);
                    }
                    else if (codeTriples[j].match.length() >= 27 and codeTriples[j].match.length() <= 30)
                    {
                        bufferCharLen.push_back(271);
                    }
                    else if (codeTriples[j].match.length() >= 31 and codeTriples[j].match.length() <= 34)
                    {
                        bufferCharLen.push_back(272);
                    }
                    else
                    {
                        exit(1);
                    }
                }
                else if (codeTriples[j].match.length() >= 35 and codeTriples[j].match.length() <= 66)
                {

                    if (codeTriples[j].match.length() >= 35 and codeTriples[j].match.length() <= 42)
                    {
                        bufferCharLen.push_back(273);
                    }
                    else if (codeTriples[j].match.length() >= 43 and codeTriples[j].match.length() <= 50)
                    {
                        bufferCharLen.push_back(274);
                    }
                    else if (codeTriples[j].match.length() >= 51 and codeTriples[j].match.length() <= 58)
                    {
                        bufferCharLen.push_back(275);
                    }
                    else if (codeTriples[j].match.length() >= 59 and codeTriples[j].match.length() <= 66)
                    {
                        bufferCharLen.push_back(276);
                    }
                    else
                    {
                        exit(1);
                    }
                }
                else if (codeTriples[j].match.length() >= 67 and codeTriples[j].match.length() <= 130)
                {

                    if (codeTriples[j].match.length() >= 67 and codeTriples[j].match.length() <= 82)
                    {
                        bufferCharLen.push_back(277);
                    }
                    else if (codeTriples[j].match.length() >= 83 and codeTriples[j].match.length() <= 98)
                    {
                        bufferCharLen.push_back(278);
                    }
                    else if (codeTriples[j].match.length() >= 99 and codeTriples[j].match.length() <= 114)
                    {
                        bufferCharLen.push_back(279);
                    }
                    else if (codeTriples[j].match.length() >= 115 and codeTriples[j].match.length() <= 130)
                    {
                        bufferCharLen.push_back(280);
                    }
                    else
                    {
                        exit(1);
                    }
                }
                else if (codeTriples[j].match.length() >= 131 and codeTriples[j].match.length() <= 257)
                {

                    if (codeTriples[j].match.length() >= 131 and codeTriples[j].match.length() <= 162)
                    {
                        bufferCharLen.push_back(281);
                    }
                    else if (codeTriples[j].match.length() >= 163 and codeTriples[j].match.length() <= 194)
                    {
                        bufferCharLen.push_back(282);
                    }
                    else if (codeTriples[j].match.length() >= 195 and codeTriples[j].match.length() <= 226)
                    {
                        bufferCharLen.push_back(283);
                    }
                    else if (codeTriples[j].match.length() >= 227 and codeTriples[j].match.length() <= 257)
                    {
                        bufferCharLen.push_back(284);
                    }
                    else
                    {
                        exit(1);
                    }
                }
                else if (codeTriples[j].match.length() == 258)
                {
                    bufferCharLen.push_back(285);
                }
                else
                {
                    exit(1);
                }

                /*Aqui termina os codeLengths das matchs*/

                if (codeTriples[j].offset >= 1 and codeTriples[j].offset <= 4)
                {
                    bufferJump.push_back(codeTriples[j].offset - 1);
                }
                else if (codeTriples[j].offset >= 5 and codeTriples[j].offset <= 6)
                {
                    bufferJump.push_back(4);
                }
                else if (codeTriples[j].offset >= 7 and codeTriples[j].offset <= 8)
                {
                    bufferJump.push_back(5);
                }
                else if (codeTriples[j].offset >= 9 and codeTriples[j].offset <= 12)
                {
                    bufferJump.push_back(6);
                }
                else if (codeTriples[j].offset >= 13 and codeTriples[j].offset <= 16)
                {
                    bufferJump.push_back(7);
                }
                else if (codeTriples[j].offset >= 17 and codeTriples[j].offset <= 24)
                {
                    bufferJump.push_back(8);
                }
                else if (codeTriples[j].offset >= 25 and codeTriples[j].offset <= 32)
                {
                    bufferJump.push_back(9);
                }
                else if (codeTriples[j].offset >= 33 and codeTriples[j].offset <= 48)
                {
                    bufferJump.push_back(10);
                }
                else if (codeTriples[j].offset >= 49 and codeTriples[j].offset <= 64)
                {
                    bufferJump.push_back(11);
                }
                else if (codeTriples[j].offset >= 65 and codeTriples[j].offset <= 96)
                {
                    bufferJump.push_back(12);
                }
                else if (codeTriples[j].offset >= 97 and codeTriples[j].offset <= 128)
                {
                    bufferJump.push_back(13);
                }
                else if (codeTriples[j].offset >= 129 and codeTriples[j].offset <= 192)
                {
                    bufferJump.push_back(14);
                }
                else if (codeTriples[j].offset >= 193 and codeTriples[j].offset <= 256)
                {
                    bufferJump.push_back(15);
                }
                else if (codeTriples[j].offset >= 257 and codeTriples[j].offset <= 384)
                {
                    bufferJump.push_back(16);
                }
                else if (codeTriples[j].offset >= 385 and codeTriples[j].offset <= 512)
                {
                    bufferJump.push_back(17);
                }
                else if (codeTriples[j].offset >= 513 and codeTriples[j].offset <= 768)
                {
                    bufferJump.push_back(18);
                }
                else if (codeTriples[j].offset >= 769 and codeTriples[j].offset <= 1024)
                {
                    bufferJump.push_back(19);
                }
                else if (codeTriples[j].offset >= 1025 and codeTriples[j].offset <= 1536)
                {
                    bufferJump.push_back(20);
                }
                else if (codeTriples[j].offset >= 1537 and codeTriples[j].offset <= 2048)
                {
                    bufferJump.push_back(21);
                }
                else if (codeTriples[j].offset >= 2049 and codeTriples[j].offset <= 3072)
                {
                    bufferJump.push_back(22);
                }
                else if (codeTriples[j].offset >= 3073 and codeTriples[j].offset <= 4096)
                {
                    bufferJump.push_back(23);
                }
                else if (codeTriples[j].offset >= 4097 and codeTriples[j].offset <= 6144)
                {
                    bufferJump.push_back(24);
                }
                else if (codeTriples[j].offset >= 6145 and codeTriples[j].offset <= 8192)
                {
                    bufferJump.push_back(25);
                }
                else if (codeTriples[j].offset >= 8193 and codeTriples[j].offset <= 12288)
                {
                    bufferJump.push_back(26);
                }
                else if (codeTriples[j].offset >= 12289 and codeTriples[j].offset <= 16384)
                {
                    bufferJump.push_back(27);
                }
                else if (codeTriples[j].offset >= 16385 and codeTriples[j].offset <= 24576)
                {
                    bufferJump.push_back(28);
                }
                else if (codeTriples[j].offset >= 24577 and codeTriples[j].offset <= 32768)
                {
                    bufferJump.push_back(29);
                }
                else
                {
                    exit(1);
                }
            }
            codeTriplesAux.emplace_back(codeTriples[j]);
        }
        i = j;

        DeflatePart(codeTriplesAux, bufferJump, bufferCharLen);
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