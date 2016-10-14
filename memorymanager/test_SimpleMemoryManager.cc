#include "SimpleMemoryManager.cc"
#include <gtest/gtest.h>

inline char charConv(char c) {
  return c;
}

// get out what you put in 
TEST(SimpleMemoryManagerTest, AccessTest) {
  SimpleMemoryManager *mem = new SimpleMemoryManager(1000);


  vector<SimplePointer *> vecPtr;

  char counter = 0;

  for(int k=0; k < 30; k++) {
    SimplePointer *ptr = mem->create(100);
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


// get out what you put in 
TEST(SimpleMemoryManagerTest, RemoveHead) {
  SimpleMemoryManager *mem = new SimpleMemoryManager(1000);


  vector<SimplePointer *> vecPtr;

  char counter = 0;

  for(int k=0; k < 2; k++) {
    SimplePointer *ptr = mem->create(100);
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

TEST(SimpleMemoryManagerTest, RemoveOther) {
  SimpleMemoryManager *mem = new SimpleMemoryManager(1000);


  vector<SimplePointer *> vecPtr;

  char counter = 0;

  for(int k=0; k < 2; k++) {
    SimplePointer *ptr = mem->create(100);
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
  SimpleMemoryManager *mem = new SimpleMemoryManager(1000);

  // we always want the same test
  srand (42);


  for (int n=1; n <=1000; n++) {


    // cout<<"AFTER CONTINUE : " << n<<endl;

    vector<SimplePointer *> vecPtr;
    vector<bool> deleted;


    // cout<<"AFTER CONTINUE : " << n<<endl;

    char counter = 0;

    // create a random vector
    

    for(int k=0; k <  40 ||  k*n <= 4000; k++) {
      // cout<<"HERE"<<endl;
      SimplePointer *ptr = mem->create(n);
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



TEST(SimpleMemoryManagerTest, FillTest) {

  int chunkSize = 1000;
  SimpleMemoryManager *mem = new SimpleMemoryManager(chunkSize);


    // use the same chunk for testin
    for (int n=1; n <= chunkSize; n++) {

      cout <<" TESTING FILLING " <<n<<" of "<<chunkSize<<endl;

      char *ch = new char[n];
      vector<SimplePointer *> vecPtr;

      char counter = 0;

      for (int k=0; k<  3*chunkSize || k < 30  ; k++) {
        SimplePointer *ptr = mem->create(n);

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

TEST(SimpleMemoryManagerTest, LoadTest) {

  int chunkSize = 1000;
  SimpleMemoryManager *mem = new SimpleMemoryManager(chunkSize);


    // use the same chunk for testin
    for (int n=1; n <= chunkSize; n++) {

      cout <<" TESTING Loading " <<n<<" of "<<chunkSize<<endl;

      char *ch = new char[n];
      vector<SimplePointer *> vecPtr;

      char counter = 0;

      for (int k=0; k<  3*chunkSize || k < 30  ; k++) {
        SimplePointer *ptr = mem->create(n);

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
