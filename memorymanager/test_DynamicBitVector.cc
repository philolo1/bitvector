#include "datastructures/DynamicBitVector.cc"
#include "compression/LZ4Compressor.cc"
#include "compression/LZ4CompressorHC.cc"
#include <gtest/gtest.h>

#include "AdvancedMemoryManager.cc"

#include <iostream>


#define CHUNK_SIZE 128
#define ITERATIONS 1000000
#define INT_BIT sizeof(uint32_t) * CHAR_BIT

using namespace std;

TEST(DynamicBitVector, MODIFY) {

  uint32_t *arr = new uint32_t[12];

  DynamicBitVector<LZ4Compressor, AdvancedMemoryManager, AdvancedPointer> bitvector(
      1,
      12 * sizeof(uint32_t),
      12 * sizeof(uint32_t)
  );

  for (int n=0; n < 12; n++) {
    arr[n] = 0;
  }

  bitvector.addBlock(arr);

  bitvector.modify(CHAR_BIT, 1);


  for (int n=0; n< CHAR_BIT; n++) {
    ASSERT_EQ(0, bitvector.rank(n)) <<"rank N "<<n<<endl;
    ASSERT_EQ(0, bitvector.access(n)) <<"acc N "<<n<<endl;
  }

  for (int n=CHAR_BIT; n< 12*CHAR_BIT; n++) {
    // cout<<"N " <<n<<endl;
    ASSERT_EQ(1, bitvector.rank(n));
    ASSERT_EQ(n == CHAR_BIT, bitvector.access(n));
    // cout<<"after"<<endl;
  }

  ASSERT_EQ(8, bitvector.select(0));
  for (int n=1; n< CHAR_BIT * 12; n++) {
    ASSERT_EQ(-1, bitvector.select(n)) <<" NUM : "<<n<<endl;
  }
}

TEST(DynamicBitVector, TWO_BLOCKS_RANDOM_MOD) {
  // make sure the tests are consistent
  // for each run
  srand(123);

  const int SIZE = 124;

  uint32_t arr[2][SIZE];


  DynamicBitVector<LZ4Compressor, AdvancedMemoryManager, AdvancedPointer> bitvector(
      2,
      SIZE * sizeof(uint32_t),
      SIZE * sizeof(uint32_t)
      );

  for (int m=0; m< 2; m++) {
    for (int n=0; n < SIZE; n++) {
      arr[m][n] = rand();
    }
  }


  vector<int> ones;


  for (int m=0; m<2; m++) {
    bitvector.addBlock(arr[m]);
  }

  int rank = 0;


        int numChanges = 0;

  for (int m=0; m < 2; m++) {
    for (int n=0; n< SIZE; n++) {
      for (int k=0; k< sizeof(uint32_t) * CHAR_BIT; k++) {


        int index = m * SIZE*CHAR_BIT * sizeof(uint32_t) + n * CHAR_BIT * sizeof(uint32_t)  + k;

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

        ASSERT_EQ(rank, bitvector.rank(index)) <<"rank N "<<index<<endl;
        ASSERT_EQ(bit, bitvector.access(index)) <<"acc N "<<index<<endl;
      }
    }
  }

  for (int n=0; n < ones.size(); n++) {
    ASSERT_EQ(ones[n], bitvector.select(n));
  }

  for (int n=ones.size(); n< SIZE * CHAR_BIT * sizeof(uint32_t) +10; n++) {
    ASSERT_EQ(-1, bitvector.select(n)) <<" NUM : "<<n<<endl;
  }
}

