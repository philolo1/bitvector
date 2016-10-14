/*
 * This is a class for caching the decompressed arrays
 * it uses the Least recently used technique
 */

#ifndef CACHE_PROVIDER
#define CACHE_PROVIDER
#include <stdlib.h>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <list>

using namespace std;

class CacheProvider {

private:
  char *arr;

  typedef std::list<std::pair<void *, char *>> List;

  List list;

  std::unordered_map<void *, List::iterator> myMap;

  const int elementSize;

public:
  CacheProvider(int elimNum, int elemSize) : elementSize(elemSize) {
    arr = new char[elimNum * elementSize];

    for (int n = 0; n < elimNum; n++) {
      list.push_back(make_pair(nullptr, &arr[n * elementSize]));
    }
  }

  virtual ~CacheProvider() { delete arr; };

  void *access(void *ptr) {
    if (!ptr) {
      return nullptr;
    }

    // put the accessed element on the front of the list
    if (myMap.find(ptr) != myMap.end()) {
      auto a = myMap[ptr]->first;
      auto b = myMap[ptr]->second;

      list.erase(myMap[ptr]);

      list.push_front(make_pair(a, b));
      myMap[ptr] = list.begin();
      auto d = myMap[ptr]->second;

      return d;
    }

    return nullptr;
  }

  // we assume ptr was not put into the cache yet
  void *put(void *ptr) {

    // get the last element of the list
    auto listElem = list.back();

    // remove the key
    if (listElem.first != nullptr) {
      myMap.erase(listElem.first);
    }

    list.pop_back();

    list.push_front(make_pair(ptr, listElem.second));

    myMap[ptr] = list.begin();

    // copy the array
    return myMap[ptr]->second;
  }
};
#endif
