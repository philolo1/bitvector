#include "SetBitVector.cc"
#include "AdvancedSetBitVector.cc"
#include "DynamicBitVector.cc"
#include "BlockBitVector.cc"
#include "../compression/LZ4Compressor.cc"
#include "../compression/LZ4CompressorHC.cc"
#include "FlexibleBitVector.cc"
#include "../MemoryManager.cc"
#include <gtest/gtest.h>

#include "test_fun_datastructures.cc"

#include <iostream>

using namespace std;

#define BLOCK FlexibleBitVector<LZ4Compressor, MemoryManager, MemoryPointer>

TEST(ModifyAccess32_t, ZERO_ACCESS) {
  // make sure the tests are consistent
  // for each run
  srand(123);

  int arr[31];

  memset(arr, 0, 31 * sizeof(int));

  cout << "INIT" << endl;
  DynamicBitVector<LZ4Compressor, MemoryManager, MemoryPointer> bitvector(
      31, sizeof(int), 248);

  for (int n = 0; n < 31; n++) {
    bitvector.addBlock(arr + n);
  }

  for (int n = 0; n < 31; n++) {
    ASSERT_EQ(0, bitvector.accessUint32_t(n));
  }
}

TEST(ModifyAccess32_t, MODIFY) {
  // make sure the tests are consistent
  // for each run
  srand(123);

  int arr[31];

  memset(arr, 0, 31 * sizeof(int));

  //  cout << "INIT" << endl;
  DynamicBitVector<LZ4Compressor, MemoryManager, MemoryPointer> bitvector(
      31, sizeof(int), 248);

  for (int n = 0; n < 31; n++) {
    bitvector.addBlock(arr + n);
  }

  for (int n = 0; n < 31; n++) {
    ASSERT_EQ(0, bitvector.accessUint32_t(n));
  }

  // now modifying
  for (int n = 0; n < 31; n++) {
    //   cout << "modify: " << n << endl;
    bitvector.modifyUint32_t(n, n + 1);
  }

  for (int n = 0; n < 31; n++) {
    //    cout << "ACCESS" << endl;
    ASSERT_EQ(n + 1, bitvector.accessUint32_t(n));
  }
}

TEST(ModifyAccess32_t, select) {
  // make sure the tests are consistent
  // for each run
  srand(123);

  int arr[31];

  memset(arr, 0, 31 * sizeof(int));

  DynamicBitVector<LZ4Compressor, MemoryManager, MemoryPointer> bitvector(
      31, sizeof(int), 248);

  for (int n = 0; n < 31; n++) {
    bitvector.addBlock(arr + n);
  }

  for (int n = 0; n < 31; n++) {
    ASSERT_EQ(0, bitvector.accessUint32_t(n));
  }

  bitvector.modifyUint32_t(10, 1);

  // now modifying
  ASSERT_EQ(320, bitvector.select(0));
  ASSERT_EQ(-1, bitvector.select(1));
}

int main(int argc, char **argv) {
  printf("Running main() DynamicBitvector \n");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
