#ifndef IZIP_UTIL_H_
#define IZIP_UTIL_H_

#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
/* Cyclic Redundancy Check
 polynomial: 0xedb88320 */
static const unsigned long crc32_table[256] = {
    0u, 1996959894u, 3993919788u, 2567524794u, 124634137u, 1886057615u,
    3915621685u, 2657392035u, 249268274u, 2044508324u, 3772115230u, 2547177864u,
    162941995u, 2125561021u, 3887607047u, 2428444049u, 498536548u, 1789927666u,
    4089016648u, 2227061214u, 450548861u, 1843258603u, 4107580753u, 2211677639u,
    325883990u, 1684777152u, 4251122042u, 2321926636u, 335633487u, 1661365465u,
    4195302755u, 2366115317u, 997073096u, 1281953886u, 3579855332u, 2724688242u,
    1006888145u, 1258607687u, 3524101629u, 2768942443u, 901097722u, 1119000684u,
    3686517206u, 2898065728u, 853044451u, 1172266101u, 3705015759u, 2882616665u,
    651767980u, 1373503546u, 3369554304u, 3218104598u, 565507253u, 1454621731u,
    3485111705u, 3099436303u, 671266974u, 1594198024u, 3322730930u, 2970347812u,
    795835527u, 1483230225u, 3244367275u, 3060149565u, 1994146192u, 31158534u,
    2563907772u, 4023717930u, 1907459465u, 112637215u, 2680153253u, 3904427059u,
    2013776290u, 251722036u, 2517215374u, 3775830040u, 2137656763u, 141376813u,
    2439277719u, 3865271297u, 1802195444u, 476864866u, 2238001368u, 4066508878u,
    1812370925u, 453092731u, 2181625025u, 4111451223u, 1706088902u, 314042704u,
    2344532202u, 4240017532u, 1658658271u, 366619977u, 2362670323u, 4224994405u,
    1303535960u, 984961486u, 2747007092u, 3569037538u, 1256170817u, 1037604311u,
    2765210733u, 3554079995u, 1131014506u, 879679996u, 2909243462u, 3663771856u,
    1141124467u, 855842277u, 2852801631u, 3708648649u, 1342533948u, 654459306u,
    3188396048u, 3373015174u, 1466479909u, 544179635u, 3110523913u, 3462522015u,
    1591671054u, 702138776u, 2966460450u, 3352799412u, 1504918807u, 783551873u,
    3082640443u, 3233442989u, 3988292384u, 2596254646u, 62317068u, 1957810842u,
    3939845945u, 2647816111u, 81470997u, 1943803523u, 3814918930u, 2489596804u,
    225274430u, 2053790376u, 3826175755u, 2466906013u, 167816743u, 2097651377u,
    4027552580u, 2265490386u, 503444072u, 1762050814u, 4150417245u, 2154129355u,
    426522225u, 1852507879u, 4275313526u, 2312317920u, 282753626u, 1742555852u,
    4189708143u, 2394877945u, 397917763u, 1622183637u, 3604390888u, 2714866558u,
    953729732u, 1340076626u, 3518719985u, 2797360999u, 1068828381u, 1219638859u,
    3624741850u, 2936675148u, 906185462u, 1090812512u, 3747672003u, 2825379669u,
    829329135u, 1181335161u, 3412177804u, 3160834842u, 628085408u, 1382605366u,
    3423369109u, 3138078467u, 570562233u, 1426400815u, 3317316542u, 2998733608u,
    733239954u, 1555261956u, 3268935591u, 3050360625u, 752459403u, 1541320221u,
    2607071920u, 3965973030u, 1969922972u, 40735498u, 2617837225u, 3943577151u,
    1913087877u, 83908371u, 2512341634u, 3803740692u, 2075208622u, 213261112u,
    2463272603u, 3855990285u, 2094854071u, 198958881u, 2262029012u, 4057260610u,
    1759359992u, 534414190u, 2176718541u, 4139329115u, 1873836001u, 414664567u,
    2282248934u, 4279200368u, 1711684554u, 285281116u, 2405801727u, 4167216745u,
    1634467795u, 376229701u, 2685067896u, 3608007406u, 1308918612u, 956543938u,
    2808555105u, 3495958263u, 1231636301u, 1047427035u, 2932959818u, 3654703836u,
    1088359270u, 936918000u, 2847714899u, 3736837829u, 1202900863u, 817233897u,
    3183342108u, 3401237130u, 1404277552u, 615818150u, 3134207493u, 3453421203u,
    1423857449u, 601450431u, 3009837614u, 3294710456u, 1567103746u, 711928724u,
    3020668471u, 3272380065u, 1510334235u, 755167117u};

/* Frequently used  streams*/
extern std::string bitString;
extern std::string strBuffer;
extern std::vector<char> vecBuffer;
extern uint8_t *u8Buffer;
extern size_t filesize;

static unsigned long CRC(const unsigned char *data, size_t size);

/*Read file as string to the global u8Buffer*/
void readFileToBufferAsString(std::string filename);
void readFileAsBinaryString(std::string filename);
void readFileAsVector(std::string filename);
/*writes the given string to a file*/
int writeStringToFile(std::string fileOutName, std::string outString);

