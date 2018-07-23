
#ifndef ZIPFLI_ZIPFLI_H_
#define ZIPFLI_ZIPFLI_H_

/*Default values*/
#define NUMITERATIONS 15
#define BLOCKSPLITTING 1
#define BLOCKSPLITTINGMAX 15

class ZipfliOptions{
    /*
    Maximum amount of times to rerun forward and backward pass to optimize LZ77
    compression cost. Good values: 10, 15 for small files, 5 for files over
    several MB in size or it will be too slow.
    */
    int numiterations;

    /*
    If true, splits the data in multiple deflate blocks with optimal choice
    for the block boundaries. Block splitting gives better compression. Default:
    true (1).
    */
    int blocksplitting;
    /*
    Maximum amount of blocks to split into (0 for unlimited, but this can give
    extreme results that hurt compression on some files). Default value: 15.
    */
    int blocksplittingmax;
    
    public:
      ZipfliOptions(int n, int split, int splitMax ){numiterations = n; blocksplitting = split; blocksplittingmax = splitMax;};
      ~ZipfliOptions();
};

#endif  /* ZIPFLI_ZIPFLI_H_ */