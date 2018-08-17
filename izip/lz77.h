#ifndef IZIP_LZ77_H_
#define IZIP_LZ77_H_
#include "util.h"
#include <deque>
extern int DICTSIZE;
extern int LOOKAHEADSIZE;
extern int DICTBITS;
extern int LOOKBITS;
/*OBS: When encoding or decoding the DICTIONARY size and the LOOKAHEAD size must be passed*/

struct Data
{
    size_t offset;
    std::string match;
    uint8_t nextChar;
    size_t flagFull= 0;
    Data(size_t offset, std::string match, uint8_t nextChar, size_t flagFull) : offset(offset), match(match), nextChar(nextChar), flagFull(flagFull) {}
};

class Dictionary{
    /*The dictionary is responsible to get the biggest match in the dictionary*/
    public:
        std::string dictionary;
        Dictionary();
        void updateDict(size_t offset);
        size_t matchSz = 0;
        size_t dpb = 0; /*Dictionary pointer to begin of Dictionary*/
        size_t dpe = 0; /*Dictionary pointer to end of Dictionary*/
        void findBestMatch(int lpe, int lpb);/*This function has to return the Data to the lookahead*/
        void addTriplas(size_t offset, std::string match, char nChar, int flag);
        std::deque<Data> triplas;
};

class Lookahead{
    
    public:
        std::string lookahead;
        Lookahead(int filesize);
        size_t lpb=0; /*lookahead pointer to begin of lookahead*/
        size_t lpe = 0; /*lookahead pointer to end of lookahead*/
        void updateLook(size_t offset);
};

/* Write the final bitString of the LZ77 in case this is the option we choose*/
void writeLZ77BitString(int offset, std::string match, uint8_t nextChar);
/* Encode LZ77 Following the algorithm*/
/* 0 for LZ77 output file*/
/* 1 for getting the triples in the filebuffer*/
std::deque<Data> EncodeLZ77(std::string filenameIn, std::string filenameOut, int encode = 0);
/* Decode LZ77 Following the specifications*/
void DecodeLZ77(std::string filenameIn, std::string filenameOut);

/* Gets the dictionary size and the lookahead size as input from the user*/
void getWindowSize();

/* The boyer moore algorithm is from https://en.m.wikipedia.org/wiki/Boyer%E2%80%93Moore_string_search_algorithm*/
// delta1 table: delta1[c] contains the distance between the last
// character of pat and the rightmost occurrence of c in pat.
// If c does not occur in pat, then delta1[c] = patlen.
// If c is at string[i] and c != pat[patlen-1], we can
// safely shift i over by delta1[c], which is the minimum distance
// needed to shift pat forward to get string[i] lined up
// with some character in pat.
// this algorithm runs in alphabet_len+patlen time.
void make_delta1(int *delta1, uint8_t *pat, int32_t patlen);
// true if the suffix of word starting from word[pos] is a prefix
// of word
int is_prefix(uint8_t *word, int wordlen, int pos);
// length of the longest suffix of word ending on word[pos].
// suffix_length("dddbcabc", 8, 4) = 2
int suffix_length(uint8_t *word, int wordlen, int pos);
// delta2 table: given a mismatch at pat[pos], we want to align
// with the next possible full match could be based on what we
// know about pat[pos+1] to pat[patlen-1].
//
// In case 1:
// pat[pos+1] to pat[patlen-1] does not occur elsewhere in pat,
// the next plausible match starts at or after the mismatch.
// If, within the substring pat[pos+1 .. patlen-1], lies a prefix
// of pat, the next plausible match is here (if there are multiple
// prefixes in the substring, pick the longest). Otherwise, the
// next plausible match starts past the character aligned with
// pat[patlen-1].
//
// In case 2:
// pat[pos+1] to pat[patlen-1] does occur elsewhere in pat. The
// mismatch tells us that we are not looking at the end of a match.
// We may, however, be looking at the middle of a match.
//
// The first loop, which takes care of case 1, is analogous to
// the KMP table, adapted for a 'backwards' scan order with the
// additional restriction that the substrings it considers as
// potential prefixes are all suffixes. In the worst case scenario
// pat consists of the same letter repeated, so every suffix is
// a prefix. This loop alone is not sufficient, however:
// Suppose that pat is "ABYXCDBYX", and text is ".....ABYXCDEYX".
// We will match X, Y, and find B != E. There is no prefix of pat
// in the suffix "YX", so the first loop tells us to skip forward
// by 9 characters.
// Although superficially similar to the KMP table, the KMP table
// relies on information about the beginning of the partial match
// that the BM algorithm does not have.
//
// The second loop addresses case 2. Since suffix_length may not be
// unique, we want to take the minimum value, which will tell us
// how far away the closest potential match is.
void make_delta2(int *delta2, uint8_t *pat, int32_t patlen);

/*The Boyer moore was modified to return the position where the match occurs*/
std::pair<uint8_t *, int> boyer_moore(uint8_t *string, uint32_t stringlen, uint8_t *pat, uint32_t patlen);
#endif
