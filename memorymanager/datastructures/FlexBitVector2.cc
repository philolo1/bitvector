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
#include "../avl/FlexibleAVLTree.cc"
#include <tuple>
#include "../defines.h"
#include "../MemoryProvider.cc"

#include <stdexcept>
#include "FlexibleCache.cc"
using namespace std;

/*
 * This class uses a set to simulate bits
 */
template <class Compressor, class MemManager, class MemPointer>
class FlexibleBitVector {

  typedef FlexibleAVLTree<Compressor, MemManager, MemPointer> FlexTree;
  typedef FlexibleCache<CACHE_SIZE, Compressor, MemManager, MemPointer>
      FlexCache;

  MemManager *mem;
  char *outBuffer;
  char *buffer;
  int bitElementCounter;
  int chunkSize;
  FlexTree *tree;
  FlexCache *cache;

public:
  int getSize() { return bitElementCounter / (INT_BIT); }

  // number of blocks that we need
  FlexibleBitVector(int numberOfBlocks, int chunkSize, int S) {

    if (chunkSize / sizeof(uint32_t) * sizeof(uint32_t) != chunkSize) {
      throw "chunkSize needs to be a multipe of sizeof(uint32_t";
    }

    (void)numberOfBlocks;

    this->chunkSize = chunkSize;

    // factor 2 because we want to add insert and delete
    int maxSize = Compressor::getMaxOutBuffer(3 * chunkSize);

    MemoryProvider provider;
    mem = provider.getMemoryManager(max(S, maxSize), maxSize);
    // mem = new MemManager(max(S, maxSize), maxSize);
    bitElementCounter = 0;

    // nothing should be bigger than
    buffer = new char[maxSize];
    outBuffer = new char[maxSize];
    cache = new FlexibleCache<CACHE_SIZE, Compressor, MemManager, MemPointer>(
        maxSize, mem);

    tree = new FlexTree(cache);
  }

  void addBlock(void *block) {

    int numOnes = 0;
    int *intArr = (int *)block;
    int len = chunkSize;
    while (len > 0) {
      numOnes += Helper::countInt(intArr[0]);
      intArr++;
      len -= INT_BIT / CHAR_BIT;
    }

    FlexibleAVLNode *node =
        tree->insert(bitElementCounter, nullptr, numOnes, chunkSize);

    cache->push((char *)block, node);

    bitElementCounter += chunkSize * CHAR_BIT;
  }

  uint32_t accessUint32_t(int pos) {

    pair<FlexibleAVLNode *, int> res = tree->find(pos * INT_BIT);

    FlexibleAVLNode *node = res.first;

    if (node == nullptr) {
      return 0;
    }

    outBuffer = cache->get(node);

    int startIndex = res.second;
    startIndex /= INT_BIT;

    pos -= startIndex;

    uint32_t *intBuffer = (uint32_t *)outBuffer;

    return intBuffer[pos];
  }

  // get the i-th bit
  int access(int pos) {
    pair<FlexibleAVLNode *, int> res = tree->find(pos);
    FlexibleAVLNode *node = res.first;

    if (node == nullptr) {
      return 0;
    }

    outBuffer = cache->get(node);

    pos -= get<1>(res);

    int posIndex = pos / CHAR_BIT;
    int posPos = pos % CHAR_BIT;

    return (outBuffer[posIndex] >> posPos) & 1;
  }

  void deleteUint32_tSimple(int pos, FlexibleAVLNode *node, int startIndex) {

    outBuffer = cache->get(node);

    pos -= startIndex;

    int posIndex = pos / CHAR_BIT;
    int posPos = pos % CHAR_BIT;

    uint32_t *intBuffer = (uint32_t *)(outBuffer + posIndex);
    int subtract = Helper::countInt(intBuffer[0]);

    // insert the number here
    memmove(outBuffer + posIndex, outBuffer + posIndex + sizeof(uint32_t),
            node->len - posIndex - sizeof(uint32_t));

    // update the length of the node
    node->len -= sizeof(uint32_t);

    // find the path to the node and update the size accordingly
    tree->updatePath2(startIndex, -subtract,
                      -(CHAR_BIT * (int)sizeof(uint32_t)));
  }

