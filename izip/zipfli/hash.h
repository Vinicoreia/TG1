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
The hash for ZipfliFindLongestMatch of lz77.c.
*/

#ifndef ZIPFLI_HASH_H_
#define ZIPFLI_HASH_H_

#include "util.h"

typedef struct ZipfliHash {
  int* head;  /* Hash value to index of its most recent occurrence. */
  unsigned short* prev;  /* Index to index of prev. occurrence of same hash. */
  int* hashval;  /* Index to hash value at this index. */
  int val;  /* Current hash value. */
} ZipfliHash;

/* Allocates ZipfliHash memory. */
void ZipfliAllocHash(size_t window_size, ZipfliHash* h);

/* Resets all fields of ZipfliHash. */
void ZipfliResetHash(size_t window_size, ZipfliHash* h);

/* Frees ZipfliHash memory. */
void ZipfliCleanHash(ZipfliHash* h);

/*
Updates the hash values based on the current position in the array. All calls
to this must be made for consecutive bytes.
*/
void ZipfliUpdateHash(const unsigned char* array, size_t pos, size_t end,
                      ZipfliHash* h);

/*
Prepopulates hash:
Fills in the initial values in the hash, before ZipfliUpdateHash can be used
correctly.
*/
void ZipfliWarmupHash(const unsigned char* array, size_t pos, size_t end,
                      ZipfliHash* h);

#endif  /* ZIPFLI_HASH_H_ */
