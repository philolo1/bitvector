// #include "../MemoryManager.cc"
#include "../MemoryManager.cc"

struct FlexibleAVLNode {

private:
  int elem; // payload

public:
  // TODO CHANGE BACK
  MemoryPointer *ptr;
  int height; // cached height of tree
  int mysize;
  int len;
  int numOnes;

  int addPos;

  // children
  FlexibleAVLNode *left, *right;

  ~ FlexibleAVLNode () {
    delete left;
    delete right;
  }

  int getMemorySize() {
    return sizeof(this) + (left ? left->getMemorySize() : 0) + (right ?  right->getMemorySize() : 0);
  }

  int getElem() { return elem; }

  // set all the values but mySize und height
  void setToNode(FlexibleAVLNode *node, int val) {
    elem = node->elem + val;
    numOnes = node->numOnes;
    ptr = node->ptr;
    len = node->len;

    // should be 0 all the time
    addPos = node->addPos;
  }

  int getStartIndex() { return elem + addPos; }

  void print(int num = 0) {
    printf("elem : %d,%d  height: %d len : %d ptr: %p\n",
           (elem + addPos + num) / 32,
           (elem + addPos + num + len * CHAR_BIT) / 32, height, len / 4, this);
    printf("ELEM : %d ADDPOS: %d NUM: %d \n", elem / 32, addPos / 32, num / 32);
    num += addPos;
    if (left) {
      printf("LEFT \n");
      left->print(num);
    }

    if (right) {
      printf("RIGHT \n");
      right->print(num);
    }
  }

  // sets addPos to zero and adds it to the children
  void propagateAddPos() {
    elem += addPos;

    if (left)
      left->addPos += addPos;
    if (right)
      right->addPos += addPos;

    addPos = 0;
  }

  // update the index of the node and its right child
  void addIndexRight(int add) {
    elem += add;
    if (right)
      right->addPos += add;
  }

  inline int getOnes() { return numOnes; }

  FlexibleAVLNode(int e, MemoryPointer *ptr, int size, int len, int numOnes)
      : elem(e), ptr(ptr), height(0), left(0), right(0), mysize(size), len(len),
        numOnes(numOnes), addPos(0)

  {}
};
