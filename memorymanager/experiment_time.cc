#include "datastructures/SetBitVector.cc"
#include "datastructures/AdvancedSetBitVector.cc"
#include "datastructures/BlockBitVector.cc"
#include "compression/LZ4Compressor.cc"
#include "compression/LZ4CompressorHC.cc"
#include "compression/SimdCompCompressor.cc"
#include "compression/NoCompressor.cc"
#include "compression/SnappyCompressor.cc"
#include "compression/Simple9Compressor.cc"
#include <gtest/gtest.h>
#include "MyTimer.cc"
#include "datastructures/DynamicBitVector.cc"
#include "datastructures/FlexibleBitVector.cc"
#include "Helper.cc"

#include "MemoryManager.cc"
#include "SimpleMemoryManager.cc"
// #include "AdvancedMemoryManager.cc"
#include "functions/SimpleFunction.cc"
#include "functions/GeometricFunction.cc"

#include <limits.h>

#include <random>
#include <iostream>
#include <functional>
#include <fstream>
#include <stdio.h>

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

template <class Compressor>
using MemoryDynamicBitVector =
    DynamicBitVector<Compressor, MemoryManager, MemoryPointer>;

template <class Compressor>
using FlexibleDynamicBitVector =
    FlexibleBitVector<Compressor, MemoryManager, MemoryPointer>;

template <class Compressor>
using AdvancedBlockBitVector =
    BlockBitVector<Compressor, MemoryManager, MemoryPointer>;

using namespace std;

bool is_file_exist(string fileName) {
  std::ifstream infile(fileName);
  return infile.good();
}

const int M = 1 << 5;
char *ch[M];

void init(int N, int prob) {
  N = N / sizeof(uint32_t);


  MyTimer timer;


  string filename = std::to_string(N) + "-" + std::to_string(prob) + ".bin";

  N /= M;

  if (is_file_exist(filename)) {
    cout << filename <<endl;

    std::cout << "EXISTS" << std::endl;

    FILE *file = fopen(filename.c_str(), "rb");

    uint32_t **arr = new uint32_t *[M];
    for (int m = 0; m < M; m++) {
      arr[m] = new uint32_t[N];
      ch[m] = (char *)arr[m];

      fread(arr[m], sizeof(*arr[m]), N, file);
    }
    fclose(file);

  } else {

    std::cout << "DOES NOT EXIST" << std::endl;

    FILE *file = fopen(filename.c_str(), "wb");

    uint32_t **arr = new uint32_t *[M];
    // ch = (char *)arr;

    for (int m = 0; m < M; m++) {
      arr[m] = new uint32_t[N];
      ch[m] = (char *)arr[m];
      for (int n = 0; n < N; n++) {
        uint32_t res = 0;
        for (int k = 0; k < 32; k++) {
          res <<= 1;
          if ((rand() % 1000) < prob) {
            res++;
          }
        }
        arr[m][n] = res;
      }
      fwrite(arr[m], sizeof(*arr[m]), N, file);
    }
    fclose(file);
  }

  timer.printTime("INIT ");
}

