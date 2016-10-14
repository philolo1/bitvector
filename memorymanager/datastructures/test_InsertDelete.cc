#include "SetBitVector.cc"
#include "AdvancedSetBitVector.cc"
#include "DynamicBitVector.cc"
#include "BlockBitVector.cc"
#include "FlexibleBitVector.cc"
#include "../compression/LZ4Compressor.cc"
#include "../compression/LZ4CompressorHC.cc"
#include "../MemoryManager.cc"
#include <gtest/gtest.h>

#include <iostream>

using namespace std;

#define FLEX FlexibleBitVector<LZ4Compressor, MemoryManager, MemoryPointer>

TEST(INSERT, ONE) {

  uint32_t arr[2];

  arr[0] = 0;
  arr[1] = 2;

  FLEX bitvector(1, 2 * sizeof(uint32_t), 248);

  bitvector.addBlock(arr);

  bitvector.insertUint32_t(1, 12);

  ASSERT_EQ(0, bitvector.accessUint32_t(0));
  ASSERT_EQ(12, bitvector.accessUint32_t(1));
  ASSERT_EQ(2, bitvector.accessUint32_t(2));
}

TEST(INSERT, ONE_2) {

  uint32_t arr[2];

  arr[0] = 0;
  arr[1] = 2;

  FLEX bitvector(1, 2 * sizeof(uint32_t), 248);

  bitvector.addBlock(arr);

  bitvector.insertUint32_t(0, 12);

  ASSERT_EQ(12, bitvector.accessUint32_t(0));
  ASSERT_EQ(0, bitvector.accessUint32_t(1));
  ASSERT_EQ(2, bitvector.accessUint32_t(2));
}

TEST(DELETE, ONE) {

  for (int k = 0; k < 3; k++) {
    uint32_t *arr = new uint32_t[3];

    arr[0] = 0;
    arr[1] = 1;
    arr[2] = 2;

    FLEX bitvector(1, 3 * sizeof(uint32_t), 248);

    bitvector.addBlock(arr);

    bitvector.deleteUint32_t(k);

    int counter = 0;

    for (int n = 0; n < 2; n++) {
      if (k == n) {
        counter++;
      }
      ASSERT_EQ(counter, bitvector.accessUint32_t(n));
      counter++;
    }
  }
}

TEST(INSERT, TWO) {

  uint32_t arr[4];

  for (int n = 0; n < 4; n++) {
    arr[n] = n;
  }

  FLEX bitvector(2, 2 * sizeof(uint32_t), 248);

  bitvector.addBlock(arr);
  bitvector.addBlock(arr + 2);

  bitvector.insertUint32_t(0, 12);

  ASSERT_EQ(12, bitvector.accessUint32_t(0));
  ASSERT_EQ(0, bitvector.accessUint32_t(1));
  ASSERT_EQ(1, bitvector.accessUint32_t(2));
  ASSERT_EQ(2, bitvector.accessUint32_t(3));
  ASSERT_EQ(3, bitvector.accessUint32_t(4));
}

TEST(DELETE, TWO) {

  for (int deleteIndex = 0; deleteIndex < 4; deleteIndex++) {
    uint32_t arr[4];

    for (int n = 0; n < 4; n++) {
      arr[n] = n;
    }

    FLEX bitvector(2, 2 * sizeof(uint32_t), 248);

    bitvector.addBlock(arr);
    bitvector.addBlock(arr + 2);

    bitvector.deleteUint32_t(deleteIndex);

    int counter = 0;

    for (int n = 0; n < 4; n++) {
      if (n == deleteIndex)
        continue;
      ASSERT_EQ(n, bitvector.accessUint32_t(counter));
      counter++;
    }
  }
}

TEST(INSERT, THREE) {

  const int SIZE = 6;
  uint32_t arr[6];

  for (int n = 0; n < SIZE; n++) {
    arr[n] = n;
  }

  FLEX bitvector(50, 2 * sizeof(uint32_t), 248);

  for (int n = 0; n < 3; n++) {
    bitvector.addBlock(arr + 2 * n);
  }

  for (int n = 0; n < 6; n++) {
    ASSERT_EQ(n, bitvector.accessUint32_t(n));
  }

  for (int n = 0; n < 6; n++) {
    int pos = 2 * n;
    bitvector.insertUint32_t(2 * n, n);
  }

  for (int n = 0; n < 12; n++) {
    ASSERT_EQ(n / 2, bitvector.accessUint32_t(n));
  }
}

