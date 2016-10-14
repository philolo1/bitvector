LIB_GTEST = $(shell cat ../make.config  | grep LIB_GTEST |  sed 's/^[^=]*=//g')
INC_GTEST = $(shell cat ../make.config  | grep INC_GTEST |  sed 's/^[^=]*=//g')

all:
	g++ -std=c++11 -I$(INC_GTEST) $(LIB_GTEST) function_test.cc -o test_function
