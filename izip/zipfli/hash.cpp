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

#include "hash.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define HASH_SHIFT 5
#define HASH_MASK 32767

void ZipfliAllocHash(size_t window_size, ZipfliHash* h) {
  h->head = (int*)malloc(sizeof(*h->head) * 65536);
  h->prev = (unsigned short*)malloc(sizeof(*h->prev) * window_size);
  h->hashval = (int*)malloc(sizeof(*h->hashval) * window_size);
}

void ZipfliResetHash(size_t window_size, ZipfliHash* h) {
  size_t i;

  h->val = 0;
  for (i = 0; i < 65536; i++) {
    h->head[i] = -1;  /* -1 indicates no head so far. */
  }
  for (i = 0; i < window_size; i++) {
    h->prev[i] = i;  /* If prev[j] == j, then prev[j] is uninitialized. */
    h->hashval[i] = -1;
  }
}

void ZipfliCleanHash(ZipfliHash* h) {
  free(h->head);
  free(h->prev);
  free(h->hashval);
}

/*
Update the sliding hash value with the given byte. All calls to this function
must be made on consecutive input characters. Since the hash value exists out
of multiple input bytes, a few warmups with this function are needed initially.
*/
static void UpdateHashValue(ZipfliHash* h, unsigned char c) {
  h->val = (((h->val) << HASH_SHIFT) ^ (c)) & HASH_MASK;
}

void ZipfliUpdateHash(const unsigned char* array, size_t pos, size_t end,
                ZipfliHash* h) {
  unsigned short hpos = pos & ZIPFLI_WINDOW_MASK;
#ifdef ZIPFLI_HASH_SAME
  size_t amount = 0;
#endif

  UpdateHashValue(h, pos + ZIPFLI_MIN_MATCH <= end ?
      array[pos + ZIPFLI_MIN_MATCH - 1] : 0);
  h->hashval[hpos] = h->val;
  if (h->head[h->val] != -1 && h->hashval[h->head[h->val]] == h->val) {
    h->prev[hpos] = h->head[h->val];
  }
  else h->prev[hpos] = hpos;
  h->head[h->val] = hpos;
}

void ZipfliWarmupHash(const unsigned char* array, size_t pos, size_t end,
                ZipfliHash* h) {
  UpdateHashValue(h, array[pos + 0]);
  if (pos + 1 < end) UpdateHashValue(h, array[pos + 1]);
}