/* Cast a integer to binary and returns the bitString corresponding*/
std::string decimalToBitString(unsigned int n, unsigned int len);

/* Read file as a uint8_t* */
uint8_t *readFileAsU8(std::string filenameIn);

/*Write the bitString to a file*/
int writeEncodedFile(std::string fileOutName);
void writeDecodedFile(std::string filenameOut, std::string outString);

/*Function to cast a char to binary using bitset*/
std::string charToBin(char c);

/*Function to get the filesize and return as a long long.*/
void getFileSize(std::string filename);

/* This function gets the frequency associated with the strbuffer*/
std::vector<std::pair<char, long long>> getFrequencyU8(std::string buffer);
std::vector<std::pair<uint16_t, long long>> getFrequencyU16(std::vector<uint16_t> buffer);
std::vector<std::pair<uint32_t, long long>> getFrequencyU32(std::vector<uint32_t> buffer);



/* Minimum and maximum length that can be encoded in deflate. */
#define ZIPFLI_MAX_MATCH 258
#define ZIPFLI_MIN_MATCH 3 /*it doesn't make sense to encode less than 3 bytes this way 00000000 corresponds to decimal 3*/

/* Number of distinct literal/length and distance symbols in DEFLATE */
#define ZIPFLI_NUM_LL 288
#define ZIPFLI_NUM_D 32

/*
The window size for deflate. Must be a power of two. This should be 32768, the
maximum possible by the deflate spec. Anything less hurts compression more than
speed.
*/
#define ZIPFLI_WINDOW_SIZE 32768

/*
The window mask used to wrap indices into the window. This is why the
window size must be a power of two.
*/

#define ZIPFLI_WINDOW_MASK (ZIPFLI_WINDOW_SIZE - 1)

/*
A block structure of huge, non-smart, blocks to divide the input into, to allow
operating on huge files without exceeding memory, such as the 1GB wiki9 corpus.
The whole compression algorithm, including the smarter block splitting, will
be executed independently on each huge block.
Dividing into huge blocks hurts compression, but not much relative to the size.
Set it to 0 to disable master blocks.
*/

#define ZIPFLI_MASTER_BLOCK_SIZE 1000000

/*
Used to initialize costs for example
*/
#define ZIPFLI_LARGE_FLOAT 1e30

/*
For longest match cache. max 256. Uses huge amounts of memory but makes it
faster. Uses this many times three bytes per single byte of the input data.
This is so because longest match finding has to find the exact distance
that belongs to each length for the best lz77 strategy.
Good values: e.g. 5, 8.
*/
#define ZIPFLI_CACHE_LENGTH 8

/*
limit the max hash chain hits for this hash value. This has an effect only
on files where the hash value is the same very often. On these files, this
gives worse compression (the value should ideally be 32768, which is the
ZIPFLI_WINDOW_SIZE, while zlib uses 4096 even for best level), but makes it
faster on some specific files.
Good value: e.g. 8192.
*/
#define ZIPFLI_MAX_CHAIN_HITS 8192

/*
Whether to use the longest match cache for ZipfliFindLongestMatch. This cache
consumes a lot of memory but speeds it up. No effect on compression size.
*/
#define ZIPFLI_LONGEST_MATCH_CACHE

/*
Enable to remember amount of successive identical bytes in the hash chain for
finding longest match
required for ZIPFLI_HASH_SAME_HASH and ZIPFLI_SHORTCUT_LONG_REPETITIONS
This has no effect on the compression result, and enabling it increases speed.
*/
#define ZIPFLI_HASH_SAME

/*
Switch to a faster hash based on the info from ZIPFLI_HASH_SAME once the
best length so far is long enough. This is way faster for files with lots of
identical bytes, on which the compressor is otherwise too slow. Regular files
are unaffected or maybe a tiny bit slower.
This has no effect on the compression result, only on speed.
*/
#define ZIPFLI_HASH_SAME_HASH

/*
Enable this, to avoid slowness for files which are a repetition of the same
character more than a multiple of ZIPFLI_MAX_MATCH times. This should not affect
the compression result.
*/
#define ZIPFLI_SHORTCUT_LONG_REPETITIONS

/*
Whether to use lazy matching in the greedy LZ77 implementation. This gives a
better result of ZipfliLZ77Greedy, but the effect this has on the optimal LZ77
varies from file to file.
*/
#define ZIPFLI_LAZY_MATCHING

/*
Appends value to dynamically allocated memory, doubling its allocation size
whenever needed.

value: the value to append, type T
data: pointer to the dynamic array to append to, type T**
size: pointer to the size of the array to append to, type size_t*. This is the
size that you consider the array to be, not the internal allocation size.
Precondition: allocated size of data is at least a power of two greater than or
equal than *size.
*/


#define ZIPFLI_APPEND_DATA(/* T */ value, /* T** */ data, /* size_t* */ size) {\
  if (!((*size) & ((*size) - 1))) {\
    /*double alloc size if it's a power of two*/\
    void** data_void = reinterpret_cast<void**>(data);\
    *data_void = (*size) == 0 ? malloc(sizeof(**data))\
                              : realloc((*data), (*size) * 2 * sizeof(**data));\
  }\
  (*data)[(*size)] = (value);\
  (*size)++;\
}

#endif // IZIP_UTIL_H
