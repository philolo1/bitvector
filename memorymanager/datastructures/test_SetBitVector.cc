#include "SetBitVector.cc"
#include "AdvancedSetBitVector.cc"
#include "BlockBitVector.cc"
#include "FlexibleBitVector.cc"
#include "../compression/LZ4Compressor.cc"
#include "../compression/LZ4CompressorHC.cc"
#include "../compression/Simple9Compressor.cc"
#include "../MemoryManager.cc"
#include <gtest/gtest.h>

#include "test_fun_datastructures.cc"

#include <iostream>

using namespace std;

#define BLOCK BlockBitVector<Simple9Compressor, MemoryManager, MemoryPointer>
#define FLEX FlexibleBitVector<Simple9Compressor, MemoryManager, MemoryPointer>

TEST(SetBitVector, MODIFY) {
  // test1<SetBitVector<LZ4Compressor>>();
  // test1<AdvancedSetBitVector<LZ4Compressor>>();
  // test1<BLOCK>();
  test1<FLEX>();
}

TEST(SetBitVector, TWO_BLOCKS_RANDOM_MOD) {
  // test5<SetBitVector<LZ4Compressor>>();
  // test5<AdvancedSetBitVector<LZ4Compressor>>();
  // test2<BLOCK>();
  test2<FLEX>();
}

// get out what you put in
TEST(SetBitVector, ONE_BIT_SET_ACCESS_RANK) {
  // test3<SetBitVector<LZ4Compressor>>();
  // test3<AdvancedSetBitVector<LZ4Compressor>>();
  // test3<BLOCK>();
  test3<FLEX>();
}

TEST(SetBitVector, OneBlock_COMPLICATED_TEST) {
  // test4<SetBitVector<LZ4Compressor>>();
  // test4<AdvancedSetBitVector<LZ4Compressor>>();
  // test4<BLOCK>();
  test4<FLEX>();
}

// get out what you put in
TEST(SetBitVector, TWO_BLOCKS) {
  // test5<SetBitVector<LZ4Compressor>>();
  // test5<AdvancedSetBitVector<LZ4Compressor>>();
  // test5<BLOCK>();
  test5<FLEX>();
}

TEST(SetBitVector, TWO_BLOCKS_RANDOM) {
  // test6<SetBitVector<LZ4Compressor>>();
  // test6<AdvancedSetBitVector<LZ4Compressor>>();
  // test6<BLOCK>();
  test6<FLEX>();
}

TEST(SetBitVector, K_BLOCKS_RANDOM_MOD) {

  // test7<SetBitVector<LZ4Compressor>>();
  // test7<AdvancedSetBitVector<LZ4Compressor>>();
  // test7<BLOCK>();
  test7<FLEX>();
}

int main(int argc, char **argv) {
  printf("Running main() DynamicBitvector \n");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