template <class MyBitVector>
void test(int blockSize, int chunkSize, int totalSize, int doRandom) {

  long long orig_vm, orig_rs;
  Helper::process_mem_usage(orig_vm, orig_rs);

  cout <<"ORIGINAL "<<orig_vm<<" "<<orig_rs<<endl;


  cout <<" RANDOM : "<<doRandom<<endl;
  int LEN = (totalSize + 1) / 5000;

  vector<int> v = vector<int>(LEN + 1);
  vector<int> v2 = vector<int>(LEN + 1);

  long long vm1, rs1;
  Helper::process_mem_usage(vm1, rs1);

  MyBitVector *bitvector =
      new MyBitVector(totalSize / blockSize, blockSize, chunkSize);

  assert(totalSize % blockSize == 0 && totalSize >= blockSize);



  MyTimer timer;

  int N = totalSize / M;

  for (int m = 0; m < M; m++) {
    for (int n = 0; n < N / blockSize; n++) {
      bitvector->addBlock(ch[m] + n * blockSize);
    }
  }

  long long vm2, rs2;
  Helper::process_mem_usage(vm2, rs2);

  cout <<" SPACE " << (vm2-vm1) <<" "<<(rs2 -rs1)<<endl;



  int DIV = bitvector->rank(totalSize-1);


  if (doRandom == 1) {
    for (int n = 0; n < LEN; n++) {
      v2[n] = rand() % DIV;
      v[n] = rand() % totalSize;
    }
  } else {
    for (int n = 0; n < LEN; n++) {
      v2[n] = n % DIV;
      v[n] = n;
    }
  }




    cout << "totalSize  " << totalSize << endl;
  cout << "numOnes    " << bitvector->rank(totalSize-1) <<" LEN "<<LEN<< endl;

  timer.printTime("BLOCK");
  // timer.print("INIT_TIME");


    for (int n = 0; n < LEN; n++) {
      int pos = v[n];
      bitvector->access(pos);
    }

  timer.printTime("ACCESS ");

  cout <<" LEN : "<<LEN<<endl;
  for (int n = 0; n < LEN; n++) {
    int pos = v2[n];
    bitvector->select(pos);
  }

  // timer.print("SELECT_TIME");
  timer.printTime("SELECT ");

  for (int n = 0; n < LEN; n++) {
    int pos = v[n];
    bitvector->rank(pos);
  }

  timer.printTime("RANK ");

  for (int n = 0; n < LEN; n++) {
    int pos = v[n];
    int bit = v[n + 1] & 1;
    bitvector->modify(pos, bit);
  };

  timer.printTime("MODIFY ");
}

template <class Compressor>
void decideStructure(int blockSize, int chunkSize, int totalSize,
                     string structure, int doRandom) {
  // this will be to slow to ! TODO do
  if (structure.compare("avlIndex") == 0) {
    test<SetBitVector<Compressor>>(blockSize, chunkSize, totalSize, doRandom);
  } else if (structure.compare("avlInt") == 0) {
    test<AdvancedSetBitVector<Compressor>>(blockSize, chunkSize, totalSize,
                                           doRandom);

  } else if (structure.compare("avlBlock") == 0) {
    test<AdvancedBlockBitVector<Compressor>>(blockSize, chunkSize, totalSize,
                                             doRandom);

  } else if (structure.compare("segmentTree") == 0) {
    test<MemoryDynamicBitVector<Compressor>>(blockSize, chunkSize, totalSize,
                                             doRandom);
  } else if (structure.compare("flexible") == 0) {
    test<FlexibleDynamicBitVector<Compressor>>(blockSize, chunkSize, totalSize,
                                               doRandom);
  } else {
    cout << "structure with name : ' " << structure << "' not found" << endl;
  }
}

int main(int argc, char **argv) {

  if (argc != 1 + 7) {
    std::cout
        << "USAGE : experiment-out structure compression powBlockSize "
        << "powChunkSize powTotal probability (0-1000) doRandom" << std::endl
        << "structure : avlIndex | avlInt | avlBlock | segmentTree | flexible"
        << endl
        << "compression: nocomp lz4 lz4hc simd snappy simple9" << endl;
    return 0;
  }

  string structure = string(argv[1]);
  string compression = string(argv[2]);
  int blockSize = 1 << atoi(argv[3]);
  int chunkSize = 1 << atoi(argv[4]);
  int totalSize = 1 << atoi(argv[5]);
  int prob = atoi(argv[6]);
  int doRandom = atoi(argv[7]);

  srand(42);

  init(totalSize, prob);

  if (compression.compare("nocomp") == 0) {
    decideStructure<NoCompressor>(blockSize, chunkSize, totalSize, structure,
                                  doRandom);
  } else if (compression.compare("lz4") == 0) {
    decideStructure<LZ4Compressor>(blockSize, chunkSize, totalSize, structure,
                                   doRandom);
  } else if (compression.compare("lz4hc") == 0) {
    decideStructure<LZ4CompressorHC>(blockSize, chunkSize, totalSize, structure,
                                     doRandom);
  } else if (compression.compare("simd") == 0) {
    decideStructure<SimdCompCompressor>(blockSize, chunkSize, totalSize,
                                        structure, doRandom);
  } else if (compression.compare("snappy") == 0) {
    decideStructure<SnappyCompressor>(blockSize, chunkSize, totalSize,
                                      structure, doRandom);
  } else if (compression.compare("simple9") == 0) {
    decideStructure<Simple9Compressor>(blockSize, chunkSize, totalSize,
                                       structure, doRandom);
  }
}
