#include "AVLTree.cpp"
#include <vector>
#include <algorithm>
#include <gtest/gtest.h>

using namespace std;

// get out what you put in
TEST(AdvancedMemoryManagerTest, ACCESS_REMOVE_TEST) {
  // repeat 10 times because we remove randomly
  for (int n=0; n < 10; n++) {
    vector<int> v;
    AVLTree tree;
    const int SIZE = 100000;

    for (int n=0; n< SIZE; n++) {
      v.push_back(n);
      tree.insert(n);
      ASSERT_EQ(n+1,  tree.size());
    }

    // test whether we can find all the elements
    for (int n=0; n < SIZE; n++) {
      ASSERT_EQ(1, tree.find(n));
    }


    // now delete in random order
    random_shuffle(v.begin(), v.end());

    int counter = SIZE;
    for (int el : v) {
      tree.deleteElem(el);
      counter--;
      ASSERT_EQ(counter,  tree.size());
    }

    // test that nothing is there anymore
    for (int n=0; n < SIZE; n++) {
      ASSERT_EQ(0, tree.find(n));
    }
  }
}


int main( int argc, char** argv ) {
  printf("Running main() from sample1\n");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