  void deleteUint32_t(int pos) {

    pos *= INT_BIT;

    pair<FlexibleAVLNode *, int> res = tree->find(pos);

    FlexibleAVLNode *node = res.first;

    if (node == nullptr) {
      throw std::runtime_error(std::string("wrong call on insert "));
    }

    if (node->len - sizeof(uint32_t) > chunkSize / 2) {

      deleteUint32_tSimple(pos, res.first, res.second);
    } else {

      // we want to merge / delete

      pair<FlexibleAVLNode *, int> resNode =
          tree->find(pos + CHAR_BIT * node->len);

      pair<FlexibleAVLNode *, int> leftNode, rightNode;

      if (resNode.first == nullptr) {
        // find right neighbor
        resNode = tree->find(res.second - 1);

        // no neighbor, just one node, we cannot do anything here
        if (resNode.first == nullptr) {
          deleteUint32_tSimple(pos, res.first, res.second);
          return;
        }
        leftNode = resNode;
        rightNode = res;

      } else {
        leftNode = res;
        rightNode = resNode;
      }

      outBuffer = cache->get(leftNode.first);

      int *myIntBuffer = (int *)outBuffer;
      cache->loadIntoBuffer(outBuffer + leftNode.first->len, rightNode.first);

      // remove the posIndex item
      pos -= leftNode.second;

      int posIndex = pos / CHAR_BIT;

      uint32_t *intBuffer = (uint32_t *)(outBuffer + posIndex);
      int deletedOnes = Helper::countInt(intBuffer[0]);

      // insert the number here
      memmove(outBuffer + posIndex, outBuffer + posIndex + sizeof(uint32_t),
              leftNode.first->len + rightNode.first->len - posIndex -
                  sizeof(uint32_t));

      int lenLeft = leftNode.first->len;
      int lenRight = rightNode.first->len;

      int sumOnes =
          leftNode.first->numOnes + rightNode.first->numOnes - deletedOnes;

      if (res == leftNode) {
        lenLeft -= sizeof(uint32_t);
      } else {
        lenRight -= sizeof(uint32_t);
      }

      // delete the numOnes of the node and tell every node right of it that it
      // is now one smaller
      tree->updatePath2(rightNode.second, -rightNode.first->numOnes,
                        -(CHAR_BIT * (int)sizeof(uint32_t)));

      int rightNodeNumberOnes = rightNode.first->numOnes;
      tree->deleteElem(rightNode.second);

      // print();

      // now decide what we do
      if (lenLeft + lenRight <= chunkSize + chunkSize / 2) {

        // if they are just 1.5 or smaller we just put everything on the
        // left Node

        FlexibleAVLNode *treeNode = leftNode.first;

        treeNode->len = lenLeft + lenRight;

        tree->updatePath2(leftNode.second, rightNodeNumberOnes - deletedOnes,

                          0);
      } else {

        // load only half of ptrOne
        // load the other half to ptrTwo
        FlexibleAVLNode *treeNode = leftNode.first;

        int newLen1 =
            (((lenLeft + lenRight) / 2) / sizeof(uint32_t)) * sizeof(uint32_t);

        int newLen2 = lenLeft + lenRight - newLen1;

        treeNode->len = newLen1;

        int oneCount = 0;
        uint32_t *myIntBuffer = (uint32_t *)outBuffer;

        for (int n = 0; n < newLen1 / sizeof(uint32_t); n++) {
          oneCount += Helper::countInt(myIntBuffer[0]);
          myIntBuffer++;
        }

        tree->updatePath2(leftNode.second, oneCount - leftNode.first->numOnes,
                          0);

        FlexibleAVLNode *newNode =
            tree->insert(leftNode.second + CHAR_BIT * newLen1, nullptr,
                         sumOnes - oneCount, newLen2);

        cache->push(outBuffer + newLen1, newNode);
        // now add the new element
      }
    }

    bitElementCounter -= INT_BIT;

  } // end of deleteUint32_t

  void print() {
    printf("CACHE\n");
    cache->printCache();
    printf("\n\nROOT\n");
    tree->print();
  }

