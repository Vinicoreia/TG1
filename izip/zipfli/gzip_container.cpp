#include "util.h"
#include "zipfli.h"
#include "gzip_container.h"
#include "deflate.h"

void ZipfliGzipCompress(const ZipfliOptions* options,
                        const unsigned char* in, size_t insize,
                        unsigned char** out, size_t* outsize) {
  unsigned long crcvalue = CRC(in, insize);

  unsigned char bp = 0; // TODO qq é isso?

/* 
  * bp: bit pointer for the output array. This must initially be 0, and for
  * consecutive calls must be reused (it can have values from 0-7). This is
  * because deflate appends blocks as bit-based data, rather than on byte
  * boundaries.
*/

  ZIPFLI_APPEND_DATA(31, out, outsize);  /* ID1 */
  ZIPFLI_APPEND_DATA(139, out, outsize);  /* ID2 */
  ZIPFLI_APPEND_DATA(8, out, outsize);  /* CM */
  ZIPFLI_APPEND_DATA(0, out, outsize);  /* FLG */
  /* MTIME */
  ZIPFLI_APPEND_DATA(0, out, outsize);
  ZIPFLI_APPEND_DATA(0, out, outsize);
  ZIPFLI_APPEND_DATA(0, out, outsize);
  ZIPFLI_APPEND_DATA(0, out, outsize);

  ZIPFLI_APPEND_DATA(2, out, outsize);  /* XFL, 2 indicates best compression. */
  ZIPFLI_APPEND_DATA(3, out, outsize);  /* OS follows Unix conventions. */

  ZipfliDeflate(options, 2 /* Dynamic block */, 1,
                in, insize, &bp, out, outsize);

  /* CRC */
  ZIPFLI_APPEND_DATA(crcvalue % 256, out, outsize);
  ZIPFLI_APPEND_DATA((crcvalue >> 8) % 256, out, outsize);
  ZIPFLI_APPEND_DATA((crcvalue >> 16) % 256, out, outsize);
  ZIPFLI_APPEND_DATA((crcvalue >> 24) % 256, out, outsize);

  /* ISIZE */
  ZIPFLI_APPEND_DATA(insize % 256, out, outsize);
  ZIPFLI_APPEND_DATA((insize >> 8) % 256, out, outsize);
  ZIPFLI_APPEND_DATA((insize >> 16) % 256, out, outsize);
  ZIPFLI_APPEND_DATA((insize >> 24) % 256, out, outsize);
}
