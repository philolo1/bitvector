// this is a very simpe test that only tests whether the array is not crashiing
//
#include <gtest/gtest.h>
#include <functional>
#include "SimpleFunction.cc"
#include "GeometricFunction.cc"

using namespace std;

TEST(FunctionTest, ClassTest) {

  Function* f[] = {
    new SimpleFunction([] (uint32_t n) { return n;})
  };

  ASSERT_EQ(1, f[0]->print());
}

int main( int argc, char** argv ) {
  printf("Running main() CompressedArray \n");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();

}
