CC = g++

LIB_GTEST = $(shell cat ../make.config  | grep LIB_GTEST |  sed 's/^[^=]*=//g')
INC_GTEST = $(shell cat ../make.config  | grep INC_GTEST |  sed 's/^[^=]*=//g')

CFLAGS = -std=c++11 -isystem $(INC_GTEST)
LM = -pthread $(LIB_GTEST)

test-1: test_AdvancedAVLTree.cc
	$(CC) $(CFLAGS) test_AdvancedAVLTree.cc -o test-advanced-avlTree.out $(LM)




