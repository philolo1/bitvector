#include <gtest/gtest.h>

#include <iostream>


using namespace std;

#define CHUNK_SIZE 128
#define ITERATIONS 1000000



#ifndef CHAR_BIT
  #define CHAR_BIT 8
#endif

#ifndef INT_BIT
  #define INT_BIT 32
#endif

template<class BitVector>
void test1() {

//std::cout<<"test 1"<<std::endl;


  
  char *arr = new char[12];

  BitVector bitvector(
      1,
      12,
      20
  );

//std::cout<<"init"<<std::endl;

 
  for (int n=0; n < 12; n++) {
    arr[n] = 0;
  }

//std::cout<<"block"<<std::endl;




  bitvector.addBlock(arr);

//std::cout<<"modify"<<std::endl;


   bitvector.modify(CHAR_BIT, 1);



  for (int n=0; n< CHAR_BIT; n++) {
     ASSERT_EQ(0, bitvector.rank(n)) <<"rank N "<<n<<endl;;
    ASSERT_EQ(0, bitvector.access(n)) <<"acc N "<<n<<endl;;
  }

  for (int n=CHAR_BIT; n< 10*CHAR_BIT; n++) {
     ASSERT_EQ(1, bitvector.rank(n));
    ASSERT_EQ(n == CHAR_BIT, bitvector.access(n));
  }
  

    ASSERT_EQ(8, bitvector.select(0));
   for (int n=1; n< CHAR_BIT * 10; n++) {
     ASSERT_EQ(-1, bitvector.select(n)) <<" NUM : "<<n<<endl;;
   }
}

template <class BitVector>
void test2() {
  // make sure the tests are consistent
  // for each run
  srand(123);

  char arr[2][124];


  BitVector bitvector(
      2,
      124,
      124
      );

  for (int m=0; m< 2; m++) {
    for (int n=0; n < 124; n++) {
      arr[m][n] = rand() % 256;
    }
  }


  vector<int> ones;


  for (int m=0; m<2; m++) {
    bitvector.addBlock(arr[m]);
  }

  int rank = 0;


  int numChanges = 0;

  for (int m=0; m < 2; m++) {
    for (int n=0; n< 124; n++) {
      for (int k=0; k< CHAR_BIT; k++) {


        int index = m * 124*CHAR_BIT + n * CHAR_BIT + k;

        int bit = (arr[m][n] >>k) &1;


        // just set some random bits 
        if (rand() % 5 == 0) {
          if (bit) {
            numChanges--;
          } else {
            numChanges++;
          }
          bit = ! bit;
          bitvector.modify(index, bit);
        }

        if (bit) {
          ones.push_back(index);
        }



        rank += bit;

        ASSERT_EQ(rank, bitvector.rank(index)) <<"rank N "<<index<<endl;;
        ASSERT_EQ(bit, bitvector.access(index)) <<"acc N "<<index<<endl;;
      }
    }
  }

  for (int n=0; n < ones.size(); n++) {
    ASSERT_EQ(ones[n], bitvector.select(n));
  }

  for (int n=ones.size(); n< 124 * CHAR_BIT +10; n++) {
    ASSERT_EQ(-1, bitvector.select(n)) <<" NUM : "<<n<<endl;;
  }
}


template<class BitVector>
  void test4() {
  // make sure the test is consistent
  srand (15);

  int SIZE = sizeof(int) * 50;


  // lets do this 10 times to be sure we cover a lot
  for (int i = 0; i < 10; ++i) {

    char *arr = new char[SIZE];
    BitVector bitvector(
        1,
        SIZE,
        SIZE
        );

    for (int n=0; n < SIZE; n++) {
      arr[n] = rand()%256;
    }

    bitvector.addBlock(arr);


    vector<int> select;
    int rank = 0;

    // test rank
    for (int i = 0; i < SIZE; ++i) {
      for (int m=0; m < CHAR_BIT; m++) {
        if ((arr[i]>>m) & 1) {
          rank++;
          select.push_back(i * CHAR_BIT + m);

          // ASSERT_EQ(m + i * CHAR_BIT, bitvector.select(sel-1);
        }

        ASSERT_EQ( (arr[i] >> m) & 1,  bitvector.access(i*CHAR_BIT + m) );

        ASSERT_EQ( (arr[i] >> m) & 1,  bitvector.access(i*CHAR_BIT + m) );

        ASSERT_EQ(
            rank, 
            bitvector.rank(i*CHAR_BIT + m)) 
          <<  "NUM : " 
          << (i*CHAR_BIT + m)
          <<" "
          << (arr[0]&1 )
          <<" "
          << (arr[1]&2)
          <<endl;;
      }
    }

    // SELECT test
    for (int n=0; n < select.size(); n++) {
      ASSERT_EQ(select[n], bitvector.select(n)) 
        <<"HELP :"<<endl
        <<"N: "<<n<<endl
        <<(arr[0]&1) <<" "
        <<(arr[0]&2) <<" "
        <<(arr[0]&4) <<" "
        <<(arr[0]&8) <<" "
        <<endl;;
    }

    // invalid input test
    ASSERT_EQ(-1, bitvector.select(select.size()));
  }
  }




