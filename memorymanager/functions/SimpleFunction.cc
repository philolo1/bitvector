#ifndef SYMBOL
#define SYMBOL

#include <iostream>
#include "Function.cc"
#include <functional>

class SimpleFunction : public Function {

  std::function<uint32_t(uint32_t)> myFun;

public:
  SimpleFunction(std::function<uint32_t(uint32_t)> f) { myFun = f; }

  void initArray(uint32_t *arr, uint64_t size) {
    for (int n = 0; n < size; n++) {
      arr[n] = myFun(n);
    }
  }

  int print() { return 1; }
};

#endif
