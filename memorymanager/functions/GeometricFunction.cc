#include <iostream>
#include "Function.cc"
#include <functional>
#include <memory>
#include <random>
#include "../defines.h"

class GeometricFunction : public Function {

  std::default_random_engine generator;
  std::geometric_distribution<uint32_t>* distribution;

public:
  GeometricFunction(double d) {
      distribution = new std::geometric_distribution<uint32_t>(d);
  }

  virtual void  initArray(uint32_t* arr, uint64_t size) {
    memset(arr, 0, size * sizeof(uint32_t));

    int index = 0;
    int pos = 0;

    while (true) {
        int add = (*distribution)(generator);
        pos += add;
        int bits = (sizeof(uint32_t) * CHAR_BIT);
        index += pos / bits;
        pos = pos % bits;


        if (index >= size) {
          break;
        }
        arr[index] |= (1<<pos);
        pos++;
    }
  }
};
