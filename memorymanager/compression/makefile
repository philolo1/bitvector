LIB_GTEST = $(shell cat ../make.config  | grep LIB_GTEST |  sed 's/^[^=]*=//g')
INC_GTEST = $(shell cat ../make.config  | grep INC_GTEST |  sed 's/^[^=]*=//g')

LIB_SNAPPY = $(shell cat ../make.config  | grep LIB_SNAPPY |  sed 's/^[^=]*=//g')
INC_SNAPPY = $(shell cat ../make.config  | grep INC_SNAPPY |  sed 's/^[^=]*=//g')

LIB_LZ4 = $(shell cat ../make.config  | grep LIB_LZ4 |  sed 's/^[^=]*=//g')
INC_LZ4 = $(shell cat ../make.config  | grep INC_LZ4 |  sed 's/^[^=]*=//g')

LIB_SIMDCOMP = $(shell cat ../make.config  | grep LIB_SIMDCOMP |  sed 's/^[^=]*=//g')
INC_SIMDCOMP = $(shell cat ../make.config  | grep INC_SIMDCOMP |  sed 's/^[^=]*=//g')

LIB_VCOMPRESS = $(shell cat ../make.config  | grep LIB_VCOMPRESS|  sed 's/^[^=]*=//g')

INC_VCOMPRESS = $(shell cat ../make.config  | grep INC_VCOMPRESS|  sed 's/^[^=]*=//g')

CFLAGS = -mpopcnt -O2 -std=c++11 -isystem  $(INC_GTEST) -I $(INC_VCOMPRESS) -I $(INC_SIMDCOMP) -I $(INC_LZ4) -I $(INC_SNAPPY)
LM = -pthread $(LIB_GTEST) $(LIB_LZ4) $(LIB_SNAPPY) $(LIB_SIMDCOMP) $(LIB_VCOMPRESS)

test: test_Compressor.cc
	g++ $(CFLAGS) test_Compressor.cc  $(LM) -o test_Compressor.out
