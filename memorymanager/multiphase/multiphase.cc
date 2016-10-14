#include "../SimpleMemoryManager.cc"
#include "../MemoryManager.cc"
#include "../AdvancedMemoryManager.cc"
#include "../functions/SimpleFunction.cc"
#include "../functions/GeometricFunction.cc"
#include "../MyTimer.cc"

#include <string>
#include <fstream>

#include <set>
#include <stack>
#include <algorithm>
// #include <malloc.h>
using namespace std;

// process_mem_usage(double &, double &) - takes two doubles by reference,
// attempts to read the system-dependent data for a process' virtual memory
// size and resident set size, and return the results in KB.
//
// On failure, returns 0.0, 0.0

// code inspired by http://stackoverflow.com/questions/669438/how-to-get-memory-usage-at-run-time-in-c
void process_mem_usage(double& vm_usage, double& resident_set)
{
  vm_usage     = 0.0;
  resident_set = 0.0;

  // 'file' stat seems to give the most reliable results
  //
  ifstream stat_stream("/proc/self/stat",ios_base::in);

  // dummy vars for leading entries in stat that we don't care about
  //
  string pid, comm, state, ppid, pgrp, session, tty_nr;
  string tpgid, flags, minflt, cminflt, majflt, cmajflt;
  string utime, stime, cutime, cstime, priority, nice;
  string O, itrealvalue, starttime;

  // the two fields we want
  //
  unsigned long vsize;
  long rss;

  stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
    >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
    >> utime >> stime >> cutime >> cstime >> priority >> nice
    >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

  stat_stream.close();

  long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
  vm_usage     = vsize / 1024.0;
  resident_set = rss * page_size_kb;
}

/**
 *
 */
void testNormal(int allocPow, int iterationPow, int chunkPow, int deleteCond) {

  MyTimer timer;

  int allocSize = 1 << allocPow;
  int chunkSize = 1 << chunkPow;
  int iterations = 1 << iterationPow;

  MemoryManager *mem = new MemoryManager(chunkSize, allocSize);
  MemoryPointer *memPtrs = new MemoryPointer[iterations];

  cout << "NORMAL\t" << allocPow << "\t" << iterationPow << "\t" << chunkPow << "\t";

  double vm, rss;
  process_mem_usage(vm, rss);
  cout << vm << "\t" << rss << "\t";

  for (int n = 0; n < iterations; n++) {
    // create a value between 1 .. allocSize
    int size = allocSize / 2;

    // Alloc
    MemoryPointer *p = &memPtrs[n];
    mem->create(size, p);
    char *memloc = mem->getMemoryLocation(p);
    for (int i = 0; i < size; i++) {
      memloc[i] = (char)i;
    }
  }

  process_mem_usage(vm, rss);
  cout << vm << "\t" << rss << "\t";

  for (int round = 1;
       round <= allocSize / 2;
       round++) {

    for (int n = 0; n < iterations / (allocSize /2); n++) {
      int idx = rand ()  % iterations;
      int size = allocSize / 2 + round;

      mem->remove(&memPtrs[idx]);
      MemoryPointer *p = &memPtrs[idx];
      mem->create(size, p);
      char *memloc = mem->getMemoryLocation(p);
      for (int i = 0; i < size; i++) {
        memloc[i] = (char)i;
      }

    }
  }


  // Alloc

  process_mem_usage(vm, rss);
  cout << vm << "\t" << rss << "\t";
  //malloc_stats();

  for (int i = 0; i < iterations; i++) {
    MemoryPointer *p = &memPtrs[i];
    mem->remove(p);
  }
  delete [] memPtrs;
  delete mem;

  timer.printTime("");

}


/**
 *
 */
void testSimple(int allocPow, int iterationPow, int chunkPow, int deleteCond) {

  MyTimer timer;

  int allocSize = 1 << allocPow;
  int chunkSize = 1 << chunkPow;
  int iterations = 1 << iterationPow;

  void **memPtrs = new void*[iterations];

  cout << "SIMPLE\t" << allocPow << "\t" << iterationPow << "\t" << chunkPow << "\t";

  double vm, rss;
  process_mem_usage(vm, rss);
  cout << vm << "\t" << rss << "\t";

  for (int n = 0; n < iterations; n++) {
    // create a value between 1 .. allocSize
    int size = allocSize / 2;

    // Alloc
    memPtrs[n] = malloc(size);
    for (int i = 0; i < size; i++) {
      ((char*)memPtrs[n])[i] = (char)i;
    }
  }

  process_mem_usage(vm, rss);
  cout << vm << "\t" << rss << "\t";

  for (int round = 1;
       round <= allocSize / 2;
       round++) {

    for (int n = 0; n < iterations / (allocSize / 2); n++) {
      int idx = rand ()  % iterations;
      int size = allocSize /2 + round;

      free(memPtrs[idx]);
      memPtrs[idx] = malloc(size);
      for (int i = 0; i < size; i++) {
        ((char*)memPtrs[idx])[i] = (char)i;
      }
    }
  }

  process_mem_usage(vm, rss);
  cout << vm << "\t" << rss << "\t";

  //malloc_stats();

  for (int i = 0; i < iterations; i++) {
    free(memPtrs[i]);
  }

  delete [] memPtrs;

  timer.printTime("");

}


int main(int argc, char *argv[]) {

  srand(42);

  // type : simple, normal, complex
  string type = string(argv[1]);

  int allocPow = atoi(argv[2]);
  int chunkPow = atoi(argv[3]);
  int allIterations = atoi(argv[4]);
  int deleteCond = atoi(argv[5]) - 1;
  if (type.compare(string("simple")) == 0) {
    testSimple(allocPow, allIterations, chunkPow, deleteCond);
  } else if (type.compare(string("normal")) == 0) {
    testNormal(allocPow, allIterations, chunkPow, deleteCond);
  } else {
    cout << "WRONG FIRST argument" << endl;
  }
  return 0;
}
