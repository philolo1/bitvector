// class for handling the memoryManager


#include <map>
#include "FlexibleCache.cc"


template <class Compressor, class MemManager, class MemPointer>
class MemoryProvider {
  // maps a MemoryManger

  public:
    static MemManager *getMemoryManager(int chunkSize, int bucketSize) {
      pair<int, int> p = make_pair(chunkSize, bucketSize);

      if (managerMap.find(p) == managerMap.end()) {
        managerMap[p] = new MemManager(chunkSize, bucketSize);
      }

      return managerMap[p];
    }

    static FlexibleCache<CACHE_SIZE, Compressor, MemManager, MemPointer> *getCache(int a, MemManager* b) {
      pair<int, MemManager*> p = make_pair(a, b);

      if (cacheMap.find(p) == cacheMap.end()) {
        cacheMap[p] = new FlexibleCache<CACHE_SIZE, Compressor, MemManager, MemPointer>(a, b);
      }

      return cacheMap[p];
    }

    static void printMemory() {
      for (auto& kv : managerMap) {
        kv.second->print();
      }
    }

  private:
    static map<pair<int, int>, MemManager *> managerMap;
    static map<pair<int, MemManager *>, FlexibleCache<CACHE_SIZE, Compressor, MemManager, MemPointer> *> cacheMap;
};

template <class Compressor, class MemManager, class MemPointer>
map<pair<int, int>, MemManager *> MemoryProvider<Compressor, MemManager, MemPointer>::managerMap;

template <class Compressor, class MemManager, class MemPointer>
map<pair<int, MemManager *>, FlexibleCache<CACHE_SIZE, Compressor, MemManager, MemPointer> *> MemoryProvider<Compressor, MemManager, MemPointer>::cacheMap;