TEST(DELETE, THREE) {

  for (int k = 0; k < 6; k++) {

    const int SIZE = 6;
    uint32_t arr[6];

    for (int n = 0; n < SIZE; n++) {
      arr[n] = n;
    }

    FLEX bitvector(50, 2 * sizeof(uint32_t), 248);

    for (int n = 0; n < 3; n++) {
      bitvector.addBlock(arr + 2 * n);
    }

    for (int n = 0; n < 6; n++) {
      ASSERT_EQ(n, bitvector.accessUint32_t(n));
    }

    bitvector.deleteUint32_t(k);

    int counter = 0;
    for (int n = 0; n < 6; n++) {
      if (n == k)
        continue;
      ASSERT_EQ(n, bitvector.accessUint32_t(counter));
      counter++;
    }
  }
}

TEST(DELETE, BIG) {

  const int SIZE = 18;
  uint32_t arr[SIZE];

  for (int n = 0; n < SIZE; n++) {
    arr[n] = n;
  }

  FLEX bitvector(SIZE, 2 * sizeof(uint32_t), 248);

  for (int n = 0; n < SIZE / 2; n++) {
    bitvector.addBlock(arr + 2 * n);
  }

  for (int n = 0; n < SIZE; n++) {
    ASSERT_EQ(n, bitvector.accessUint32_t(n));
  }

  for (int n = 0; n < SIZE / 2; n++) {
    bitvector.deleteUint32_t(n);
  }

  for (int n = 0; n < SIZE / 2; n++) {
    ASSERT_EQ(2 * n + 1, bitvector.accessUint32_t(n));
  }
}

TEST(OVERFLOW_INSERT, ONE) {

  const int SIZE = 2;
  uint32_t arr[SIZE];

  for (int n = 0; n < 2; n++) {
    arr[n] = n;
  }

  FLEX bitvector(SIZE, 2 * sizeof(uint32_t), 248);

  for (int n = 0; n < SIZE / 2; n++) {
    bitvector.addBlock(arr + 2 * n);
  }

  for (int n = 0; n < 2; n++) {
    bitvector.insertUint32_t(0, 12 - n);
  }

  ASSERT_EQ(11, bitvector.accessUint32_t(0));
  ASSERT_EQ(12, bitvector.accessUint32_t(1));
  ASSERT_EQ(0, bitvector.accessUint32_t(2));
  ASSERT_EQ(1, bitvector.accessUint32_t(3));
}

TEST(OVERFLOW_INSERT, ONE_BLOCK_THOUSAND_INSERTS) {
  const int SIZE = 2;
  uint32_t arr[SIZE];

  arr[0] = 1001;
  arr[1] = 1002;

  FLEX bitvector(SIZE, 2 * sizeof(uint32_t), 248);

  bitvector.addBlock(arr);

  for (int n = 1000; n >= 0; n--) {
    bitvector.insertUint32_t(0, n);
  }

  for (int n = 0; n < 1003; n++) {
    ASSERT_EQ(n, bitvector.accessUint32_t(n));
  }
}

TEST(OVERFLOW_DELETE, DELETE_CASES_1) {
  const int SIZE = 80 * 4;
  uint32_t arr[SIZE];

  for (int n = 0; n < SIZE; n++) {
    arr[n] = n;
  }

  FLEX bitvector(SIZE, 4 * sizeof(uint32_t), 248);

  for (int n = 0; n < SIZE / 4; n++) {
    bitvector.addBlock(arr + 4 * n);
  }

  bitvector.deleteUint32_t(252);
}

TEST(OVERFLOW_DELETE, DELETE_CASES) {
  int SIZE = 3 * 4;
  uint32_t arr[SIZE];

  for (int n = 0; n < SIZE; n++) {
    arr[n] = n;
  }

  FLEX bitvector(SIZE, 4 * sizeof(uint32_t), 248);

  for (int n = 0; n < SIZE / 4; n++) {
    bitvector.addBlock(arr + 4 * n);
  }

  for (int n = 0; n < SIZE; n++) {
  }

  SIZE--;

  bitvector.deleteUint32_t(4);

  SIZE--;

  bitvector.deleteUint32_t(4);

  SIZE--;

  bitvector.deleteUint32_t(4);

  for (int n = 0; n < 3; n++) {
    ASSERT_EQ(n, bitvector.accessUint32_t(n));
  }

  for (int n = 4; n < SIZE; n++) {
    ASSERT_EQ(n + 3, bitvector.accessUint32_t(n));
  }
}

