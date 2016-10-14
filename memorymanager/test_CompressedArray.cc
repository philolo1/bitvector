#include "CompressedArray.cc"
#include "compression/LZ4Compressor.cc"
#include "compression/LZ4CompressorHC.cc"
#include <gtest/gtest.h>

inline char charConv(char c) {
  return c;
}

#define CHUNK_SIZE 128
#define ITERATIONS 1000000

// get out what you put in 
TEST(AdvancedMemoryManagerTest, Compressed_scan_char_LZ4) {
  // 1024 * 1000000  byte

  CompressedArray<LZ4Compressor> array(CHUNK_SIZE);

  char *ch = new char[CHUNK_SIZE];

  for (int k=0; k < ITERATIONS; k++) {
    for (int n=0; n< CHUNK_SIZE; n++) {
      ch[n] = 0;
    }

    array.addBlock(ch);
  }


  for (int k=0; k < ITERATIONS; k++) {
    char *ch = array.accessBlock(k);
    for (int n=0; n< CHUNK_SIZE; n++) {
      ASSERT_EQ(charConv(0), ch[n]);
    }
  }

  cout<<"SIZE               : "<<array.getMemorySize()<<endl;

}

// get out what you put in 
TEST(AdvancedMemoryManagerTest, Compressed_scan_char_LZ4HC) {
  // 1024 * 1000000  byte

  CompressedArray<LZ4CompressorHC> array(CHUNK_SIZE);

  char *ch = new char[CHUNK_SIZE];

  for (int k=0; k < ITERATIONS; k++) {
    for (int n=0; n< CHUNK_SIZE; n++) {
      ch[n] = 0;
    }

    array.addBlock(ch);
  }


  for (int k=0; k < ITERATIONS; k++) {
    char *ch = array.accessBlock(k);
    for (int n=0; n< CHUNK_SIZE; n++) {
      ASSERT_EQ(charConv(0), ch[n]);
    }
  }

  cout<<"SIZE               : "<<array.getMemorySize()<<endl;

}

TEST(AdvancedMemoryManagerTest, Uncompressed_scan_char) {

  char *ch = new char[CHUNK_SIZE*ITERATIONS];
  int index = 0;


  for (int k=0; k < ITERATIONS; k++) {
    for (int n=0; n< CHUNK_SIZE; n++) {
      ch[index] = 0;
    }
  }

  for (int k=0; k < ITERATIONS; k++) {
    for (int n=0; n< CHUNK_SIZE; n++) {
      ASSERT_EQ(0, ch[index]);
    }
  }


  cout<<"UNCOMPRESSED SIZE   :"<<(CHUNK_SIZE * ITERATIONS)<<endl;


}

        
int main( int argc, char** argv ) {

/*  int* ch = new int[100];
  ch[0] = 1;
  ch[1] = 2;
  char* help = (char*)ch;
  int* ch2 = (int*)help;
  cout<<"first : "<<ch[1]<<endl;
*/

  printf("Running main() CompressedArray \n");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
  
}
