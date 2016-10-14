/** author : Patrick Klitzke
 *
 * class for representing a dynamic bitvector
 * with
 * access
 * modify
 * rank
 * select
 */

#ifndef SET_BITVECTOR
#define SET_BITVECTOR

#include <set>
#include <vector>
#include <string>
#include <iostream>
#include "../avl/AVLTree.cpp"

using namespace std;

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

/*
 * This class uses a set to simulate bits
 */
template <class T> class SetBitVector {

  AVLTree *tree;
  int chunkSize;
  long long index;

public:
  SetBitVector(int numberOfBlocks, int chunkSize)
      : SetBitVector(numberOfBlocks, chunkSize, chunkSize) {}

  // number of blocks that we need
  SetBitVector(int numberOfBlocks, int chunkSize, int S) {
    this->chunkSize = chunkSize;
    this->tree = new AVLTree();
    index = 0;
  }

  void addBlock(char *arr) {
    for (int n = 0; n < chunkSize; n++) {
      for (int m = 0; m < CHAR_BIT; m++) {
        if ((arr[n] >> m) & 1) {
          this->tree->insert(index);
        }
        index++;
      }
    }
  }

  // get the i-th bit
  int access(int pos) { return tree->find(pos); }

  // modify element[pos] to either 1 or 2
  void modify(int pos, int val) {
    if (tree->find(pos) != val) {
      if (val) {
        tree->insert(pos);
      } else {
        tree->deleteElem(pos);
      }
    }
  }

  // at what position is the ith bit set
  int select(int num) { return tree->select(num); }

  int rank(int pos) { return tree->rank(pos); }
};

#endif
