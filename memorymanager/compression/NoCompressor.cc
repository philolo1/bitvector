// this is a no compressor, just for testing we would like
// to see what happens if we don't compress at all 
// comparison wise etc

#include <memory.h>

struct NoCompressor {

  static int getMaxOutBuffer(int chunkSize) {
    return chunkSize;
  }

  static int compress(void *inBuffer, void *outBuffer, int lenInBuffer) {
    memcpy(outBuffer, inBuffer, lenInBuffer);
    return lenInBuffer;
  }

  static void decompress(void *inBuffer, void *outBuffer, int lenOutBuffer, int lenInputBuffer) {
    memcpy(outBuffer, inBuffer, lenOutBuffer);
  }
};
