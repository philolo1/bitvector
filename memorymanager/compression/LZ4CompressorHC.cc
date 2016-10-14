#include <lz4hc.h>
#include <lz4.h>

// uses the slower with higher compression
struct LZ4CompressorHC {

  static int getMaxOutBuffer(int chunkSize) {
    return LZ4_COMPRESSBOUND(chunkSize);
  }

  static int compress(void *inBuffer, void * outBuffer, int lenInBuffer) {
    return LZ4_compressHC((char *)inBuffer, (char *)outBuffer, lenInBuffer);
  }

  static void decompress(void *inBuffer, void* outBuffer, int lenOutBuffer, int lenInputBuffer) {
    LZ4_decompress_fast((char *)inBuffer, (char *)outBuffer, lenOutBuffer);
  }
};