// // get out what you put in 
 TEST(DynamicBitVector, ONE_BIT_SET_ACCESS_RANK) {

   uint32_t *arr = new uint32_t[12];

   DynamicBitVector<LZ4Compressor, AdvancedMemoryManager, AdvancedPointer> bitvector(
       1,
       12 * sizeof(uint32_t),
       12 * sizeof(uint32_t) 
   );

   for (int n=0; n < 12; n++) {
     arr[n] = 0;
   }

   arr[1] = 1;

   bitvector.addBlock(arr);

   for (int n=0; n< INT_BIT; n++) {
     ASSERT_EQ(0, bitvector.rank(n)) <<"rank N "<<n<<endl;
     ASSERT_EQ(0, bitvector.access(n)) <<"acc N "<<n<<endl;
   }

   for (int n=INT_BIT; n< 12*INT_BIT; n++) {
     ASSERT_EQ(1, bitvector.rank(n));
     ASSERT_EQ(n == INT_BIT, bitvector.access(n));
   }

   ASSERT_EQ(INT_BIT, bitvector.select(0));
   for (int n=1; n< INT_BIT * 12; n++) {
     ASSERT_EQ(-1, bitvector.select(n)) <<" NUM : "<<n<<endl;
   }
 }

 TEST(DynamicBitVector, OneBlock_COMPLICATED_TEST) {

   // make sure the test is consistent
   srand (15);

   // lets do this 12 times to be sure we cover a lot
   for (int i = 0; i < 12; ++i) {
     uint32_t *arr = new uint32_t[100];

     DynamicBitVector<LZ4Compressor, AdvancedMemoryManager, AdvancedPointer> bitvector(
         1,
         100 * sizeof(uint32_t),
         100 * sizeof(uint32_t)
         );

     for (int n=0; n < 100; n++) {
       arr[n] = rand();
     }

     bitvector.addBlock(arr);


     vector<int> select;
     int rank = 0;

     // test rank
     for (int i = 0; i < 100; ++i) {
       for (int m=0; m < INT_BIT; m++) {
         if ((arr[i]>>m) & 1) {
           rank++;
           select.push_back(i * INT_BIT + m);

           // ASSERT_EQ(m + i * CHAR_BIT, bitvector.select(sel-1);
         }

         ASSERT_EQ( (arr[i] >> m) & 1,  bitvector.access(i*INT_BIT + m) );

         ASSERT_EQ( (arr[i] >> m) & 1,  bitvector.access(i*INT_BIT+ m) );

         ASSERT_EQ(
             rank, 
             bitvector.rank(i*INT_BIT + m)) 
           <<  "NUM : " 
           << (i*INT_BIT + m)
           <<" "
           << (arr[0]&1 )
           <<" "
           << (arr[1]&2)
           <<endl;
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
         <<endl;
     }

     // invalid input test
     ASSERT_EQ(-1, bitvector.select(select.size()));
   }
 }


 // get out what you put in 
 TEST(DynamicBitVector, TWO_BLOCKS) {

   uint32_t *arr1 = new uint32_t[12];
   uint32_t *arr2 = new uint32_t[12];


   DynamicBitVector<LZ4Compressor, AdvancedMemoryManager, AdvancedPointer> bitvector(
       2,
       12 * sizeof(uint32_t) ,
       12 * sizeof(uint32_t) 
   );

   for (int n=0; n < 12; n++) {
     arr1[n] = 0;
     arr2[n] = 0;
   }

   arr1[1] = 1;
   arr2[5] = 1;

   vector<int> ones;
   ones.push_back(1*INT_BIT);
   ones.push_back((12 + 5)*INT_BIT);


   bitvector.addBlock(arr1);
   bitvector.addBlock(arr2);

   int rank = 0;


   for (int n=0; n< 24*INT_BIT; n++) {
     int bit = 0;
     if (rank < ones.size() && ones[rank] == n) {
       rank++;
       bit = 1;
     }
     // cout<<"N : "<<n<<" rank : "<<rank<<endl;
     ASSERT_EQ(rank, bitvector.rank(n)) <<"rank N "<<n<<endl;
     ASSERT_EQ(bit, bitvector.access(n)) <<"acc N "<<n<<endl;
   }

   for (int n=0; n < ones.size(); n++) {
     // cout<<"SELECT : "<<n<<" pos: "<<ones[n]<<endl;
     ASSERT_EQ(ones[n], bitvector.select(n));
   }

   for (int n=ones.size(); n< INT_BIT * 24; n++) {
     ASSERT_EQ(-1, bitvector.select(n)) <<" NUM : "<<n<<endl;
   }
 }

 TEST(DynamicBitVector, TWO_BLOCKS_RANDOM) {
   // make sure the tests are consistent
   // for each run
   srand(123);

   uint32_t arr[2][124];


   DynamicBitVector<LZ4Compressor, AdvancedMemoryManager, AdvancedPointer> bitvector(
       2,
       124 * sizeof(uint32_t),
       124 * sizeof(uint32_t)
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

   for (int m=0; m < 2; m++) {
     for (int n=0; n< 124; n++) {
       for (int k=0; k< INT_BIT; k++) {
         int bit = (arr[m][n] >>k) &1;

         int index = m * 124*INT_BIT + n * INT_BIT + k;

         if (bit) {
           ones.push_back(index);
         }

         rank += bit;

         ASSERT_EQ(rank, bitvector.rank(index)) <<"rank N "<<index<<endl;
         ASSERT_EQ(bit, bitvector.access(index)) <<"acc N "<<index<<endl;
       }
     }
   }

   for (int n=0; n < ones.size(); n++) {
     ASSERT_EQ(ones[n], bitvector.select(n));
   }
   
   for (int n=ones.size(); n< 124 * INT_BIT +10; n++) {
     ASSERT_EQ(-1, bitvector.select(n)) <<" NUM : "<<n<<endl;
   }
 }

 



 TEST(DynamicBitVector, K_BLOCKS_RANDOM_MOD) {
   // make sure the tests are consistent
   // for each run
   srand(123);

   for (int k=3; k < 20; k++) {

     if (k % 100 == 0) {
       cout<<"Testin "<<k<<" of " <<1024<<endl;
     }

     uint32_t arr[k][120];

     DynamicBitVector<LZ4Compressor, AdvancedMemoryManager, AdvancedPointer> bitvector(
         k,
         120 * sizeof(uint32_t),
         120 * sizeof(uint32_t)
         );

     for (int m=0; m< k; m++) {
       for (int n=0; n < 120; n++) {
         arr[m][n] = rand();
       }
     }

     vector<int> ones;

     for (int m=0; m<k; m++) {
       bitvector.addBlock(arr[m]);
     }

     int rank = 0;

     for (int m=0; m < k; m++) {
       for (int n=0; n< 120; n++) {
         for (int k=0; k< INT_BIT; k++) {
           int bit = (arr[m][n] >>k) &1;

           int index = m * 120*INT_BIT + n * INT_BIT + k;

           if (rand() % 5 == 0) {
             bit = !bit;
             bitvector.modify(index, bit);
           }

           if (bit) {
             ones.push_back(index);
           }

           rank += bit;

           ASSERT_EQ(rank, bitvector.rank(index)) <<"rank N "<<index<<endl;
           ASSERT_EQ(bit, bitvector.access(index)) <<"acc N "<<index<<endl;
         }
       }
     }

     for (int n=0; n < ones.size(); n++) {
       ASSERT_EQ(ones[n], bitvector.select(n));
     }

     for (int n=ones.size(); n< 120 * INT_BIT +10; n++) {
       ASSERT_EQ(-1, bitvector.select(n)) <<" NUM : "<<n<<endl;
     }
   }
 }



int main( int argc, char** argv ) {

  printf("Running main() DynamicBitvector \n");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
  
}
