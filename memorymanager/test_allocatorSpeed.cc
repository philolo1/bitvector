#include "MemoryManager.cc"
#include "SimpleMemoryManager.cc"
#include <gtest/gtest.h>


template<class Manager>
void test() {
  Manager *mem = new Manager(1024*1024,1024);

  for (int n=0; n < 10000000; n++) {
    mem->create(1000);
  }
}

template<class Manager>
void test2() {
  Manager *mem = new Manager(1024*1024,1024);

  for (int n=0; n < 10000000; n++) {
    mem->create(n%1024);
  }
}


// get out what you put in 
TEST(AllocTest, MyMemoryManager) {
  test<MemoryManager>();
}

TEST(AllocTest, SimpleMemoryManager) {
  test<SimpleMemoryManager>();
}


// get out what you put in 
TEST(AllocTest, MyMemoryManager2) {
  test2<MemoryManager>();
}

TEST(AllocTest, SimpleMemoryManager2) {
  test2<SimpleMemoryManager>();
}


int main( int argc, char** argv ) {
  printf("Running main() from sample1\n");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
  
}