TEST(INSERT, BIG) {

  const int SIZE = 20;
  uint32_t arr[SIZE];

  for (int n = 0; n < SIZE; n++) {
    arr[n] = n;
  }

  FLEX bitvector(SIZE, 2 * sizeof(uint32_t), 248);

  for (int n = 0; n < SIZE / 2; n++) {
    bitvector.addBlock(arr + 2 * n);
  }

  for (int n = 0; n < SIZE; n++) {
    ASSERT_EQ(n, bitvector.accessUint32_t(n));
  }

  for (int n = 0; n < SIZE; n++) {
    int pos = 2 * n;
    bitvector.insertUint32_t(2 * n, n);
  }
  //
  for (int n = 0; n < 2 * SIZE; n++) {
    ASSERT_EQ(n / 2, bitvector.accessUint32_t(n));
  }
}

TEST(DELETE, END) {

  const int SIZE = 100;
  uint32_t arr[SIZE];

  for (int n = 0; n < SIZE; n++) {
    arr[n] = n;
  }

  FLEX bitvector(SIZE, 2 * sizeof(uint32_t), 248);

  for (int n = 0; n < SIZE / 2; n++) {
    bitvector.addBlock(arr + 2 * n);
  }

  for (int n = 0; n < SIZE; n++) {
    ASSERT_EQ(n, bitvector.accessUint32_t(n));
  }

  for (int n = 0; n < SIZE / 2; n++) {
    int num = SIZE - 1 - n;
    bitvector.deleteUint32_t(SIZE - 1 - n);
  }

  for (int n = 0; n < SIZE / 2; n++) {
    ASSERT_EQ(n, bitvector.accessUint32_t(n));
  }
}

TEST(DELETE, DELETE_ALL) {

  srand(42);

  for (int j = 0; j < 1000; j++) {

    int SIZE = 16;
    uint32_t arr[SIZE];

    for (int n = 0; n < SIZE; n++) {
      arr[n] = n;
    }

    FLEX bitvector(SIZE, 4 * sizeof(uint32_t), 248);

    for (int n = 0; n < SIZE / 4; n++) {
      bitvector.addBlock(arr + 4 * n);
    }

    vector<bool> used(SIZE, 1);
    for (int k = 0; k < 4 * SIZE; k++) {

      int num = rand() % SIZE;

      if (used[num] == false)
        continue;

      used[num] = false;

      int sum = 0;
      for (int n = 0; n < num; n++) {
        sum += used[n];
      }

      bitvector.deleteUint32_t(sum);

      int index = 0;
      for (int n = 0; n < SIZE; n++) {

        if (!used[n]) {
          continue;
        };
        ASSERT_EQ(arr[n], bitvector.accessUint32_t(index))
            << "n : " << n << " index : " << index << endl;
        index++;
      }
    }
  }
}

TEST(DELETE, DELETE_ALL_2) {

  srand(42);

  for (int j = 0; j < 100; j++) {
    int SIZE = 40;
    uint32_t arr[SIZE];

    for (int n = 0; n < SIZE; n++) {
      arr[n] = n;
    }

    FLEX bitvector(SIZE, 4 * sizeof(uint32_t), 248);

    for (int n = 0; n < SIZE / 4; n++) {
      bitvector.addBlock(arr + 4 * n);
    }

    vector<bool> used(SIZE, 1);
    for (int k = 0; k < 4 * SIZE; k++) {

      int num = rand() % SIZE;

      if (used[num] == false)
        continue;

      used[num] = false;

      int sum = 0;
      for (int n = 0; n < num; n++) {
        sum += used[n];
      }

      bitvector.deleteUint32_t(sum);

      int index = 0;
      for (int n = 0; n < SIZE; n++) {

        if (!used[n]) {
          continue;
        };
        ASSERT_EQ(arr[n], bitvector.accessUint32_t(index))
            << "n : " << n << " index : " << index << endl;
        index++;
      }
    }
  }
}

