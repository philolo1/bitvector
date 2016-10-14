#include "AdvancedMemoryManager.cc"

template <class T>
class CompressedArray {

  public:
  AdvancedMemoryManager *mem;

  int chunkSize;
  vector<AdvancedPointer*> ptrList;
  int SIZE = 0;

  int lastSize;

  char *buffer;
  char *outBuffer;

  CompressedArray(int chunkSize): chunkSize(chunkSize) {
    int maxSize = T::getMaxOutBuffer(chunkSize);
    mem = new AdvancedMemoryManager(maxSize);
    buffer = new char[maxSize];
    outBuffer = new char[maxSize];
  }

  void addBlock(char* arr) {

    lastSize = T::compress(arr, buffer, chunkSize);
    SIZE += lastSize;
    AdvancedPointer* ptr = mem->create(lastSize);
    mem->fill(ptr, buffer);
    ptrList.push_back(ptr);

  }

  char* accessBlock(int pos) {
    AdvancedPointer *ptr = ptrList[pos];

    T::decompress(buffer, outBuffer, chunkSize, lastSize);
    return outBuffer;
  }

  int getMemorySize() {
    return SIZE;
  }
};
