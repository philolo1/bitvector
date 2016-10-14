#include <simdcomp.h>

#include <stdio.h>

#include <iostream>

using namespace std;

/* TODO
 * look at
 *https://github.com/lemire/simdcomp/blob/master/example.c
 * to see how to use it
 */
struct SimdCompCompressor {

  static int getMaxOutBuffer(int chunkSize) {
    int numIntegers = chunkSize / sizeof(uint32_t);
    return chunkSize + numIntegers / SIMDBlockSize;
  }

  static int compress(void * /*uint32_t*/ datainVoidPtr, void *bufferPtr,
                      int length) {

    uint32_t *datain = (uint32_t *)datainVoidPtr;
    char *buffer = (char *)bufferPtr;

    // due to the interface the lenght is in char*
    length /= sizeof(uint32_t);

    uint32_t offset = 0;
    char *initout = buffer;
    for (size_t k = 0; k < length / SIMDBlockSize; ++k) {
      uint32_t b = simdmaxbitsd1(offset, datain + k * SIMDBlockSize);
      *initout++ = b;
      simdpackwithoutmaskd1(offset, datain + k * SIMDBlockSize,
                            (__m128i *)initout, b);
      offset = datain[k * SIMDBlockSize + SIMDBlockSize - 1];
      initout += b * sizeof(__m128i);
    }
    return initout - buffer;
  }

  static void decompress(void *decbufferPtr, void *backbufferPtr, int length,
                         int length2) {

    char *decbuffer = (char *)decbufferPtr;
    uint32_t *backbuffer = (uint32_t *)backbufferPtr;

    // length is in chars right now
    length /= sizeof(uint32_t);

    uint32_t offset = 0;
    uint32_t *backPtr = backbuffer;

    for (int k = 0; k * SIMDBlockSize < length; ++k) {
      uint8_t b = *decbuffer++;
      simdunpackd1(offset, (__m128i *)decbuffer, backPtr, b);

      /* do something here with backbuffer */
      decbuffer += b * sizeof(__m128i);
      offset = backPtr[SIMDBlockSize - 1];
      backPtr += SIMDBlockSize;
    }
  }
};

size_t compress(uint32_t *datain, size_t length, char *buffer) {
  uint32_t offset;
  char *initout;
  size_t k;
  if (length / SIMDBlockSize * SIMDBlockSize != length) {
    printf("Data length should be a multiple of %i \n", SIMDBlockSize);
  }
  offset = 0;
  initout = buffer;
  for (k = 0; k < length / SIMDBlockSize; ++k) {
    uint32_t b = simdmaxbitsd1(offset, datain + k * SIMDBlockSize);
    *buffer++ = b;
    simdpackwithoutmaskd1(offset, datain + k * SIMDBlockSize, (__m128i *)buffer,
                          b);
    offset = datain[k * SIMDBlockSize + SIMDBlockSize - 1];
    buffer += b * sizeof(__m128i);
  }
  return buffer - initout;
}
