#include <compress/policy/Simple9.hpp>

using namespace integer_encoding::internals;

struct Simple9Compressor {

  static int getMaxOutBuffer(int chunkSize) { return 2 * chunkSize; }

  static int compress(void *inBuffer, void *outBuffer, int length) {
    Simple9 simp;

    length /= sizeof(uint32_t);
    uint64_t len2 = length;
    uint32_t *intBuffer = (uint32_t *)inBuffer;

    int pos = 0;

    int mynum = 0;
    int counter = 0;

    int newLength = length;

    for (int pos = 0; pos < len2; pos++) {
      mynum += intBuffer[pos] >> 28;
      intBuffer[pos] &= 0x0fffffff;
      counter++;
      if (counter == 7) {
        intBuffer[newLength] = mynum;
        newLength++;
        counter = 0;
        mynum = 0;
      } else {
        mynum <<= 4;
      }
    }

    if (counter > 0) {
      // easier for decompress (first posit
      intBuffer[newLength] = mynum << (4 * (6 - counter));
      newLength++;
    }

    simp.encodeArray((uint32_t *)inBuffer, newLength, (uint32_t *)outBuffer,
                     &len2);
    return len2 * sizeof(uint32_t);
  }

  static void decompress(void *inBuffer, void *outBuffer, int lenOut,
                         int lenIn) {
    Simple9 simp;
    lenOut /= sizeof(uint32_t);
    lenIn /= sizeof(uint32_t);

    int len2 = lenOut;

    // calculate lenght depending on our new information
    lenOut = lenOut + (lenOut + 6) / 7;

    simp.decodeArray((uint32_t *)inBuffer, lenIn, (uint32_t *)outBuffer,
                     lenOut);

    uint32_t *intBuffer = (uint32_t *)outBuffer;

    int pattern = 0x0f000000;
    int move = 4;
    int oldPos = len2;

    for (int pos = 0; pos < len2; pos++) {

      intBuffer[pos] |= (intBuffer[oldPos] & pattern) << move;

      pattern >>= 4;

      if (pattern == 0) {
        pattern = 0x0f000000;
        move = 4;
        oldPos++;
      } else {
        move += 4;
      }
    }
  }
};
