#include "AdvancedMemoryManager.cc"
#include <gtest/gtest.h>

inline char charConv(char c) {
  return c;
}

// get out what you put in 
TEST(AdvancedMemoryManagerTest, AccessTest) {
  AdvancedMemoryManager *mem = new AdvancedMemoryManager(1000);


  vector<AdvancedPointer *> vecPtr;

  char counter = 0;

  for(int k=0; k < 30; k++) {
    AdvancedPointer *ptr = mem->create(100);
    vecPtr.push_back(ptr);

    auto p = mem->getMemoryArray(ptr);

    for (auto info : p) {
      for (int n=0; n < info.second; n++) {
        counter++;
        info.first[n] = counter;
      }
    }
  }

  for (int k=0; k <30; k++) {
    for (int n=0; n<100; n++) {
      ASSERT_EQ(charConv(k*100+ n + 1),  mem->access(vecPtr[k], n));
    }
  }
}

TEST(AdvancedMemoryManagerTest, RemoveEmpty) {
  AdvancedMemoryManager *mem = new AdvancedMemoryManager(1000);

  auto ptr = mem->create(32);
  mem->remove(ptr);

  ASSERT_EQ(0, mem->getPointerList(32).size());
  ASSERT_EQ(0, mem->getChunkList(32).size());
}


// get out what you put in 
TEST(AdvancedMemoryManagerTest, RemoveHead) {
  AdvancedMemoryManager *mem = new AdvancedMemoryManager(1000);


  vector<AdvancedPointer *> vecPtr;

  char counter = 0;

  for(int k=0; k < 2; k++) {
    AdvancedPointer *ptr = mem->create(100);
    vecPtr.push_back(ptr);

    auto p = mem->getMemoryArray(ptr);

    for (auto info : p) {
      for (int n=0; n < info.second; n++) {
        counter++;
        info.first[n] = counter;
      }
    }
  }

  mem->remove(vecPtr[1]);

  for (int n=0; n<100; n++) {
    ASSERT_EQ(charConv(n+1),  mem->access(vecPtr[0], n));
  }
}

TEST(AdvancedMemoryManagerTest, RemoveOther) {
  AdvancedMemoryManager *mem = new AdvancedMemoryManager(1000);


  vector<AdvancedPointer *> vecPtr;

  char counter = 0;

  for(int k=0; k < 2; k++) {
    AdvancedPointer *ptr = mem->create(100);
    vecPtr.push_back(ptr);

    auto p = mem->getMemoryArray(ptr);

    for (auto info : p) {
      for (int n=0; n < info.second; n++) {
        counter++;
        info.first[n] = counter;
      }
    }
  }

  mem->remove(vecPtr[0]);

  for (int n=0; n<100; n++) {

    ASSERT_EQ(charConv(100+n+1),  mem->access(vecPtr[1], n));
  }
}


TEST(Remove, RandomRemove) {
  AdvancedMemoryManager *mem = new AdvancedMemoryManager(1000);

  // we always want the same test
  srand (42);


  for (int n=1; n <=1000; n++) {


    // cout<<"AFTER CONTINUE : " << n<<endl;

    vector<AdvancedPointer *> vecPtr;
    vector<bool> deleted;


    // cout<<"AFTER CONTINUE : " << n<<endl;

    char counter = 0;

    // create a random vector
    

    for(int k=0; k <  40 ||  k*n <= 4000; k++) {
      // cout<<"HERE"<<endl;
      AdvancedPointer *ptr = mem->create(n);
      vecPtr.push_back(ptr);
      deleted.push_back(false);

     // cout<<"after create"<<endl;

      auto p = mem->getMemoryArray(ptr);

     // cout<<"ACCESS "<<(ptr->pos)<<" SIZE : "<<p.size()<<endl;
    //  cout<<p[0].second<<" "<<p[1].second<<endl;

    //  cout<<mem->getChunkList(ptr->len).size()<<endl;

      for (auto info : p) {
        for (int n=0; n < info.second; n++) {
          counter++;
          // cout<<"n : "<<n<<endl;
          info.first[n] = counter;
        }
      }
      //cout<<"AFER ACCESS"<<endl;
    }

    // cout<<"REACH"<<endl;


    // randomly delete stuff 5 times
    for(int t=0; t<5; t++) {
      //cout<<"size : "<<vecPtr.size() <<" " <<deleted.size()<<endl;
      for (int k=0; k<vecPtr.size(); k++) {
        if (deleted[k]) {
          continue;
        }
        // cout<<"here"<<endl;
        if (rand() % 2 == 0) {
          deleted[k] = true;
          // mem->remove(vecPtr[k]);
          // cout<<"DELETE " <<k<<endl;
        } else {

          for(int j=0; j< n; j++) {
            ASSERT_EQ(charConv(k*n + j + 1),  mem->access(vecPtr[k], j)) 
              << "MESSAGE N : " << n
              << " k : " <<k 
              <<"  j : " << j 
              <<"  t : " << t
              <<endl;


          }
        }
      }
    }
  }

}


