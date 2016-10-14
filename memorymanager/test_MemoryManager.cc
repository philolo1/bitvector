#include "MemoryManager.cc"
#include <gtest/gtest.h>

inline char charConv(char c) {
  return c;
}

// get out what you put in 
TEST(BigSMemoryManagerTest, bigS) {

  int S = 1000;
  for (int M=1; M<=30; M++) {
    MemoryManager *mem = new MemoryManager(S, M);

    for (int k=0; k<=100; k++) {
      MemoryPointer *firstPointer;

      int counter =0;

      for(int m=0; m < S/M ; m++) {
        MemoryPointer *ptr = mem->create(M);
        if (m==0) {
          firstPointer = ptr;
        }

        char *c = mem->getMemoryLocation(ptr);
        for (int k=0; k<M; k++) {
          counter++;
          c[k] = k;
          ASSERT_EQ(charConv(k),  mem->access(ptr, k));
        }
      }

      // test that everything is in the first one container
      for (int m=0; m<counter; m++) {
        int help = m %M;
        ASSERT_EQ(charConv(help),  mem->access(firstPointer, m));
      }
    }
  }
}



// get out what you put in 
TEST(MemoryManagerTest, AccessTest) {
  cout<<"A"<<endl;
  MemoryManager *mem = new MemoryManager(100000,1000);


  vector<MemoryPointer *> vecPtr;

  char counter = 0;

  for(int k=0; k < 30; k++) {

    MemoryPointer *ptr = mem->create(100);

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
TEST(MemoryManagerTest, RemoveHead) {
  MemoryManager *mem = new MemoryManager(1000);

  vector<MemoryPointer *> vecPtr;

  char counter = 0;

  for(int k=0; k < 2; k++) {
    MemoryPointer *ptr = mem->create(100);
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

TEST(MemoryManagerTest, RemoveOther) {
  MemoryManager *mem = new MemoryManager(1000);


  vector<MemoryPointer *> vecPtr;

  char counter = 0;

  for(int k=0; k < 2; k++) {
    MemoryPointer *ptr = mem->create(100);
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
  MemoryManager *mem = new MemoryManager(1000);

  // we always want the same test
  srand (42);


  for (int n=1; n <=1000; n++) {


    // cout<<"AFTER CONTINUE : " << n<<endl;

    vector<MemoryPointer *> vecPtr;
    vector<bool> deleted;


    // cout<<"AFTER CONTINUE : " << n<<endl;

    char counter = 0;

    // create a random vector
    

    for(int k=0; k <  40 ||  k*n <= 4000; k++) {
      // cout<<"HERE"<<endl;
      MemoryPointer *ptr = mem->create(n);
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

TEST(MemoryManagerTest, FillTest) {

  int chunkSize = 1000;
  MemoryManager *mem = new MemoryManager(100000,1000);


  // use the same chunk for testin
  for (int n=1; n <= chunkSize; n++) {

    if (n % 50 == 0) 
      cout <<" TESTING FILLING " <<n<<" of "<<chunkSize<<endl;

    char *ch = new char[n];
    vector<MemoryPointer *> vecPtr;

    char counter = 0;

    for (int k=0; k<  3*chunkSize || k < 30  ; k++) {
      MemoryPointer *ptr = mem->create(n);

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

TEST(MemoryManagerTest, LoadTest) {

  int chunkSize = 1000;
  MemoryManager *mem = new MemoryManager(100000, 1000);


    // use the same chunk for testin
    for (int n=1; n <= chunkSize; n++) {

      if (n % 50 == 0) {
        cout <<" TESTING Loading " <<n<<" of "<<chunkSize<<endl;
      }

      char *ch = new char[n];
      vector<MemoryPointer *> vecPtr;

      char counter = 0;

      for (int k=0; k<  1*chunkSize || k < 30  ; k++) {
        MemoryPointer *ptr = mem->create(n);

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
