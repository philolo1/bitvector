/** author : Patrick Klitzke
 *
 *
 * This file contains the main implementation of a very simple memory
 * that uses malloc and free
 * To compile it one need to use the "make" command
 * furthermore the library Google test is needed (lib has to be included)
 *
 */

#ifndef SIMPLE_MEM_MANAGER 
#define SIMPLE_MEM_MANAGER 

#include <list>
#include <string>
#include <iostream>
#include <vector>
#include <assert.h>
#include <list>
#include <stack>
#include <memory.h>
#include "Helper.cc"

using namespace std;

// class for representing a logical pointer
struct SimplePointer {
  void * memory;
  int len;
}; 


class SimpleMemoryManager{
  int addNum;

  public:
    // constructor does nothing
    SimpleMemoryManager(int length): SimpleMemoryManager(length, length) {

    }

    // only for the interface
    SimpleMemoryManager(int a, int b) {
    }

    SimplePointer* create(int length) {
      addNum+= length;
      SimplePointer *ptr = new SimplePointer();
      ptr->memory = malloc(length);
      ptr->len = length;
      return ptr;
    }

    inline char *getMemoryLocation(SimplePointer *ptr) {
      return (char *)ptr->memory;
    }



    // get the memory address to access data of size CHUNK_SIZE
    // this is a vector since the data can be splitted accross two
    // chunks
    vector< pair<char*, int> > getMemoryArray(SimplePointer *ptr) {
      vector< pair<char*, int> >  v;
      v.push_back(make_pair((char *)ptr->memory, ptr->len));
      return v;
    }

    void fill(SimplePointer *ptr, void* data) {
      memcpy(ptr->memory, data,  ptr->len);
    }

    // load the chunk with ptr into data
    void load(SimplePointer *ptr, void* data) {
      memcpy(data, ptr->memory, ptr->len);
    }

    // access the pos element of the ptr 
    // not really used in code only for testing
    char access(SimplePointer *ptr, int pos) {
      char* ch = (char *)ptr->memory;
      return ch[pos];
    }

    void remove(SimplePointer *ptr) {
      addNum-= ptr->len;
      free(ptr->memory);
    }

    void print() {
      std::cout << "MEMORY IN MEMORY MANAGER: "<<Helper::FormatWithCommas(addNum) <<endl;
    }
};


#endif
