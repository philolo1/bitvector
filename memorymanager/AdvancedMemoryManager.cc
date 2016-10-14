/** author : Patrick Klitzke
 *
 *
 * This file contains the main implementation of the memory mananger
 * the tests can be found in test_AdvancedMemoryManager.cc
 *
 * To compile it one need to use the "make" command
 * furthermore the library Google test is needed (lib has to be included)
 *
 * The 
 * 
 *
 *
 */

#include <list>
#include <string>
#include <iostream>
#include <vector>
#include <assert.h>
#include <list>
#include <stack>
#include <memory.h>

using namespace std;

static int global = 0;

class Chunk;

// return values for the memory array
class MemoryArray{
public:
  char* address; // address where one can write
  int length;    // length of the address

MemoryArray(char *address, int length) : address(address), length(length) {}
};


// class for representing a logical pointer
struct AdvancedPointer {
    list<Chunk*>::iterator chunkRef; // reference to the Physical chunk
    int len; // length of the pointer (bucket position)
    int pos; // position inside the Chunk

    list<AdvancedPointer*>::iterator ref; // reference to where the pointer is located

    AdvancedPointer(int len) : len(len) {};
}; 

class Chunk {
  public:
  list<Chunk*>::iterator ref; // reference to the actual linked list

  int val; // ONLY FOR DEBUGGING
  char* array; // actual memory


  Chunk (int size) {
    array = new char[size];
    global++;
    val = global;
  }

   ~Chunk() {
     delete array;
   }
};


class AdvancedMemoryManager {

  private:
   int chunkSize; // size of buckets
   list<AdvancedPointer*> *ptrList; // lists of pointers for each bucket
   list<Chunk*> *chunkList; // list of chunks for each 

  public:

   // returns the pointer list of a bucket
   // mainly used for testing
   list<AdvancedPointer*>& getPointerList(int size) {
     return ptrList[size]; 
   }

   // retuns the chunklist 
   // mainly used for testing
   list<Chunk*>& getChunkList(int size) {
     return chunkList[size];
   }

  AdvancedMemoryManager(int chunkSize) : AdvancedMemoryManager(chunkSize, chunkSize) {}

  AdvancedMemoryManager(int chunkSize, int bucketSize) : chunkSize(chunkSize) {
      ptrList = new list<AdvancedPointer*>[bucketSize + 1];
      chunkList = new list<Chunk*>[bucketSize + 1];
  }

