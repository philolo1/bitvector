#include "avl/AVLTree.cpp"
#include <vector>
#include <algorithm>
#include <gtest/gtest.h>
#include "Helper.cc"

using namespace std;

// get out what you put in 
TEST(HelperTest, getXPosOneTest) {


  // one simple test
  for (int n=0; n<31; n++) {
    ASSERT_EQ(n, Helper::getXOnePos(1 << n, 1));
  }

  ASSERT_EQ(1, Helper::getXOnePos(3 , 2));
  ASSERT_EQ(2, Helper::getXOnePos(5 , 2));

}


int main( int argc, char** argv ) {
  printf("Running main() from sample1\n");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
