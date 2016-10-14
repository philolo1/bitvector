#ifndef MEM_POOL
#define MEM_POOL

#include <list>
#include <string>
#include <iostream>
#include <vector>
#include <assert.h>
#include <list>
#include <stack>

#include <assert.h>
#include <stdlib.h>
#include <sys/mman.h>

using namespace std;

class MemoryChunk;

struct MemoryPointer {
  MemoryChunk *chunk;
  int pos;
  int len;


  MemoryPointer(MemoryChunk *ptr, int pos, int len)
      : chunk(ptr), pos(pos), len(len) {}

  MemoryPointer() {}

  void init(MemoryChunk *ptr, int pos, int len) {
    this->chunk = ptr;
    this->pos = pos;
    this->len = len;
  }
};

class MemoryChunk {
public:
  int size;
  int space;
  int len;
  char *memory;

  vector<MemoryPointer *> ptrVec;

  MemoryChunk(int size, int len) : size(size), len(len) {
    memory = new char[size];
    // memory = new char[size];
    space = 0;
    // maybe do ptr->reserve later on
  }

  ~ MemoryChunk() {
    delete [] memory;
  }

  void setLen(int val) { len = val; }

  // only called when bigger 0
  MemoryPointer *getLastPointer() { return ptrVec[ptrVec.size() - 1]; }

  bool isFull() { return len + space > size; }

  // indicates whether ptr is the last pointer in ptrVec
  bool isLastPointer(MemoryPointer *ptr) {
    return (ptr->chunk == this) && ptr->pos == (space - len);
  }

  MemoryPointer *addElement() {
    int pos = space;
    space += len;
    MemoryPointer *ptr = new MemoryPointer(this, pos, len);
    ptrVec.push_back(ptr);
    return ptr;
  }

  void addElement(MemoryPointer *p) {
    int pos = space;
    space += len;
    p->init(this, pos, len);
    ptrVec.push_back(p);
  }

  void removeElement() {
    space -= len;
    ptrVec.pop_back();
  }

  bool isEmpty() { return space == 0; }
};

// class for reserving chunks of memory of size S
class MemoryPool {
  int chunkSize;

  stack<MemoryChunk *> savedMemory;

public:
  MemoryPool(int chunkSize) : chunkSize(chunkSize) {}

  ~MemoryPool() {
    while (savedMemory.size() > 0) {
      MemoryChunk * m = savedMemory.top();
      delete m;
      savedMemory.pop();
    }
  }

  MemoryChunk *reserve(int len) {
    if (savedMemory.empty()) {
      return new MemoryChunk(chunkSize, len);
    } else {
      MemoryChunk *chunk = savedMemory.top();
      chunk->setLen(len);
      savedMemory.pop();
      return chunk;
    }
  }

  void print() {
    std::cout << "FREE CHUNKS : " << savedMemory.size() << std::endl;
  }

  // precondition chunk empty
  void save(MemoryChunk *chunk) { savedMemory.push(chunk); }
};

#endif
