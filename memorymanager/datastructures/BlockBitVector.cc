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
#include "../avl/BlockAVLTree.cc"

#ifndef INT_BIT
#define INT_BIT sizeof(uint32_t) * CHAR_BIT 
#endif


using namespace std;

/*
 * This class uses a set to simulate bits
 */
template <class Compressor, class MemManager, class MemPointer>
class BlockBitVector {

  MemManager *mem;
  char *outBuffer;
  char *buffer;
  int counter;
  int chunkSize;
  BlockAVLTree *tree;

  public:

  // number of blocks that we need
  BlockBitVector(int numberOfBlocks, int chunkSize, int S) {

    if (chunkSize / sizeof(uint32_t) * sizeof(uint32_t) != chunkSize) {
      throw "chunkSize needs to be a multipe of sizeof(uint32_t";
    }

    (void)numberOfBlocks;

    this->chunkSize = chunkSize;

    tree = new BlockAVLTree();


    int maxSize = Compressor::getMaxOutBuffer(chunkSize);
    mem = new MemManager(max(S, maxSize), maxSize);
    counter = 0 ;

    buffer = new char[maxSize];
    outBuffer = new char[maxSize];
  }
 

  void addBlock(void* block) {

////// std::cout<<"p add block"<<std::endl;



    int numOnes = 0;
    int *intArr = (int *) block;
    int len = chunkSize;
    while (len > 0) {
      numOnes += Helper::countInt(intArr[0]);
      intArr++;
      len -= INT_BIT / CHAR_BIT;
    }
////// std::cout<<"end while"<<std::endl;



    int size = Compressor::compress(block, buffer, chunkSize);
    MemPointer* ptr = mem->create(size);
    mem->fill(ptr, buffer);

////// std::cout<<"p insert"<<std::endl;

////// cout<<"numberOnes : "<<numOnes<<endl;

    tree->insert(
        counter,
        ptr,
        numOnes,
        chunkSize
    );
//cout<<"ptrlen : "<<ptr->len<<endl;
//cout<<"chunksize : "<<(chunkSize * CHAR_BIT)<<endl;

////// std::cout<<"p after insert"<<std::endl;


    counter += chunkSize * CHAR_BIT;
  }

  // get the i-th bit
  int access(int pos) {
    BlockAVLNode *node = tree->find(pos);

    if (node == NULL) {
      return 0;
    } 

    MemoryPointer *ptr = node->ptr;
    mem->load(ptr, buffer);

    // decompress block
    Compressor::decompress(buffer, outBuffer, chunkSize, ptr->len);

    pos -= node->elem;

// cout<<"pos : "<<pos<<endl;

    int posIndex = pos / CHAR_BIT;
    int posPos = pos % CHAR_BIT;

    return (outBuffer[posIndex] >> posPos) & 1;
  }

  // modify element[pos] to either 1 or 2
  void modify(int pos, int val) {

    BlockAVLNode *node = tree->find(pos);

    if (node == NULL) {
      return;
    } 


    MemoryPointer *ptr = node->ptr;
    mem->load(ptr, buffer);

    // decompress block
    Compressor::decompress(buffer, outBuffer, chunkSize, ptr->len);


    pos -= node->elem;

    int posIndex = pos / CHAR_BIT;
    int posPos = pos % CHAR_BIT;
    

    int outVal = (outBuffer[posIndex] >> posPos) & 1;

    
    if (outVal != val) {
      if (val == 1) {
        outBuffer[posIndex] |= 1 << posPos;
      } else {
        outBuffer[posIndex] &= ~(1 <<posPos);
      }
    
      // compress again 
      int size = Compressor::compress(outBuffer, buffer, chunkSize);
      mem->remove(ptr);
////// std::cout<<"p remove"<<std::endl;
      ptr = mem->create(size);
      mem->fill(ptr, buffer);
      node->ptr = ptr;


      int add = val ? 1 : -1;
      // find the path to the node and update the size accordingly
      tree->updatePath(node->elem, add);
    }
  }

  // at what position is the ith bit set
  int select(int num) {
   
    pair<int, BlockAVLNode*> pair = tree->select(num);

    if (pair.second == NULL) {
      return -1;
    }

    BlockAVLNode *tree = pair.second;
    MemPointer *ptr = tree->ptr;
    mem->load(ptr, buffer);
    Compressor::decompress(buffer, outBuffer, chunkSize, ptr->len);

    uint32_t* intArr = (uint32_t *)outBuffer;
    num = pair.first;

    int res = tree->elem;
    while(true) {
        int help = Helper::countInt(intArr[0]);
        if (help >= num) {
        return res + Helper::getXOnePos(intArr[0], num);
        } else {
        num-=help;
        res += 32;
        intArr++;
        }
    }

    return -1;
  }

  int rank(int pos) {

    pair<int, BlockAVLNode*> pair = tree->rank(pos);

    if (pair.second == NULL) {
      return pair.first;
    }
    
//// std::cout<<"p not_NULL"<<std::endl;
//// cout<<"pos : "<<pos<<endl;

    BlockAVLNode *tree = pair.second;

    MemPointer *ptr = tree->ptr;
    mem->load(ptr, buffer);
    Compressor::decompress(buffer, outBuffer, chunkSize, ptr->len);

    uint32_t* intArr = (uint32_t *)outBuffer;
    int elements = pair.first;


    int num = pos  - tree->elem;

//// std::cout<<"p HERE"<<std::endl;

    while (num >= 32) {
      elements += Helper::countInt(intArr[0]);
      intArr++;
//// std::cout<<"p counter"<<std::endl;
      num -= 32;
    }

    if (num >= 31) {
      return elements + Helper::countInt(intArr[0]);
    } else {
      int help = (1 <<(num+1)) -1;
      return elements + Helper::countInt(intArr[0] & help);
    }
  }
};