  // int pos
  void insertUint32_t(int pos, uint32_t num) {

    int dataLen = 1;
    char *data = (char *)&num;

    uint32_t addNum = 0;
    for (int n = 0; n < dataLen; n++) {
      addNum = Helper::countInt(((uint32_t *)data)[n]);
    }

    pos *= INT_BIT;

    pair<FlexibleAVLNode *, int> res = tree->find(pos);

    // special case insert at the end of the last array
    if (pos == bitElementCounter) {
      if (pos > 0) {
        res = tree->find(pos - INT_BIT);
      } else {

        FlexibleAVLNode *node = tree->insert(bitElementCounter, nullptr, addNum,
                                             (dataLen * sizeof(uint32_t)));

        cache->push(data, node);

        bitElementCounter += dataLen * INT_BIT;
        return;
      }
    }

    FlexibleAVLNode *node = res.first;

    if (node == nullptr) {
      throw std::runtime_error(std::string("wrong call on insert "));
    }

    outBuffer = cache->get(node);

    pos -= get<1>(res);

    int posIndex = pos / CHAR_BIT;
    int posPos = pos % CHAR_BIT;

    // insert the number here
    memmove(outBuffer + posIndex + (dataLen * sizeof(uint32_t)),
            outBuffer + posIndex, node->len - posIndex);

    uint32_t *intBuffer = (uint32_t *)(outBuffer + posIndex);
    uint32_t *intData = (uint32_t *)data;

    for (int n = 0; n < dataLen; n++) {
      intBuffer[n] = intData[n];
    }

    if (node->len + (dataLen * sizeof(uint32_t)) < 2 * chunkSize) {

      // update the length of the node
      node->len += dataLen * sizeof(uint32_t);

      // find the path to the node and update the size accordingly
      tree->updatePath2(get<1>(res), addNum,
                        dataLen * CHAR_BIT * sizeof(uint32_t));

    } else {

      // update the length of the node
      node->len += dataLen * sizeof(uint32_t);

      int len = node->len;

      // this is now chunkSize
      node->len = len / 2;

      // update the pointer of the node
      int sumFirstHalf = 0;

      uint32_t *intBuffer2 = (uint32_t *)outBuffer;

      // Calculate the sum of the fist half of bits
      for (int n = 0; n < node->len / sizeof(uint32_t); n++) {
        sumFirstHalf += Helper::countInt(intBuffer[0]);
        intBuffer++;
      }

      int diffBits = sumFirstHalf - node->numOnes;
      int diffIndex = node->len - (len - dataLen * sizeof(uint32_t));

      int sumSecondHalf = node->numOnes + Helper::countInt(num) - sumFirstHalf;

      // cutt of the part of the node
      tree->updatePath2(res.second, diffBits,
                        dataLen * CHAR_BIT * sizeof(uint32_t));

      len = len - node->len;

      int start = res.second + (node->len * CHAR_BIT);
      FlexibleAVLNode *newNode =
          tree->insert(start, nullptr, sumSecondHalf, len);

      cache->push(outBuffer + node->len, newNode);
    }

    bitElementCounter += INT_BIT * dataLen;
  } // end of insertUint32_t(int pos)

  // modify element[pos] to either 1 or 2
  void modify(int pos, int val) {

    pair<FlexibleAVLNode *, int> res = tree->find(pos);
    FlexibleAVLNode *node = res.first;

    if (get<0>(res) == nullptr) {
      return;
    }

    outBuffer = cache->get(node);

    pos -= get<1>(res);

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

      int add = val ? 1 : -1;

      // find the path to the node and update the size accordingly
      tree->updatePath(get<1>(res), add);
    }
  }

  void modifyUint32_t(int pos, uint32_t val) {
    pair<FlexibleAVLNode *, int> res = tree->find(pos * INT_BIT);

    FlexibleAVLNode *node = res.first;

    if (get<0>(res) == nullptr) {
      throw std::runtime_error(
          std::string("the element you want to modify does not exist"));
    }

    outBuffer = cache->get(node);

    int startIndex = res.second;
    startIndex /= INT_BIT;
    pos -= startIndex;

    uint32_t *intBuffer = (uint32_t *)outBuffer;

    int add = Helper::countInt(val) - Helper::countInt(intBuffer[pos]);

    intBuffer[pos] = val;

    // find the path to the node and update the size accordingly
    tree->updatePath(get<1>(res), add);
  }

  // at what position is the ith bit set
  int select(int num) {

    tuple<int, FlexibleAVLNode *, int> tuple = tree->select(num);

    if (get<1>(tuple) == nullptr) {
      return -1;
    }

    FlexibleAVLNode *node = get<1>(tuple);

    outBuffer = cache->get(node);

    uint32_t *intArr = (uint32_t *)outBuffer;
    num = get<0>(tuple);

    int res = get<2>(tuple);
    while (true) {
      int help = Helper::countInt(intArr[0]);
      if (help >= num) {
        return res + Helper::getXOnePos(intArr[0], num);
      } else {
        num -= help;
        res += 32;
        intArr++;
      }
    }

    return -1;
  }

  int rank(int pos) {

    tuple<int, FlexibleAVLNode *, int> tuple = tree->rank(pos);

    if (get<1>(tuple) == nullptr) {
      return get<0>(tuple);
    }

    FlexibleAVLNode *node = get<1>(tuple);

    outBuffer = cache->get(node);

    uint32_t *intArr = (uint32_t *)outBuffer;
    int elements = get<0>(tuple);
    int num = get<2>(tuple);

    while (num >= 32) {
      elements += Helper::countInt(intArr[0]);
      intArr++;
      num -= 32;
    }

    if (num >= 31) {
      return elements + Helper::countInt(intArr[0]);
    } else {
      int help = (1 << (num + 1)) - 1;
      return elements + Helper::countInt(intArr[0] & help);
    }
  }
};
