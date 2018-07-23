#include "zipfli.h"
#include <iostream>
#include <string>
#include "gzip_container.h"
#include "util.h"
/*The format will always be GZIP*/




int main(int argc, char **argv){
    ZipfliOptions options(15, 1, 15);
    std::string filename = "bee.bmp";
    std::string outfilename = "teste.gz";
    CompressFile(&options, filename, outfilename);

    return 0;
}
// TODO porque ele trata a entrada como out e reserva outsize mas não lê o arquivo.
static int LoadFile(std::string filename,
                    unsigned char** out, size_t* outsize) {
  FILE* file;

  *out = 0;
  *outsize = 0;
  file = fopen(filename.c_str(), "rb");
  if (!file) return 0;

  fseek(file , 0 , SEEK_END);
  *outsize = ftell(file);
  if(*outsize > 2147483647) {
    fprintf(stderr,"Files larger than 2GB are not supported.\n");
    exit(EXIT_FAILURE);
  }
  rewind(file);

  *out = (unsigned char*)malloc(*outsize);
// TODO a parte a baixo é para checar se é diretório
//   if (*outsize && (*out)) {
//     size_t testsize = fread(*out, 1, *outsize, file);
//     if (testsize != *outsize) {
//       /* It could be a directory */
//       free(*out);
//       *out = 0;
//       *outsize = 0;
//       fclose(file);
//       return 0;
//     }
//   }


  fclose(file);
  return 1;
}

static void CompressFile(const ZipfliOptions* options,
                            std::string infilename,
                            std::string outfilename){
    
    unsigned char *in;
    size_t insize;
    unsigned char* out=0;
    size_t outsize=0;

//TODO Trocar para meus próprios metodos de leitura de arquivo
  if (!LoadFile(infilename, &in, &insize)) {
    std::cout<<"Erro! arquivo não encontrado";
    return;
  }
  
  ZipfliGzipCompress(options, in, insize, &out, &outsize);



}