  // manages the memory and returns an AdvancedPointer to the
  // location of the memory
  // it might also allocate new memory
 AdvancedPointer* create(int length) {

  AdvancedPointer *ptr = new AdvancedPointer(length);

   // beginning : nothing is allocated yet
   if (ptrList[length].size() == 0 ) {

     // we don't have space that is why we create a new Pointer
     Chunk *ch = new Chunk(chunkSize);

     // put the chunk at the beginning of the linked_list
     chunkList[length].push_front(ch);
     ch->ref = chunkList[length].begin();


     // put the pointer at the begining of the list
     ptrList[length].push_front(ptr);
     ptr->pos = 0;
     ptr->ref = ptrList[length].begin();
     ptr->chunkRef = ch->ref;

   } else {

     // 1.step  move the pointer to the of the memory

     AdvancedPointer *prevPtr = (*ptrList[length].begin());
     ptr->pos = prevPtr->pos + prevPtr->len;
     ptr->chunkRef = prevPtr->chunkRef;


     // check whether we are actually pointing to the next chunk
     if (ptr->pos > chunkSize) {
       ptr->chunkRef = std::prev(ptr->chunkRef, 1);
       ptr->pos -= chunkSize;
     } 

     // case 1 : we are at the end of the chunk
     if (ptr->pos == chunkSize) {
       Chunk *ch = new Chunk(chunkSize);
       ptr->pos = 0;

       // put the chunk at the begging of the linked_list
       chunkList[length].push_front(ch);
       ch->ref = chunkList[length].begin();
       ptr->chunkRef = ch->ref;
     } else if ( ptr->pos + length > chunkSize) {
       // case 2: overflow 
       
       Chunk *ch = new Chunk(chunkSize);

       chunkList[length].push_front(ch);
       ch->ref = chunkList[length].begin();

     } else {
       // case 3: we still point to the old chunk
       // nothing to do here ptr is set correctly
       // and no new chunk is needed
     }

    // add ptr to the list
    ptrList[length].push_front(ptr);
    ptr->ref = ptrList[length].begin();

   }

   return ptr;
 }


// get the memory address to access data of size CHUNK_SIZE
// this is a vector since the data can be splitted accross two
// chunks
vector< pair<char*, int> > getMemoryArray(AdvancedPointer *ptr) {

  vector< pair<char*, int> > vec;

  // get the first pointer with its length
  char* address = &(*ptr->chunkRef)->array[ptr->pos];
  int len = min(ptr->len, chunkSize - ptr->pos);

  vec.push_back(make_pair(address, len));

  // there is a second pointer if the length is still bigger than 1
  if (ptr->len - len > 0) {
   
    // get the element in front of it 
    auto it = std::prev(ptr->chunkRef); 

    vec.push_back(
        make_pair(
          (*it)->array,
          ptr->len - len
        )
    );
  }

  return vec;
}

// fill the chunk allocated with ptr with the data from
// data
void fill(AdvancedPointer *ptr, char* data) {
  auto vec = getMemoryArray(ptr);

  int pos = 0;

  for (pair<char*,int> array: vec) {
    memcpy(array.first, &data[pos],  array.second * sizeof(char));
    pos += array.second;
  }
}

// load the chunk with ptr into data
void load(AdvancedPointer *ptr, char* data) {
  auto vec = getMemoryArray(ptr);

  int pos = 0;

  for (pair<char*,int> array: vec) {
    memcpy(&data[pos], array.first, array.second * sizeof(char));
    pos += array.second;
  }
}

// access the pos element of the ptr 
// not really used in code only for testing
char access(AdvancedPointer *ptr, int pos) {
  auto obj = getMemoryArray(ptr);

  if (pos < obj[0].second) {
    return obj[0].first[pos];
  }

  return obj[1].first[pos-obj[0].second];
}

// help functino for swapping two values used in remove
void swapPointer(AdvancedPointer *p1, AdvancedPointer* p2) {
  swap(p1->len, p2->len);
  swap(p1->pos, p2->pos);
  swap(p1->chunkRef, p2->chunkRef);

  *(p1->ref) = p2;
  *(p2->ref) = p1;

  swap(p1->ref, p2->ref);
}

// removing chunk of the pointer
// the pointer is not deleted here on purpose one should delete it later 
// on or use smartpointe
void remove(AdvancedPointer *ptr) {

  // assumption only remove what is there
  
  if (ptr != (*ptrList[ptr->len].begin())) {

    // the content to be deleted is not in the last chunk


    // 1. step copy the content of the last element
    vector<pair<char*, int> > mem1 = getMemoryArray(ptr);
    vector<pair<char*, int> > mem2 = getMemoryArray(*ptrList[ptr->len].begin());

    int index[] = {0, 0};
    int add[] = {0, 0};
    int len = ptr->len;

    while (len > 0) {
      int l1 = mem1[index[0]].second-add[0];
      int l2 = mem2[index[1]].second-add[1];
      int l = min(l1, l2);

      memcpy(
          mem1[index[0]].first + add[0],
          mem2[index[1]].first + add[1],
          l*sizeof(char)
      );

      if (l1 < l2) {
        add[0] = 0;
        add[1] += l1;
        index[0]++;
        l = l1;
      } else if (l1 == l2) {
        add[0] = 0;
        add[1] = 0;
        index[0]++;
        index[1]++;
        l = l1;
      } else {
        add[0] += l2;
        add[1] = 0;
        index[1]++;
        l = l2;
      }

      len -= l;
    }

    // 2. step: move the pointer
    swapPointer(ptr, *ptrList[ptr->len].begin());
  }
  
  // remove the old pointer on the list
  ptrList[ptr->len].pop_front();

  // do we have empty space ? 
  if (ptr-> pos == 0 || ptr->pos + ptr->len > chunkSize) {

    // delete the chunk
    delete (*chunkList[ptr->len].begin());

    // remove from chunk List
    chunkList[ptr->len].pop_front();
  }
}
};
