/*
Copyright 2011 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Author: lode.vandevenne@gmail.com (Lode Vandevenne)
Author: jyrki.alakuijala@gmail.com (Jyrki Alakuijala)
*/

/*
Functions for basic LZ77 compression and utilities for the "squeeze" LZ77
compression.
*/

#ifndef ZOPFLI_LZ77_H_
#define ZOPFLI_LZ77_H_

#include <stdlib.h>

#include "cache.h"
#include "hash.h"
#include "zopfli.h"

/*
Stores lit/length and dist pairs for LZ77.
Parameter litlens: Contains the literal symbols or length values.
Parameter dists: Contains the distances. A value is 0 to indicate that there is
no dist and the corresponding litlens value is a literal instead of a length.
Parameter size: The size of both the litlens and dists arrays.
The memory can best be managed by using ZipfliInitLZ77Store to initialize it,
ZipfliCleanLZ77Store to destroy it, and ZipfliStoreLitLenDist to append values.

*/
typedef struct ZipfliLZ77Store {
  unsigned short* litlens;  /* Lit or len. */
  unsigned short* dists;  /* If 0: indicates literal in corresponding litlens,
      if > 0: length in corresponding litlens, this is the distance. */
  size_t size;

  const unsigned char* data;  /* original data */
  size_t* pos;  /* position in data where this LZ77 command begins */

  unsigned short* ll_symbol;
  unsigned short* d_symbol;

  /* Cumulative histograms wrapping around per chunk. Each chunk has the amount
  of distinct symbols as length, so using 1 value per LZ77 symbol, we have a
  precise histogram at every N symbols, and the rest can be calculated by
  looping through the actual symbols of this chunk. */
  size_t* ll_counts;
  size_t* d_counts;
} ZipfliLZ77Store;

void ZipfliInitLZ77Store(const unsigned char* data, ZipfliLZ77Store* store);
void ZipfliCleanLZ77Store(ZipfliLZ77Store* store);
void ZipfliCopyLZ77Store(const ZipfliLZ77Store* source, ZipfliLZ77Store* dest);
void ZipfliStoreLitLenDist(unsigned short length, unsigned short dist,
                           size_t pos, ZipfliLZ77Store* store);
void ZipfliAppendLZ77Store(const ZipfliLZ77Store* store,
                           ZipfliLZ77Store* target);
/* Gets the amount of raw bytes that this range of LZ77 symbols spans. */
size_t ZipfliLZ77GetByteRange(const ZipfliLZ77Store* lz77,
                              size_t lstart, size_t lend);
/* Gets the histogram of lit/len and dist symbols in the given range, using the
cumulative histograms, so faster than adding one by one for large range. Does
not add the one end symbol of value 256. */
void ZipfliLZ77GetHistogram(const ZipfliLZ77Store* lz77,
                            size_t lstart, size_t lend,
                            size_t* ll_counts, size_t* d_counts);

/*
Some state information for compressing a block.
This is currently a bit under-used (with mainly only the longest match cache),
but is kept for easy future expansion.
*/
typedef struct ZipfliBlockState {
  const ZipfliOptions* options;

#ifdef ZOPFLI_LONGEST_MATCH_CACHE
  /* Cache for length/distance pairs found so far. */
  ZipfliLongestMatchCache* lmc;
#endif

  /* The start (inclusive) and end (not inclusive) of the current block. */
  size_t blockstart;
  size_t blockend;
} ZipfliBlockState;

void ZipfliInitBlockState(const ZipfliOptions* options,
                          size_t blockstart, size_t blockend, int add_lmc,
                          ZipfliBlockState* s);
void ZipfliCleanBlockState(ZipfliBlockState* s);

/*
Finds the longest match (length and corresponding distance) for LZ77
compression.
Even when not using "sublen", it can be more efficient to provide an array,
because only then the caching is used.
array: the data
pos: position in the data to find the match for
size: size of the data
limit: limit length to maximum this value (default should be 258). This allows
    finding a shorter dist for that length (= less extra bits). Must be
    in the range [ZOPFLI_MIN_MATCH, ZOPFLI_MAX_MATCH].
sublen: output array of 259 elements, or null. Has, for each length, the
    smallest distance required to reach this length. Only 256 of its 259 values
    are used, the first 3 are ignored (the shortest length is 3. It is purely
    for convenience that the array is made 3 longer).
*/
void ZipfliFindLongestMatch(
    ZipfliBlockState *s, const ZipfliHash* h, const unsigned char* array,
    size_t pos, size_t size, size_t limit,
    unsigned short* sublen, unsigned short* distance, unsigned short* length);

/*
Verifies if length and dist are indeed valid, only used for assertion.
*/
void ZipfliVerifyLenDist(const unsigned char* data, size_t datasize, size_t pos,
                         unsigned short dist, unsigned short length);

/*
Does LZ77 using an algorithm similar to gzip, with lazy matching, rather than
with the slow but better "squeeze" implementation.
The result is placed in the ZipfliLZ77Store.
If instart is larger than 0, it uses values before instart as starting
dictionary.
*/
void ZipfliLZ77Greedy(ZipfliBlockState* s, const unsigned char* in,
                      size_t instart, size_t inend,
                      ZipfliLZ77Store* store, ZipfliHash* h);

#endif  /* ZOPFLI_LZ77_H_ */
