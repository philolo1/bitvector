// small helper class for bit manipulations
#include <climits>
#include <stdint.h>
#include "defines.h"

#include <iostream>
#include <sstream>
#include <fstream>

#include <unistd.h> // sysconf, _SC_PAGE_SIZE

#ifndef HELPER_H
#define HELPER_H

using namespace std;

// count number of in a vector
struct Helper {
  static char countChar(char i)
  {
    int num = 0;
    // do this faster
    for (int n=0; n < CHAR_BIT; n++) {
      num+= (i>>n) &1;
    }

    return num;
  }

  template<class T>
  static  std::string FormatWithCommas(T value)
    {
      std::stringstream ss;
      ss.imbue(std::locale(""));
      ss << std::fixed << value;
      return ss.str();
    }


  static int countInt(uint32_t num) {
    return __builtin_popcount(num);
  }

  static int innerRank(uint32_t help, int num) {
    int pos = 0;

    for (int n=0; n < INT_BIT; n++) {
      if ((help >>n) &1) {
        num--;
        if (num == 0) {
          return n;
        }
      }
    }
    // should never happen
    return -1;
  }


  static inline int setOne(int data, int pos) {
      return data | (1<<pos);
    }

   static inline int setZero(int data, int pos) {
      return data & ~(1<<pos);
    }






  // get the position of the num-th one in data
  // 0000001 , 1 => 0
  // 0000101, 2 => 3
  //
   static int getXOnePos(int data, int num) {
     int pos = 0;
     while (num > 0) {
       if (data &1 ) {
         num--;
       }
       data >>= 1;
       pos++;
     }

     return pos -1;
   }

   static void measureSpace() {
     long long vm, rss;
     process_mem_usage(vm, rss);
     cout << "VM: " << vm << "; RSS: " << rss << endl;
   }


   //  code inspired by  http://stackoverflow.com/questions/669438/how-to-get-memory-usage-at-run-time-in-c
   static void process_mem_usage(long long & vm_usage, long long & resident_set)
   {
     using std::ios_base;
     using std::ifstream;
     using std::string;

     vm_usage     = 0;
     resident_set = 0;

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
     vm_usage     = vsize / 1024;
     resident_set = rss * page_size_kb;
}



};

#endif
