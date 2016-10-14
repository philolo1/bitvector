/** author : Patrick Klitzke
 *
 * class for representing a dynamic bitvector
 * with
 * access
 * modify
 * rank
 * select
 */
#include <iostream>
#include "../avl/AdvancedAVLTree.cpp"
#include <vector>

using namespace std;

#ifndef CHAR_BIT
  #define CHAR_BIT 8
#endif


/*
 * This class uses a set to simulate bits
 */
template <class T>
class AdvancedSetBitVector {

  AdvancedAVLTree *tree;
  int chunkSize;
  long long index;

  int counter=0;

  public:


  AdvancedSetBitVector(int numberOfBlocks, int chunkSize) :
    
    AdvancedSetBitVector(numberOfBlocks, chunkSize, chunkSize) {
    }

  // number of blocks that we need
  AdvancedSetBitVector(int numberOfBlocks, int chunkSize, int S) {
    (void)S;
    (void)numberOfBlocks;
    this->chunkSize = chunkSize;
    this->tree = new AdvancedAVLTree();
    index = 0;
  }

  void addBlock(char *arr) {

    int* intArr = (int *)arr;

    for (int n=0; n < chunkSize/ sizeof(int); n++) {
        tree->insert(counter*chunkSize * CHAR_BIT + n* 32, intArr[n]);
      }

    counter++;
  }

  // get the i-th bit
  int access(int pos) {
    return tree->find(pos);
  }

  // modify element[pos] to either 1 or 2
  void modify(int pos, int val) {
    tree->modify(pos, val);
  }

  // at what position is the ith bit set
  int select(int num) {
    return tree->select(num);
  }

  int rank(int pos) {
    return tree->rank(pos);
  }
};
