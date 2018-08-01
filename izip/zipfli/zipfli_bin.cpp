#include "zipfli.h"
#include <iostream>
#include <string>
#include "gzip_container.h"
#include "util.h"
/*The format will always be GZIP*/

// TODO porque ele trata a entrada como out e reserva outsize mas não lê o arquivo.
static int LoadFile(std::string filename,
                    unsigned char **out, size_t *outsize)
{
  FILE *file;

  *out = 0;
  *outsize = 0;
  file = fopen(filename.c_str(), "rb");
  if (!file)
    return 0;
  fseek(file, 0, SEEK_END);
  *outsize = ftell(file);
  rewind(file);
  *out = (unsigned char *)malloc(*outsize);
  fclose(file);
  return 1;
}

static void SaveFile(std::string filename,
                     const unsigned char* in, size_t insize) {
  FILE* file = fopen(filename.c_str(), "wb" );
  if (file == NULL) {
      fprintf(stderr,"Error: Cannot write to output file, terminating.\n");
      exit (EXIT_FAILURE);
  }
  fwrite((char*)in, 1, insize, file);
  fclose(file);
}


static void CompressFile(const ZipfliOptions *options,
                         std::string infilename,
                         std::string outfilename)
{

  readFileAsU8(infilename);
  //TODO Trocar para meus próprios metodos de leitura de arquivo
  
  ZipfliGzipCompress(options);

  if (outfilename.size() > 0)
  {
    SaveFile(outfilename, out, outsize);
  }
  else
  {
  #if _WIN32
    /* Windows workaround for stdout output. */
    _setmode(_fileno(stdout), _O_BINARY);
  #endif
    fwrite(out, 1, outsize, stdout);
  }

  free(out);
  free(in);
}


int main(int argc, char **argv)
{
  ZipfliOptions options(15, 1, 15);
  std::string filename = "bee.bmp";
  std::string outfilename = "teste.gz";
  CompressFile(&options, filename, outfilename);

  return 0;
}