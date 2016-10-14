#include "SetBitVector.cc"
#include "AdvancedSetBitVector.cc"
#include "BlockBitVector.cc"
#include "../compression/LZ4Compressor.cc"
#include "../compression/LZ4CompressorHC.cc"
#include "../MemoryManager.cc"
#include <gtest/gtest.h>
#include "CacheProvider.cc"

#include "test_fun_datastructures.cc"

#include <iostream>

using namespace std;

void testCache(int cacheSize) {
  int SIZE = 1000;

  int arr[SIZE];
  for (int n = 0; n < SIZE; n++) {
    arr[n] = n;
  }

  // we don't really care what is put here, we just want to get pointers
  MemoryManager *manager = new MemoryManager(10);

  CacheProvider *cache = new CacheProvider(cacheSize, sizeof(int));

  vector<MemoryPointer *> vecPtr;

  for (int n = 0; n < SIZE; n++) {

    MemoryPointer *ptr = manager->create(10);
    // before we have put something it should be empty
    ASSERT_EQ(nullptr, cache->access(ptr));

    ((int *)cache->put(ptr))[0] = arr[n];

    int num = ((int *)cache->access(ptr))[0];
    ASSERT_EQ(n, num);

    if (vecPtr.size() > 0) {

      // do this in reverse order to make sure the least
      // recently used technique works
      for (int k = cacheSize - 2; k >= 0; k--) {
        int index = vecPtr.size() - k - 1;
        if (index < 0)
          break;
        int lastnum = ((int *)cache->access(vecPtr[vecPtr.size() - k - 1]))[0];
        ASSERT_EQ(n - k - 1, lastnum);
      }

      cache->access(ptr);
    }

    // old pointer should not be valid any more
    for (int m = 0; m < n - cacheSize; m++) {
      ASSERT_EQ(nullptr, cache->access(vecPtr[n]));
    }

    vecPtr.push_back(ptr);
  }
}

void testRecentlyUsed(int cacheSize) {
  int SIZE = 1000;

  int arr[SIZE];
  for (int n = 0; n < SIZE; n++) {
    arr[n] = n;
  }

  arr[0] = -3;

  // we don't really care what is put here, we just want to get pointers
  MemoryManager *manager = new MemoryManager(10);

  CacheProvider *cache = new CacheProvider(cacheSize, sizeof(int));

  MemoryPointer *ptr = manager->create(10);
  ((int *)cache->put(ptr))[0] = arr[0];

  int counter = 0;

  for (int n = 1; n < SIZE; n++) {
    auto ptr2 = manager->create(10);

    ((int *)cache->put(ptr2))[0] = arr[n];

    counter++;
    if (counter == cacheSize - 1) {
      ASSERT_EQ(arr[0], ((int *)cache->access(ptr))[0]);
      counter = 0;
    }
  }
}

TEST(CacheProvider, OneInt) { testCache(1); }

TEST(CacheProvider, TwoCache) { testCache(2); }

TEST(CacheProvider, ThreeCache) { testCache(3); }

TEST(CacheProvider, SurviveOne) {
  for (int n = 2; n < 100; n++) {
    if (n % 10 == 0)
      printf("recently used : testing %d of 100\n", n);
    testCache(n);
  }
}

TEST(CacheProvider, BigCache) {
  for (int n = 4; n < 100; n++) {
    if (n % 10 == 0)
      printf("bigcache: testing %d of 100\n", n);
    testCache(n);
  }
}

int main(int argc, char **argv) {
  printf("Running main() DynamicBitvector \n");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