//
// // TESTS FOR EMPTY does not create anything
TEST(AdvancedMemoryManagerTest, EmptyTestChunkList) {
  AdvancedMemoryManager *mem = new AdvancedMemoryManager(1000);
  for (int n=0; n<=1000; n++) {
    ASSERT_EQ(0, mem->getChunkList(n).size());
  }
}

TEST(AdvancedMemoryManagerTest, EmptyTestPointerList) {
  AdvancedMemoryManager *mem = new AdvancedMemoryManager(1000);
  for (int n=0; n<=1000; n++) {
    ASSERT_EQ(0, mem->getPointerList(n).size());
  }
}


TEST(AdvancedMemoryManagerTest, CreateRightIndex) {

  for (int n=0; n < 1000; n++) {
    AdvancedMemoryManager *mem = new AdvancedMemoryManager(1000);

    auto ptr = mem->create(n);

    ASSERT_EQ(ptr->len, n);
    ASSERT_EQ(ptr->pos, 0);


    for (int i=0; i <= 1000; i++) {
      int result = i==n ? 1 : 0;
      ASSERT_EQ(result, mem->getPointerList(i).size());
      ASSERT_EQ(result, mem->getChunkList(i).size());

      if (result == 1) {
        auto listRef = mem->getPointerList(i).begin();

        // the pointer is the one listed
        ASSERT_EQ(listRef, ptr->ref);
        ASSERT_EQ(*listRef, ptr);
      }
    }
  }
}


TEST(AdvancedMemoryManagerTest, PointerPosition) {

  int chunkSize = 1000;
  AdvancedMemoryManager *mem = new AdvancedMemoryManager(chunkSize);

    for (int n=1; n <= chunkSize; n++) {

      stack<AdvancedPointer*> ptrStack;

      // test whether the position is set correctly
      for (int k=0; k<  3*chunkSize || k < 30  ; k++) {
        auto ptr = mem->create(n);

        ptrStack.push(ptr);

        // position of pointer is right
        ASSERT_EQ((k*n) % chunkSize, ptr->pos) <<
          "k : "<<k<<" n : " <<n <<" pos: " << ptr->pos
          <<endl;

        // length of pointer is right
        ASSERT_EQ(ptr->len, n);
        ASSERT_EQ(ptr->ref, mem->getPointerList(n).begin());


        // test the number of pointers
        ASSERT_EQ(k+1, mem->getPointerList(n).size());

        int space = (k+1)*n;

        ASSERT_EQ(
            space / chunkSize + ((space % chunkSize == 0) ? 0 : 1)
            ,mem->getChunkList(n).size()
        );
      }

      // check that the pointers are referencing the right objects
      for (auto it = mem->getPointerList(n).begin(); it != mem->getPointerList(n).end(); it++) {
        ASSERT_EQ(ptrStack.top()->ref, it);
        ASSERT_EQ(ptrStack.top(), *it);
        ptrStack.pop();
      }

    }
}


TEST(AdvancedMemoryManagerTest, FillTest) {

  int chunkSize = 100;
  AdvancedMemoryManager *mem = new AdvancedMemoryManager(chunkSize);


    // use the same chunk for testin
    for (int n=1; n <= chunkSize; n++) {

      if (n % 50 == 0) {
        cout <<" TESTING FILLING " <<n<<" of "<<chunkSize<<endl;
      }

      char *ch = new char[n];
      vector<AdvancedPointer *> vecPtr;

      char counter = 0;

      for (int k=0; k<  3*chunkSize || k < 30  ; k++) {
        AdvancedPointer *ptr = mem->create(n);

        for (int j=0; j<n; j++) {
          ch[j] = counter;
          counter++;
        }

        mem->fill(ptr, ch);

       for (int j=0; j<n; j++) {
         ASSERT_EQ(charConv(k*n+j), mem->access(ptr, j));
       }
      }
    }
}

TEST(AdvancedMemoryManagerTest, LoadTest) {

  int chunkSize = 100;
  AdvancedMemoryManager *mem = new AdvancedMemoryManager(chunkSize);


    // use the same chunk for testin
    for (int n=1; n <= chunkSize; n++) {

      if (n % 50 == 0) {
        cout <<" TESTING Loading " <<n<<" of "<<chunkSize<<endl;
      }

      char *ch = new char[n];
      vector<AdvancedPointer *> vecPtr;

      char counter = 0;

      for (int k=0; k<  3*chunkSize || k < 30  ; k++) {
        AdvancedPointer *ptr = mem->create(n);

        auto p = mem->getMemoryArray(ptr);

        for (auto info : p) {
          for (int n=0; n < info.second; n++) {
            counter++;
            info.first[n] = counter;
          }
        }

        mem->load(ptr, ch);

        // check whether the values loaded are the same
        int index = 0;
        for (auto info : p) {
          for (int n=0; n < info.second; n++) {
            ASSERT_EQ(info.first[n], ch[index]);
            index++;
          }
        }

      }
    }
}


int main( int argc, char** argv ) {
  printf("Running main() from sample1\n");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
  
}
