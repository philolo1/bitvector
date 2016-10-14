#include <snappy.h>

struct SnappyCompressor {

  static int getMaxOutBuffer(int chunkSize) {
    return snappy::MaxCompressedLength(chunkSize);
  }

  static int compress(void *inBuffer, void *outBuffer, int lenInBuffer) {
    size_t outputLength;
    snappy::RawCompress((char *)inBuffer, lenInBuffer, (char *)outBuffer,
                        &outputLength);
    return outputLength;
  }

  static void decompress(void *inBuffer, void *outBuffer, int lenOutBufferr,
                         int lenInBuffer) {
    snappy::RawUncompress((char *)inBuffer, lenInBuffer, (char *)outBuffer);
  }
};
