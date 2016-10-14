#include "AdvancedAVLTree.cpp"
#include <vector>
#include <algorithm>
#include <stack>
#include "../Helper.cc"
#include <gtest/gtest.h>

using namespace std;


template <class Tree>
void testSelectAndRank(Tree& tree, vector<int>& v, int SIZE) {
  for (int n=0; n< v.size(); n++) {
    ASSERT_EQ(v[n], tree.select(n)) <<" n : "<<n;
  }

  for (int k=0; k < 100; k++) {
    int index = v.size() + k;
    ASSERT_EQ(-1, tree.select(index)) <<" n : " <<index;
  }

  stack<int> st;
  for (int n= v.size() -1; n >= 0; n--) {
    st.push(v[n]);
  }

  int counter = 0;
  for (int n=0; n < SIZE; n++) {
      if (!st.empty()) {
      int elem = st.top();
      if (elem <= n) {
        st.pop();
        counter++;
      }
    }

    ASSERT_EQ(counter, tree.rank(n));
  }
}


template<class Tree>
void testAllBlocksRandom() {
  srand(42);
  for (int n=0; n < 100; n++) {
    Tree tree;

    vector<int> v;
    vector<int> selectVal;

    for (int k=0; k < 1000; k++) {
      v.push_back(rand());
      tree.insert(k*32, v[k]);
    }

    for (int k=0; k<20; k++) {
      int a = rand() % 2;
      int b = rand() % 32;
      int c = rand() % 1000;
      
      if (a) {
        v[c] = Helper::setZero(v[c], b);
        tree.modify(c*32 + b, 0);
      } else {
        v[c] = Helper::setOne(v[c], b);
        tree.modify(c*32 + b, 1);
      }
    }

    for (int k=0; k < 1000; k++) {
      for (int n=0; n<32; n++) {
        ASSERT_EQ((v[k] >> n)&1, tree.find(k*32 + n));
        if ((v[k] >> n)&1) { 
          selectVal.push_back(k * 32 + n); 
        }
      }
    }
    testSelectAndRank<Tree>(tree, selectVal, 1000*32);
  }
}

template <class Tree>
void testSimple() {
   for (int n=0; n < 32; n++) {
    Tree tree;
    tree.insert(0, 1<<n);

    for (int m=0; m<32; m++) {
      ASSERT_EQ(m==n, tree.find(m));
    }

    ASSERT_EQ(n,  tree.select(0));

    for (int n=1; n<100; n++) {
      ASSERT_EQ(-1,  tree.select(n));
    }

    for (int m=0; m<n; m++) {
      ASSERT_EQ(0, tree.rank(m)) <<" m : "<<m<<" n : "<<n<<endl;
    }

    for (int m=n; m<32; m++) {
      ASSERT_EQ(1, tree.rank(m)) <<" m : "<<m<<" n : "<<n<<endl;
    }
   }

}

template<class Tree>
void testSimple2() {
  srand(42);
  for (int n=0; n < 100; n++) {
    Tree tree;
    int num = rand();
    tree.insert(0, num);

    vector<int> selectVal;

    for (int n=0; n<32; n++) {
      if ((num >> n)&1) {
        selectVal.push_back(n);
        ASSERT_EQ(1, tree.find(n));
      } else {
        ASSERT_EQ(0, tree.find(n));
      }
    }
    testSelectAndRank<Tree>(tree, selectVal, 32);
  }
}


template<class Tree> 
void testTwoBlocks() {

  srand(42);
  for (int n=0; n < 100; n++) {
    Tree tree;
    int num = rand();
    int num2 = rand();
    tree.insert(0, num);
    tree.insert(32, num2);

    vector<int> selectVal;

    for (int n=0; n<32; n++) {
      int res   = (num >> n)&1;
      if (res) {
        selectVal.push_back(n);
      }

      ASSERT_EQ(res, tree.find(n));
    }
    for (int n=0; n<32; n++) {
      ASSERT_EQ((num2 >> n)&1, tree.find(32 + n));

      if ((num2 >> n)&1) {
        selectVal.push_back(32 + n);
      }
    }

    testSelectAndRank<Tree>(tree, selectVal, 64);
  }
}

