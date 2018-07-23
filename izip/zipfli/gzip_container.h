#ifndef ZIPFLI_GZIP_H_
#define ZIPFLI_GZIP_H_

#include "zipfli.h"
#include <cstdio>
/*
Compresses according to the gzip specification and append the compressed
result to the output.

options: global program options
out: pointer to the dynamic output array to which the result is appended. Must
  be freed after use.
outsize: pointer to the dynamic output array size.
*/
void ZipfliGzipCompress(const ZipfliOptions* options,
                        const unsigned char* in, size_t insize,
                        unsigned char** out, size_t* outsize);

#endif  /* ZIPFLI_GZIP_H_ */
