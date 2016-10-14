#include <gtest/gtest.h>
#include "SimdCompCompressor.cc"
#include "LZ4Compressor.cc"
#include "LZ4CompressorHC.cc"
#include "NoCompressor.cc"
#include "SnappyCompressor.cc"
#include "Simple9Compressor.cc"

template <class Compressor> void testFun() {
  function<int(int)> F[] = {
      [](int n) { return n; },          [](int n) { return n % 256; },
      [](int n) { return 0; },          [](int n) { return 1; },
      [](int n) { return n&1; },      [](int n) { return (n&2)>>1; },
      [](int n) { return rand() % 2; }, [](int n) { return rand() % 4; },
      [](int n) { return rand() % 8; }, [](int n) { return rand() % 16; }};

  for (auto f : F) {
    Compressor *compressor = new Compressor();

    size_t N = 999 * SIMDBlockSize; /* SIMDBlockSize is 128 */

    uint32_t *datain = new uint32_t[N * 2];
    uint32_t *datain2 = new uint32_t[N];

    char *buffer = new char[Compressor::getMaxOutBuffer(N * sizeof(uint32_t))];

    // initialiaze array
    for (int k = 0; k < 1 * N; ++k) {
      datain[k] = f(k);
      datain2[k] = datain[k];
    }

    int normalSize = N * sizeof(uint32_t);
    int compSize = compressor->compress(datain, buffer, normalSize);

    printf("compression ratio = %f \n",
           (N * sizeof(uint32_t)) / (compSize * 1.0));

    // cout <<"SIZE (UNCOMPRESSED) : "<< normalSize <<endl;
    // cout <<"SIZE (  COMPRESSED) : "<<compSize<<endl;

    char *decBuffer = buffer;
    uint32_t *dataout = new uint32_t[2 * N];
    compressor->decompress(decBuffer, dataout, normalSize, compSize);
    for (int n = 0; n < N; n++) {
      ASSERT_EQ(datain2[n], dataout[n]) << "data N: " << n << endl;
    }
  }
}

// get out what you put in
TEST(SimdCompressor, Compressed_scan_and_test) {
  testFun<SimdCompCompressor>();
}

// get out what you put in
TEST(LZ4Compressor, Compressed_scan_and_test) { testFun<LZ4Compressor>(); }

// get out what you put in
TEST(LZ4CompressorHC, Compressed_scan_and_test) { testFun<LZ4CompressorHC>(); }

// get out what you put in
TEST(NoCompressor, Compressed_scan_and_test) { testFun<NoCompressor>(); }

// get out what you put in
TEST(SnappyCompressor, Compressed_scan_and_test) {
  testFun<SnappyCompressor>();
}

// get out what you put in
TEST(Simple9Compressor, Compressed_scan_and_test) {
  testFun<Simple9Compressor>();
}

int main(int argc, char **argv) {
  printf("Running main() CompressedArray \n");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
