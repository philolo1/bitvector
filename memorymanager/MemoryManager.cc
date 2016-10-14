/** author : Patrick Klitzke
 *
 *
 * This file contains the main implementation of the memory mananger.
 * I include the description of the email here for now:
 *
 *
 *
 *
3. Bucket allocation We have M buckets. Each bucket consists of a list of
pointers to the memory location. We always allocate chunks of size S, and one
chunk belongs only to one bucket. Because S might not be divide the size bi of
one bucket, the memory might be split between two chunks. Thus one needs to also
keep a list of the allocated S for each bucket. The issue with this is that we
need many extra pointers (for each S an pointer and for each allocated size). We
also waste up to S ∗ (M^2 ) memory.
---------------------
I just want to clarify what I was envisioning here (it is similar but maybe a
bit simpler, and I give a few more details):

DATA STRUCTURE: We have M buckets, b_1, ... b_m and each bucket b_i is a list
that will be responsible for storing blocks of size i.  Since we don't want to
call malloc each time we allocate a block of size i (since malloc likely just
rounds the requested block size to the next power of 2 anyway) we have a fixed
superblock size S, and initially assign one superblock of size S to each bucket.
The space wasted by this is O(S*M)

- INSERTION: As blocks of size i are created and assigned to b_i, the superblock
will start to fill up.  We can fit floor(S/i) blocks into the superblock before
we need to allocate a new superblock (using malloc) for bucket b_i: for some
terminology, we can say the superblock is *at capacity* if it is assigned to
bucket b_i and contains floor(S/i) .  I suggest not worrying about the O(M)
wasted space by not filling up the superblock completely, as splitting blocks
across superblocks is too complicated.  Instead, just ensure S is large enough
that the wasted space is a lower order term.  If we allocate and fill X
superblocks, then the wasted space by not completely filling them from this is
at most O(MX), so as long as MX = o(SX) then we are okay.  If M ~ sqrt(S) this
is fine.

- DELETION: One important detail omitted so far is that each block B needs to
store a *back pointer* that points to the pointer that points to it.  So
pointers to blocks are objects that may be later updated by the memory manager.
This is the price we pay to avoid memory fragmentation, and needs to be factored
into the calculations for how many blocks fit in a superblock. So, bucket i can
really only store floor(S/(i +c)) blocks for some constant c.  If we allocate a
block B of size i, and are returned a pointer p by the memory manager, we may
need to later update p since our memory manager will shift the location of B
around during deletions.  Explicitly, when a block B' is deleted from list b_i,
then we take the head block B, and swap it into the location where B' was: the
pointer p->B needs to be corrected.  This is partially why my code is such a
mess.  The wasted space for these back pointers is small provided that we not
allocating small blocks for no good reason.  That is, if we only ever allocate
constant sized blocks when we are getting massive compression savings, then we
are okay here.

- UNDERFLOWS: If the "head" superblock becomes empty for b_i, then we put the
superblock onto a global list of free superblocks (don't deallocate it, since it
can be later used by some other bucket b_j where j is not necessarily equal to
i).  Perhaps in the future if the free list gets too big we can flush it, but
for now I wouldn't bother returning it to the system.

TOTAL SPACE: ignoring the back pointers, the total wasted space is succinct
provided that S*M << the amount of space needed for the computation.  The back
pointer space is only non-negligible when the data is highly compressible, since
the assumption is that we are breaking data into chunks of size M and M is a
large number.
 *
 */

#ifndef MEM_MANAGER
#define MEM_MANAGER
#include <list>
#include <string>
#include <iostream>
#include <vector>
#include <assert.h>
#include <list>
#include <stack>
#include <memory.h>

// this file defines the memory pool but also the MemoryChunk and MemoryPointer
#include "MemoryPool.cc"

using namespace std;

class MemoryManager {

private:
  MemoryPool *pool;
  MemoryChunk **lastChunk;
  int bagSize;

public:
  MemoryManager(int chunkSize) : MemoryManager(chunkSize, chunkSize) {}

  MemoryManager(int chunkSize, int bucketSize) {
    pool = new MemoryPool(chunkSize);
    lastChunk = new MemoryChunk *[bucketSize + 1];
    bagSize = bucketSize + 1;

    // this is STUPID, WHY DO I HAVE TO DO THIS ???????
    for (int n = 0; n < bucketSize + 1; n++) {
      lastChunk[n] = NULL;
    }
  }

  ~MemoryManager() {
    //for (int i = 0; i < bagSize +1;i++) {
    //  delete lastChunk[i];
    //}
    delete [] lastChunk;
    delete pool;
  }

  MemoryPointer *create(int len) {
    if (lastChunk[len] == NULL || lastChunk[len]->isFull()) {
      lastChunk[len] = pool->reserve(len);
    }

    auto Add = lastChunk[len]->addElement();

    return Add;
  }

  void create(int len, MemoryPointer *p) {
    if (lastChunk[len] == NULL || lastChunk[len]->isFull()) {
      lastChunk[len] = pool->reserve(len);
    }

    lastChunk[len]->addElement(p);
  }


  // access testing in a unique way
  // just for the unit testings
  vector<pair<char *, int> > getMemoryArray(MemoryPointer *ptr) {
    vector<pair<char *, int> > vec;
    vec.push_back(make_pair(&ptr->chunk->memory[ptr->pos], ptr->chunk->len));
    return vec;
  }

  // fill the chunk allocated with ptr with the data from
  void fill(MemoryPointer *ptr, char *data) {
    memcpy(&ptr->chunk->memory[ptr->pos], data, ptr->chunk->len);
  }

  // load the chunk with ptr into data
  void load(MemoryPointer *ptr, char *data) {
    memcpy(data, &ptr->chunk->memory[ptr->pos], ptr->chunk->len);
  }

  // access the pos element of the ptr
  // not really used in code only for testing
  char access(MemoryPointer *ptr, int pos) {
    int index = ptr->pos + pos;
    return ptr->chunk->memory[index];
  }

  // help functino for swapping two values used in remove
  void swapPointer(MemoryPointer *p1, MemoryChunk *chunk2) {

    MemoryChunk *chunk1 = p1->chunk;

    int ptrPos1 = p1->pos / p1->chunk->len;

    MemoryPointer *p2 = chunk2->ptrVec[chunk2->ptrVec.size() - 1];

    swap(p1->chunk, p2->chunk);
    swap(p1->pos, p2->pos);

    chunk1->ptrVec[ptrPos1] = p2;

    chunk2->ptrVec[chunk2->ptrVec.size() - 1] = p1;
  }

  inline char *getMemoryLocation(MemoryPointer *ptr) {
    return (ptr->chunk->memory + ptr->pos);
  }

  // removing chunk of the pointer
  // the pointer is not deleted here on purpose one should delete it later
  // on or use smartpointe
  void remove(MemoryPointer *ptr) {

    int len = ptr->chunk->len;

    if (lastChunk[len] == NULL) {
      lastChunk[len] = ptr->chunk;
    }

    if (!lastChunk[len]->isLastPointer(ptr)) {

      // move the memory of the pointers
      memcpy(getMemoryLocation(ptr),
             getMemoryLocation(lastChunk[len]->getLastPointer()),
             ptr->chunk->len);

      // swap the actual pointers
      swapPointer(ptr, lastChunk[len]);
    }

    // now delete
    lastChunk[len]->removeElement();

    if (lastChunk[len]->isEmpty()) {
      pool->save(lastChunk[len]);
      lastChunk[len] = NULL;
    }
  }
};

#endif