template<class Tree>
void testAllBlocks() {
   srand(42);
   for (int n=0; n < 100; n++) {
     Tree tree;

     vector<int> v;
     vector<int> selectVal;

     for (int k=0; k < 1000; k++) {
       v.push_back(rand());
       tree.insert(k*32, v[k]);
     }

     for (int k=0; k < 1000; k++) {
       for (int n=0; n<32; n++) {
         ASSERT_EQ((v[k] >> n)&1, tree.find(k*32 + n));
         if ((v[k] >> n)&1) { 
           selectVal.push_back(k * 32 + n); 
         }
       }
     }
     testSelectAndRank<Tree>(tree, selectVal, 1000*32);
   }
}


template <class Tree>
void testTwoBlocksModify() {
  srand(42);
  for (int n=0; n < 1000; n++) {
    AdvancedAVLTree tree;
    int  num[2];
    num[0] = rand();
    num[1] = rand();

    tree.insert(0, num[0]);
    tree.insert(32, num[1]);

    // modify our world
    for (int k=0; k<20; k++) {
      int a = rand() % 2;
      int b = rand() % 32;
      int c = rand() % 2;
      
      if (a) {
        // cout<<"setZero"<<endl;
        num[c] = Helper::setZero(num[c], b);
        tree.modify(c*32 + b, 0);
      } else {
        // cout<<"setOne "<<"c : "<<c<<" b : "<<b<<endl;
        num[c] = Helper::setOne(num[c], b);
        tree.modify(c*32 + b, 1);
      }

    }

    vector<int> selectVal;

    for (int n=0; n<32; n++) {
      int res   = (num[0] >> n)&1;
      if (res) {
        selectVal.push_back(n);
      }

      ASSERT_EQ(res, tree.find(n));
    }
    for (int n=0; n<32; n++) {
      ASSERT_EQ((num[1] >> n)&1, tree.find(32 + n));

      if ((num[1] >> n)&1) {
        selectVal.push_back(32 + n);
      }
    }

    testSelectAndRank<Tree>(tree, selectVal, 64);
  }

}

 template<class Tree>
void testSimleModify() {
   for (int n=0; n < 31; n++) {
    Tree tree;
    tree.insert(0, 0);

    tree.modify(n, 1);

    for (int m=0; m<32; m++) {
      ASSERT_EQ(m==n, tree.find(m));
    }

    ASSERT_EQ(n, tree.select(0));

    for (int n=1; n<100; n++) {
      ASSERT_EQ(-1,  tree.select(n));
    }

    for (int m=0; m<n; m++) {
      ASSERT_EQ(0, tree.rank(m)) <<" m : "<<m<<" n : "<<n<<endl;
    }

    for (int m=n; m<32; m++) {
      ASSERT_EQ(1, tree.rank(m)) <<" m : "<<m<<" n : "<<n<<endl;
    }
   }

}


// get out what you put in 
TEST(AdvancedAVLTree, SimpleTest) {
  testSimple<AdvancedAVLTree>();
}



// get out what you put in 
TEST(AdvancedAVLTree, SimpleTest2) {
  testSimple2<AdvancedAVLTree>();
}
//
// // // get out what you put in 
TEST(AdvancedAVLTree, TwoBlocks) {
  testTwoBlocks<AdvancedAVLTree>();
}

// // get out what you put in 
 TEST(AdvancedAVLTree, AllBlocks) {
   testAllBlocks<AdvancedAVLTree>();
 }

TEST(AdvancedAVLTree, SimpleModify) {
  testSimleModify<AdvancedAVLTree>();
}

TEST(AdvancedAVLTree, TwoBlocksModify) {
  testTwoBlocksModify<AdvancedAVLTree>();
}


TEST(AdvancedAVLTree, AllBlocksRandom) {
  testAllBlocksRandom<AdvancedAVLTree>();
}

int main( int argc, char** argv ) {
  printf("Running main() from sample1\n");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
