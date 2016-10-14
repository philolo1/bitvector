#ifndef FUNCTION_H

#include <stdint.h>
#define FUNCTION_H
struct Function {
  virtual void initArray(uint32_t* arr, uint64_t size) { }
  virtual int print() { return 0;}
};

#endif
