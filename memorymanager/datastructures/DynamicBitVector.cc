/** author : Patrick Klitzke
 *
 * class for representing a dynamic bitvector
 * with
 * access
 * modify
 * rank
 * select
 */
#include <set>
#include <vector>
#include <string>
#include <iostream>
#include "../Helper.cc"
#include "SimpleCache.cc"
#include "../defines.h"

#ifndef INT_BIT
#define INT_BIT sizeof(uint32_t) * CHAR_BIT
#endif

using namespace std;

/*
 * This class uses a set to simulate bits
 */
template <class Compressor, class MemManager, class MemPointer>
class DynamicBitVector {

  MemManager *mem;
  vector<MemPointer *> ptrList;

  char *buffer;
  char *outBuffer;

  int numberOfBlocks;
  int chunkSize;

  // numberOfOnes
  vector<long long> numberOfOnes;
  int counter = 0; // the number of blocks we have
  SimpleCache<CACHE_SIZE, Compressor, MemManager, MemPointer> *simpleCache;

  MemPointer *lastPointer;
  MemPointer *lastPointer2;
  char *lastResult;
  char *lastResult2;

public:
  inline int roundToPowerOf2(int num) {
    int res = 1;
    while (res < num)
      res *= 2;
    return res;
  }

  // number of blocks that we need
  DynamicBitVector(int numberOfBlocks, int chunkSize, int S) {

    if (chunkSize / sizeof(uint32_t) * sizeof(uint32_t) != chunkSize) {
      throw "chunkSize needs to be a multipe of sizeof(uint32_t";
    }

    this->numberOfBlocks = roundToPowerOf2(numberOfBlocks);
    this->chunkSize = chunkSize;

    numberOfOnes = vector<long long>(2 * this->numberOfBlocks - 1, 0);

    int maxSize = Compressor::getMaxOutBuffer(chunkSize);
    mem = new MemManager(max(S, maxSize), maxSize);
    buffer = new char[maxSize];
    outBuffer = new char[maxSize];
    lastResult = new char[maxSize];

    lastResult2 = new char[maxSize];
    simpleCache =
        new SimpleCache<CACHE_SIZE, Compressor, MemManager, MemPointer>(
            maxSize, chunkSize, ptrList, mem);
  }

  void addBlock(void *block) {
    // ASSUMPTION USE INTS
    int index = counter + numberOfBlocks - 1;
    counter++;

    int val = 0;

    uint32_t *intArray = (uint32_t *)block;
    for (int n = 0; n < chunkSize / sizeof(uint32_t); n++) {
      val += Helper::countInt(intArray[n]);
    }

    numberOfOnes[index] = val;

    simpleCache->push((char *)block);

    // int size = Compressor::compress(block, buffer, chunkSize);
    // MemPointer *ptr = mem->create(size);
    // mem->fill(ptr, buffer);
    // ptrList.push_back(ptr);

    // propagate up
    while (index > 0) {
      index = (index - 1) / 2;
      numberOfOnes[index] += val;
    };
  }

  // get the i-th bit
  int access(int pos) {
    // select the bit number n
    int chunkIndex = pos / (chunkSize * CHAR_BIT);
    pos = pos % (chunkSize * CHAR_BIT);

    outBuffer = simpleCache->get(chunkIndex);

    int posIndex = pos / CHAR_BIT;
    int posPos = pos % CHAR_BIT;

    return (outBuffer[posIndex] >> posPos) & 1;
  }

  uint32_t accessUint32_t(int pos) {
    // select the bit number n
    int chunkIndex = pos / (chunkSize / sizeof(uint32_t));
    pos = pos % (chunkSize / sizeof(uint32_t));

    outBuffer = simpleCache->get(chunkIndex);
    // MemPointer *ptr = ptrList[chunkIndex];
    // outBuffer = decompress(ptr, chunkSize);
    // mem->load(ptr, buffer);
    // Compressor::decompress(buffer, outBuffer, chunkSize, ptr->len);

    return outBuffer[pos];
  }

  // modify element[pos] to either 1 or 2
  void modify(int pos, int val) {
    // select the bit number n
    int chunkIndex = pos / (chunkSize * CHAR_BIT);
    pos = pos % (chunkSize * CHAR_BIT);

    // MemPointer *ptr = ptrList[chunkIndex];
    // outBuffer = decompress(ptr, chunkSize);

    outBuffer = simpleCache->get(chunkIndex);

    // mem->load(ptr, buffer);
    // Compressor::decompress(buffer, outBuffer, chunkSize, ptr->len);

    int posIndex = pos / CHAR_BIT;
    int posPos = pos % CHAR_BIT;

    int outVal = (outBuffer[posIndex] >> posPos) & 1;

    if (outVal != val) {
      if (val == 1) {
        outBuffer[posIndex] |= 1 << posPos;
      } else {
        outBuffer[posIndex] &= ~(1 << posPos);
      }

      // compress again
      // int size = Compressor::compress(outBuffer, buffer, chunkSize);
      // mem->remove(ptr);
      // delete ptr;
      // ptr = mem->create(size);
      // mem->fill(ptr, buffer);
      // ptrList[chunkIndex] = ptr;
      //

      // updates the segment tree
      int bufferIndex = numberOfBlocks - 1 + chunkIndex;

      int add = val ? 1 : -1;

      while (true) {
        numberOfOnes[bufferIndex] += add;
        if (bufferIndex <= 0)
          return;
        bufferIndex = (bufferIndex - 1) / 2;
      }
    }
  }

