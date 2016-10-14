#include <lz4.h>

struct LZ4Compressor {

  static int getMaxOutBuffer(int chunkSize) {
    return LZ4_COMPRESSBOUND(chunkSize);
  }

  static int compress(void *inBuffer, void *outBuffer, int lenInBuffer) {
    return LZ4_compress((char *)inBuffer, (char *)outBuffer, lenInBuffer);
  }

  static void decompress(void *inBuffer, void *outBuffer, int lenOutBuffer, int lenInputBuffer) {
    LZ4_decompress_fast((char *)inBuffer, (char *)outBuffer, lenOutBuffer);
  }
};
