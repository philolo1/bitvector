#include <memory.h>
#include <iostream>
#include <vector>

using namespace std;

template <class Compressor, class MemManager, class MemPointer>
class SimpleCache2 {
public:
  int *ptrArr;
  char **chArr;
  int N;
  char *buffer;
  char *outBuffer;

  int chunkSize;

  vector<MemPointer *> &ptrList;
  MemManager *mem;

  SimpleCache2(int num, int maxSize, int chunkSize, vector<MemPointer *> &vec,
               MemManager *mem)
      : N(num), chunkSize(chunkSize), ptrList(vec), mem(mem) {
    buffer = new char[maxSize];
    outBuffer = new char[maxSize];
    ptrArr = new int[num];
    for (int n = 0; n < num; n++) {
      ptrArr[n] = -1;
    }

    chArr = new char *[num];

    char *helpArr = new char[maxSize * num];
    for (int n = 0; n < num; n++) {
      chArr[n] = helpArr + n * maxSize;
    }
  }

  inline char *decompress(MemPointer *ptr, int bufferLen) {
    mem->load(ptr, buffer);
    Compressor::decompress(buffer, outBuffer, bufferLen, ptr->len);
    return outBuffer;
  }

  void push(char *block) {
    int size = Compressor::compress(block, buffer, chunkSize);
    MemPointer *ptr = mem->create(size);
    mem->fill(ptr, buffer);
    ptrList.push_back(ptr);
  }

  char *get(int chunkIndex) {
    MemPointer *ptr = ptrList[chunkIndex];
    outBuffer = decompress(ptr, chunkSize);
    mem->load(ptr, buffer);
    Compressor::decompress(buffer, outBuffer, chunkSize, ptr->len);
    return outBuffer;
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
    MemPointer *ptr = mem->create(size);
    mem->fill(ptr, buffer);
    ptrList[ourIndex] = ptr;
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

    // cout << "A" << endl;
    if (ptrArr[index] != -1) {
      store(index);
    }

    // cout << "B" << endl;
    // swap such that the element is at the start position
    for (int n = index - 1; N >= 0; n--) {
      swap(chArr[n], chArr[n + 1]);
      swap(ptrArr[n], ptrArr[n + 1]);
    }

    cout << "C" << endl;
  }
};