  // pos : starting position as an int
  void modifyUint32_t(int pos, uint32_t val) {
    // select the bit number n
    int chunkIndex = pos / (chunkSize / sizeof(uint32_t));
    pos = pos % (chunkSize / sizeof(uint32_t));

    outBuffer = simpleCache->get(chunkIndex);
    // MemPointer *ptr = ptrList[chunkIndex];
    //
    // outBuffer = decompress(ptr, chunkSize);
    // mem->load(ptr, buffer);
    // Compressor::decompress(buffer, outBuffer, chunkSize, ptr->len);

    uint32_t *intBuffer = (uint32_t *)outBuffer;

    int add = Helper::countInt(val) - Helper::countInt(intBuffer[pos]);
    intBuffer[pos] = val;

    // compress again
    // int size = Compressor::compress(outBuffer, buffer, chunkSize);
    // mem->remove(ptr);
    // delete ptr;
    // ptr = mem->create(size);
    // mem->fill(ptr, buffer);
    // ptrList[chunkIndex] = ptr;

    // updates the segment tree
    int bufferIndex = numberOfBlocks - 1 + chunkIndex;

    while (true) {
      numberOfOnes[bufferIndex] += add;
      if (bufferIndex <= 0)
        return;
      bufferIndex = (bufferIndex - 1) / 2;
    }
  }

  // at what position is the ith bit set
  int select(int num) {
    // we begin with index = 0 for the first but actually index = 1
    // is better to deal with
    // cout<<"NUM : "<<num<<endl;
    // cout<<"NUM : "<<numberOfOnes.size()<<endl;
    num++;

    int index = 0;
    int maxIndex = numberOfOnes.size();

    // cout<<"MAXINDEX " <<maxIndex<<endl;

    // walk down the tree
    while (2 * index + 2 <= maxIndex) {
      if (numberOfOnes[2 * index + 1] < num) {
        num -= numberOfOnes[2 * index + 1];
        index = 2 * index + 2;
      } else {
        index = 2 * index + 1;
      }
    }

    // there is no solution if we search
    // for more ones then there are
    if (numberOfOnes[index] < num) {
      return -1;
    }

    long long position =
        (index - (numberOfBlocks - 1)) * ((long long)chunkSize);

    position *= CHAR_BIT;

    int ptrIndex = index - (numberOfBlocks - 1);
    outBuffer = simpleCache->get(ptrIndex);

    // MemPointer *ptr = ptrList[ptrIndex];
    // outBuffer = decompress(ptr, chunkSize);
    // mem->load(ptr, buffer);
    // Compressor::decompress(buffer, outBuffer, chunkSize, ptr->len);

    uint32_t *intArray = (uint32_t *)outBuffer;
    int helpVal = Helper::countInt(intArray[0]);

    // find the right int
    while (helpVal < num) {
      num -= helpVal;
      intArray++;
      position += INT_BIT;
      helpVal = Helper::countInt(intArray[0]);
    }

    // find the k-th set bit
    return position + Helper::innerRank(intArray[0], num);
  }

  int rank(int pos) {
    // 1. find the block
    int blockIndex = pos / (chunkSize * CHAR_BIT);
    pos = pos % (chunkSize * CHAR_BIT);

    outBuffer = simpleCache->get(blockIndex);
    // MemPointer *ptr = ptrList[blockIndex];
    // outBuffer = decompress(ptr, chunkSize);
    // mem->load(ptr, buffer);
    // Compressor::decompress(buffer, outBuffer, chunkSize, ptr->len);

    long long numCounter = 0;

    uint32_t *intArray = (uint32_t *)outBuffer;

    while (pos >= INT_BIT) {
      pos -= INT_BIT;
      numCounter += Helper::countInt(intArray[0]);
      intArray++;
    }

    uint32_t help = intArray[0];

    // 2. find the inner rank
    for (int num = 0; num <= pos; num++) {
      numCounter += (help >> num) & 1;
    }
    blockIndex = blockIndex + numberOfBlocks - 1;

    // look in the segement tree
    while (blockIndex != 0) {
      int help = (blockIndex - 1) / 2;
      // if it is the right child, we add the value of the left child
      if ((help * 2 + 2) == blockIndex) {
        numCounter += numberOfOnes[help * 2 + 1];
      }
      blockIndex = (blockIndex - 1) / 2;
    }

    return numCounter;
  }
};