template<class BitVector>
void test3() {
  char *arr = new char[12];

std::cout<<"p hello wold"<<std::endl;

  BitVector bitvector(
      1,
      12,
      20
  );

  for (int n=0; n < 12; n++) {
    arr[n] = 0;
  }

  arr[1] = 1;

  bitvector.addBlock(arr);

  for (int n=0; n< CHAR_BIT; n++) {
    ASSERT_EQ(0, bitvector.rank(n)) <<"rank N "<<n<<endl;;
    ASSERT_EQ(0, bitvector.access(n)) <<"acc N "<<n<<endl;;
  }

  for (int n=CHAR_BIT; n< 12*CHAR_BIT; n++) {
    //// cout<<"N " <<n<<endl;;
    ASSERT_EQ(1, bitvector.rank(n));
    ASSERT_EQ(n == CHAR_BIT, bitvector.access(n));
    //// cout<<"after"<<endl;;
  }

  ASSERT_EQ(8, bitvector.select(0));
  for (int n=1; n< CHAR_BIT * 12; n++) {
    ASSERT_EQ(-1, bitvector.select(n)) <<" NUM : "<<n<<endl;;
  }
  }

template<class BitVector>
void test5() {

  int *arr1 = new int[10];
  int *arr2 = new int[10];

  int SIZE = 10 * sizeof(int);

  BitVector bitvector(
      2,
      SIZE,
      2*SIZE
  );

  for (int n=0; n < SIZE / sizeof(int); n++) {
    arr1[n] = 0;
    arr2[n] = 0;
  }

  arr1[1] = 1;
  arr2[5] = 1;

  bitvector.addBlock((char *)arr1);
  bitvector.addBlock((char *)arr2);


  vector<int> ones;
  ones.push_back(1*INT_BIT);
  ones.push_back(15*INT_BIT);

  ASSERT_EQ(2, bitvector.rank(480));

  return;

  ASSERT_EQ(1, bitvector.access(480));


  int rank = 0;


  for (int n=0; n< 20*INT_BIT; n++) {
    int bit = 0;
    if (rank < ones.size() && ones[rank] == n) {
      rank++;
      bit = 1;
    }
    //// cout<<"N : "<<n<<" rank : "<<rank<<endl;;
     ASSERT_EQ(rank, bitvector.rank(n)) <<"rank N "<<n<<endl;;
    ASSERT_EQ(bit, bitvector.access(n)) <<"acc N "<<n<<endl;;
  }

  for (int n=0; n < ones.size(); n++) {
    //// cout<<"SELECT : "<<n<<" pos: "<<ones[n]<<endl;;
    ASSERT_EQ(ones[n], bitvector.select(n));
  }

  for (int n=ones.size(); n< 20 * INT_BIT; n++) {
    ASSERT_EQ(-1, bitvector.select(n)) <<" NUM : "<<n<<endl;;
  }
}


template<class BitVector>
void test6() {
  // make sure the tests are consistent
  // for each run
  srand(123);



  int arr[2][60];

  int SIZE = 60 * sizeof(int);


  BitVector bitvector(
      2,
      SIZE,
      2 * SIZE
  );

  for (int m=0; m< 2; m++) {
    for (int n=0; n < SIZE / sizeof(int); n++) {
      arr[m][n] = rand() % 256;
    }
  }


  vector<int> ones;


  for (int m=0; m<2; m++) {
    bitvector.addBlock((char *)arr[m]);
  }


  int rank = 0;

  for (int m=0; m < 2; m++) {
    for (int n=0; n< SIZE / sizeof(int); n++) {
      for (int k=0; k< INT_BIT; k++) {
        int bit = (arr[m][n] >>k) &1;

        int index = m * SIZE*CHAR_BIT + n * INT_BIT + k;

        if (bit) {
          ones.push_back(index);
        }

        rank += bit;

////         cout<<"HERE !!! "<<index<<" " <<rank<<" "<<m<<" "<<n<<" "<<k<<endl;;
        ASSERT_EQ(rank, bitvector.rank(index)) <<"rank N "<<index<<endl;;
        ASSERT_EQ(bit, bitvector.access(index)) <<"acc N "<<index<<endl;;
      }
    }
  }

  for (int n=0; n < ones.size(); n++) {
    ASSERT_EQ(ones[n], bitvector.select(n));
  }
  
  for (int n=ones.size(); n< SIZE * CHAR_BIT +10; n++) {
    ASSERT_EQ(-1, bitvector.select(n)) <<" NUM : "<<n<<endl;;
  }
}

template<class BitVector>
void test7() {
  // make sure the tests are consistent
  // for each run
  srand(123);

  for (int k=3; k < 1026; k++) {

    if (k % 10 == 0) cout<<"Testin "<<k<<" of " <<1026<<endl;;

    int arr[k][31];


    BitVector bitvector(
        k,
        31 * sizeof(int),
        248
        );

    for (int m=0; m< k; m++) {
      for (int n=0; n < 31; n++) {
        arr[m][n] = rand() % 256;
     //   arr[m][n] = 1;
      }
    }

    vector<int> ones;

    for (int m=0; m<k; m++) {
      bitvector.addBlock((char *)arr[m]);
    }


    int rank = 0;

    for (int m=0; m < k; m++) {
      for (int n=0; n< 31; n++) {
        for (int k=0; k< INT_BIT; k++) {
          int bit = (arr[m][n] >>k) &1;

          int index = m * 31*INT_BIT + n * INT_BIT + k;

          if (rand() % 5 == 0) {
            bit = !bit;
            bitvector.modify(index, bit);
          } 

          if (bit) {
            ones.push_back(index);
          }

          rank += bit;

          ASSERT_EQ(rank, bitvector.rank(index)) <<"rank N "<<index<<endl;;
          ASSERT_EQ(bit, bitvector.access(index)) <<"acc N "<<index<<endl;;
        }
      }
    }

    for (int n=0; n < ones.size(); n++) {
      ASSERT_EQ(ones[n], bitvector.select(n));
    }

    for (int n=ones.size(); n< 124 * CHAR_BIT +10; n++) {
       ASSERT_EQ(-1, bitvector.select(n)) <<" NUM : "<<n<<endl;;
    }
  }
}
