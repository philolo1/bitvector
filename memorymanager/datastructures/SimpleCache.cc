#include <memory.h>
#include <vector>
#include <iostream>

using namespace std;

template <int N, class Compressor, class MemManager, class MemPointer>
class SimpleCache {
public:
  int ptrArr[N];
  char *chArr[N];
  int chunkSize;

  char *buffer;

  vector<MemPointer *> &ptrList;
  MemManager *mem;

  SimpleCache(int maxSize, int chunkSize, vector<MemPointer *> &vec,
              MemManager *mem)
      : chunkSize(chunkSize), ptrList(vec), mem(mem) {
    buffer = new char[maxSize];
    for (int n = 0; n < N; n++) {
      ptrArr[n] = -1;
    }

    char *helpArr = new char[maxSize * N];
    for (int n = 0; n < N; n++) {
      chArr[n] = helpArr + n * maxSize;
    }
  }

  void push(char *buffer) {
    makeFree();
    ptrArr[0] = ptrList.size();
    ptrList.push_back(nullptr);
    memcpy(chArr[0], buffer, chunkSize);
  }

  char *get(int ptrIndex) {
    // check whether element is in the caceh
    for (int n = 0; n < N; n++) {
      if (ptrArr[n] == ptrIndex) {
        return chArr[n];
      }
    }

    // its not in the cache
    makeFree();
    return load(ptrIndex);
  }

private:
  // internal functions
  char *load(int ptrPos) {
    mem->load(ptrList[ptrPos], buffer);
    Compressor::decompress(buffer, chArr[0], chunkSize, ptrList[ptrPos]->len);
    ptrArr[0] = ptrPos;
    return chArr[0];
  }

  // store the index in the real pointer array
  void store(int ourIndex) {
    int size = Compressor::compress(chArr[ourIndex], buffer, chunkSize);

    if (ptrList[ptrArr[ourIndex]] != nullptr) {
      mem->remove(ptrList[ptrArr[ourIndex]]);
    }

    MemPointer *ptr = mem->create(size);
    mem->fill(ptr, buffer);

    ptrList[ptrArr[ourIndex]] = ptr;
  }

  // make the cache such that one element is free and
  // return that index
  void makeFree() {

    int index = N - 1;

    for (int n = 0; n < N; n++) {
      if (ptrArr[n] == -1) {
        index = n;
        break;
      }
    }

    if (ptrArr[index] != -1) {
      store(index);
    }

    // swap such that the element is at the start position
    for (int n = index - 1; n >= 0; n--) {
      swap(chArr[n], chArr[n + 1]);
      swap(ptrArr[n], ptrArr[n + 1]);
    }
  }
};
