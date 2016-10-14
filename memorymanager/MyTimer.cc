// Simple timer class for outputting time
#include <ctime>
#include <iostream>
#include <sys/time.h>

class MyTimer {
  struct timeval start, end;

public:
  MyTimer() { gettimeofday(&start, NULL); }

  void printTime(const char *text) {
    gettimeofday(&end, NULL);
    clock_t help = clock();

    long secs = end.tv_sec - start.tv_sec;
    long usecs = end.tv_usec - start.tv_usec;
    double time = secs + usecs / 1000000.0;

    printf("%s %.10lf\n", text, time);
    start = end;
  }
};