TEST(DELETE, DELETE_ALL_3) {

  srand(42);

  int SIZE = 400;
  uint32_t arr[SIZE];

  for (int n = 0; n < SIZE; n++) {
    arr[n] = n;
  }

  FLEX bitvector(SIZE, 4 * sizeof(uint32_t), 248);

  for (int n = 0; n < SIZE / 4; n++) {
    bitvector.addBlock(arr + 4 * n);
  }

  vector<bool> used(SIZE, 1);
  for (int k = 0; k < 4 * SIZE; k++) {

    int num = rand() % SIZE;

    if (used[num] == false)
      continue;

    used[num] = false;

    int sum = 0;
    for (int n = 0; n < num; n++) {
      sum += used[n];
    }

    bitvector.deleteUint32_t(sum);

    int index = 0;
    for (int n = 0; n < SIZE; n++) {

      if (!used[n]) {
        continue;
      };
      ASSERT_EQ(arr[n], bitvector.accessUint32_t(index))
          << "n : " << n << " index : " << index << endl;
      index++;
    }
  }
}

TEST(COMBINE, DELETE_INSERT) {

  srand(42);

  for (int j = 0; j < 1000; j++) {
    // std::cout << "j : " << j << std::endl;
    int SIZE = 40;
    uint32_t arr[SIZE];

    for (int n = 0; n < SIZE; n++) {
      arr[n] = n;
    }

    FLEX bitvector(SIZE, 4 * sizeof(uint32_t), 248);

    for (int n = 0; n < SIZE / 4; n++) {
      bitvector.addBlock(arr + 4 * n);
    }

    vector<bool> used(SIZE, 1);
    for (int k = 0; k < 4 * SIZE; k++) {

      int num = rand() % SIZE;

      int sum = 0;
      for (int n = 0; n < num; n++) {
        sum += used[n];
      }

      if (used[num] == false) {
        used[num] = true;

        // std::cout << "Insert" << sum << std::endl;
        bitvector.insertUint32_t(sum, arr[num]);

      } else {
        used[num] = false;

        bitvector.deleteUint32_t(sum);
      }

      int index = 0;
      for (int n = 0; n < SIZE; n++) {

        if (!used[n]) {
          continue;
        };
        ASSERT_EQ(arr[n], bitvector.accessUint32_t(index))
            << "n : " << n << " index : " << index << endl;
        index++;
      }
    }
  }
}

TEST(COMBINE, DELETE_INSERT_2) {

  srand(42);

  for (int j = 0; j < 100; j++) {
    int SIZE = 400;
    uint32_t arr[SIZE];

    for (int n = 0; n < SIZE; n++) {
      arr[n] = n;
    }

    FLEX bitvector(SIZE, 4 * sizeof(uint32_t), 248);

    for (int n = 0; n < SIZE / 4; n++) {
      bitvector.addBlock(arr + 4 * n);
    }

    vector<bool> used(SIZE, 1);
    for (int k = 0; k < 4 * SIZE; k++) {

      int num = rand() % SIZE;

      int sum = 0;
      for (int n = 0; n < num; n++) {
        sum += used[n];
      }

      if (used[num] == false) {
        used[num] = true;

        bitvector.insertUint32_t(sum, arr[num]);

      } else {
        used[num] = false;
        bitvector.deleteUint32_t(sum);
      }

      int index = 0;
      for (int n = 0; n < SIZE; n++) {

        if (!used[n]) {
          continue;
        };
        ASSERT_EQ(arr[n], bitvector.accessUint32_t(index))
            << "n : " << n << " index : " << index << endl;
        index++;
      }
    }
  }
}

TEST(COMBINE, DELETE_INSERT_3) {

  srand(42);
  int SIZE = 4000;
  uint32_t arr[SIZE];

  for (int n = 0; n < SIZE; n++) {
    arr[n] = n;
  }

  FLEX bitvector(SIZE, 4 * sizeof(uint32_t), 248);

  for (int n = 0; n < SIZE / 4; n++) {
    bitvector.addBlock(arr + 4 * n);
  }

  vector<bool> used(SIZE, 1);
  for (int k = 0; k < 4 * SIZE; k++) {

    int num = rand() % SIZE;

    int sum = 0;
    for (int n = 0; n < num; n++) {
      sum += used[n];
    }

    if (used[num] == false) {
      used[num] = true;

      bitvector.insertUint32_t(sum, arr[num]);

    } else {
      used[num] = false;
      bitvector.deleteUint32_t(sum);
    }

    int index = 0;
    for (int n = 0; n < SIZE; n++) {

      if (!used[n]) {
        continue;
      };
      ASSERT_EQ(arr[n], bitvector.accessUint32_t(index))
          << "n : " << n << " index : " << index << endl;
      index++;
    }
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
