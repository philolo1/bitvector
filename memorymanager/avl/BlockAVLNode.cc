#include "../MemoryManager.cc"

struct BlockAVLNode {
    int elem;         // payload
    MemoryPointer *ptr;
    int height;       // cached height of tree
    int mysize;
    int len;

    int numOnes;

    void addSize(int num) {
      mysize += num;
      numOnes += num;
    }

    inline int getOnes() {
      return numOnes;
     }

    BlockAVLNode *left, *right;
    BlockAVLNode(int e, MemoryPointer *ptr, int size, int len, int numOnes) : elem(e), ptr(ptr), height(0), left(0), right(0), mysize(size), len(len), numOnes(numOnes) {

    }
    ~BlockAVLNode() {delete left; delete right;}
};
