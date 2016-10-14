#ifndef FLEXIBLE_CACHE
#define FLEXIBLE_CACHE
#include <memory.h>
#include <vector>
#include <iostream>

#include "../avl/FlexibleAVLTree.cc"

using namespace std;


template <int N, class Compressor, class MemManager, class MemPointer>
class FlexibleCache {

  
  // typedef FlexibleAVLTree<Compressor, MemManager, MemPointer> FlexTree;
   typedef void FlexTree;

public:
  FlexibleAVLNode *ptrArr[N];
  char *chArr[N];
  char *buffer;


  // buffer for the FlexibleBitVector
  char *flexBuffer1;
  char *flexBuffer2;

  // startpos, int len
  pair<int, FlexTree*> startPos[N];

  MemManager *mem;
  int maxSize;

  FlexibleCache(int maxSize, MemManager *mem) : mem(mem), maxSize(maxSize) {
    buffer = new char[maxSize];

    flexBuffer1 = new char[maxSize];
    flexBuffer2 = new char[maxSize];

    for (int n = 0; n < N; n++) {
      ptrArr[n] = nullptr;
    }

    char *helpArr = new char[maxSize * N];
    for (int n = 0; n < N; n++) {
      chArr[n] = helpArr + n * maxSize;
    }
  }

  void push(char *buffer, FlexibleAVLNode *node, int beginPos, FlexTree* ptr) {
    makeFree();

    ptrArr[0] = node;
    startPos[0] = make_pair(beginPos, ptr);
    memcpy(chArr[0], buffer, node->len);
  }

  char *get(FlexibleAVLNode *node, int beginPos, FlexTree* ptr) {

    // check whether element is in the cache
    for (int n = 0; n < N; n++) {
      if (ptrArr[n] == node) {
        return chArr[n];
      }
    }

    // its not in the cache
    makeFree();

    // load the node in to the cache
    return load(node, beginPos, ptr);
  }

  // function for printing the cache
  void printCache() {
    printf("\n");
    for (int n = 0; n < N; n++) {
      printf("%d : %p", n, ptrArr[n]);
      if (ptrArr[n] != nullptr) {
        for (int m = 0; m < ptrArr[n]->len / sizeof(uint32_t); m++) {
          printf(" %d", ((int *)chArr[n])[m]);
        }
      }
      printf("\n");
    }
  }

  void updateStartPos(int refIndex, int addValue, FlexTree* ptr) {
    for (int n = 0; n < N; n++) {
      if (ptrArr[n] !=nullptr && startPos[n].first > refIndex && startPos[n].second == ptr) {
        startPos[n].first += addValue;
      }
    }
  }
  void loadIntoBuffer(char *ch, FlexibleAVLNode *node) {
    int foundIndex = -1;
    for (int n = 0; n < N; n++) {
      if (ptrArr[n] == node) {
        foundIndex = n;
      }
    }

    if (foundIndex != -1) {
      ////////// cout << "ELEMENT LOADED FOUND " << endl;
      // move it to the last slot
      // for (int n = foundIndex; n + 1 < N; n++) {
      //   swap(chArr[n], chArr[n + 1]);
      //   swap(ptrArr[n], ptrArr[n + 1]);
      // }
      memcpy(ch, chArr[foundIndex], ptrArr[foundIndex]->len);
      // delete the element
      // mem->remove(ptrArr[N - 1]->ptr);
      // ptrArr[N - 1] = nullptr;
      return;
    }

    // load the element from decompress
    MemPointer *ptrRight = node->ptr;
    mem->load(ptrRight, buffer);

    Compressor::decompress(buffer, ch, node->len, ptrRight->len);
    mem->remove(ptrRight);


    delete ptrRight;
  }

  void deletePointer(FlexibleAVLNode *node) {
    for (int n = 0; n < N; n++) {
      if (ptrArr[n] == node) {
        removeItemAtIndex(n);
        return;
      }
    }
  }

  void magic(FlexibleAVLNode *node1, FlexibleAVLNode *node2, int beginPos, FlexTree* ptr) {

    int index1 = -1;
    int index2 = -1;

    for (int n = 0; n < N; n++) {
      if (node1 == ptrArr[n]) {
        index1 = n;
      } else if (node2 == ptrArr[n]) {
        index2 = n;
      }
    }

    // cache invalidation:
    if (index1 == -1 || index2 == -1) {

      // element to be copied is in the cache
      if (index2 != -1) {
        ptrArr[index2] = node1;
        startPos[index2] = make_pair(beginPos, ptr);
      } else if (index1 != -1) {
        removeItemAtIndex(index1);
      }

      return;
    }

    swap(chArr[index1], chArr[index2]);
    swap(startPos[index1], startPos[index2]);
    return;
  }

  pair<FlexibleAVLNode *, int> getObjectByIndex(int pos, FlexTree* ptr) {
    for (int n = 0; n < N; n++) {
      if (ptrArr[n] != nullptr) {
        if ( ptr == startPos[n].second && pos >= startPos[n].first && pos < startPos[n].first + ptrArr[n]->len) {
          return make_pair(ptrArr[n], startPos[n].first);
        }
      }
    }
    return make_pair(nullptr, -1);
  }

private:
  // internal functions
  char *load(FlexibleAVLNode *node, int beginPos, FlexTree* ptr) {
    Compressor::decompress(mem->getMemoryLocation(node->ptr), chArr[0],
                           node->len, node->ptr->len);
    ptrArr[0] = node;
    startPos[0] = make_pair(beginPos, ptr);
    return chArr[0];
  }

  void removeItemAtIndex(int index) {
    ptrArr[index] = nullptr;

    for (int n = index; n + 1 < N; n++) {
      swap(ptrArr[n], ptrArr[n + 1]);
      swap(chArr[n], chArr[n + 1]);
      swap(startPos[n], startPos[n + 1]);
    }
  }

  // store the index in the real pointer array
  void store(int ourIndex) {

    FlexibleAVLNode *node = ptrArr[ourIndex];

    int size = Compressor::compress(chArr[ourIndex], buffer, node->len);

    if (node->ptr != nullptr) {
      mem->remove(node->ptr);
      delete node->ptr;
    } 

    node->ptr = mem->create(size);

    // fill the node with content
    mem->fill(node->ptr, buffer);
  }
  // make the cache such that one element is free and
  // return that index
  void makeFree() {

    int index = N - 1;

    for (int n = 0; n < N; n++) {
      if (ptrArr[n] == nullptr) {
        index = n;
        break;
      }
    }

    if (ptrArr[index] != nullptr) {
      store(index);
    }

    // swap such that the element is at the start position
    for (int n = index - 1; n >= 0; n--) {
      swap(chArr[n], chArr[n + 1]);
      swap(ptrArr[n], ptrArr[n + 1]);
      swap(startPos[n], startPos[n + 1]);
    }
  }
}

#endif

;
