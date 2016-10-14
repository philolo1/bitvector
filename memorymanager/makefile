# g++ -isystem  ./external_lib/gtest/include -g -Wall -Wextra -pthread -lpthread mytest.cc lib/libgtest.a -o sample1_unittest

CC = g++

# configurations in ./make.config

# gtest library
LIB_GTEST = $(shell cat make.config  | grep LIB_GTEST |  sed 's/^[^=]*=//g')
INC_GTEST = $(shell cat make.config  | grep INC_GTEST |  sed 's/^[^=]*=//g')

LIB_SNAPPY = $(shell cat make.config  | grep LIB_SNAPPY |  sed 's/^[^=]*=//g')
INC_SNAPPY = $(shell cat make.config  | grep INC_SNAPPY |  sed 's/^[^=]*=//g')

LIB_LZ4 = $(shell cat make.config  | grep LIB_LZ4 |  sed 's/^[^=]*=//g')
INC_LZ4 = $(shell cat make.config  | grep INC_LZ4 |  sed 's/^[^=]*=//g')

LIB_SIMDCOMP = $(shell cat make.config  | grep LIB_SIMDCOMP |  sed 's/^[^=]*=//g')
INC_SIMDCOMP = $(shell cat make.config  | grep INC_SIMDCOMP |  sed 's/^[^=]*=//g')

LIB_VCOMPRESS = $(shell cat make.config  | grep LIB_VCOMPRESS|  sed 's/^[^=]*=//g')

INC_VCOMPRESS = $(shell cat make.config  | grep INC_VCOMPRESS|  sed 's/^[^=]*=//g')

CFLAGS = -mpopcnt -O2 -std=c++11 -isystem  $(INC_GTEST) -I $(INC_VCOMPRESS) -I $(INC_SIMDCOMP) -I $(INC_LZ4) -I $(INC_SNAPPY)
LM = -pthread $(LIB_GTEST) $(LIB_LZ4) $(LIB_SNAPPY) $(LIB_SIMDCOMP) $(LIB_VCOMPRESS)

test-1: test_AdvancedMemoryManager.cc
	$(CC) $(CFLAGS) test_AdvancedMemoryManager.cc $(LM) -o test-advanced_memorymanager.out
	$(CC) $(CFLAGS) test_SimpleMemoryManager.cc -o test-simpleMemoryManager.out $(LM)
	$(CC) $(CFLAGS) test_CompressedArray.cc -o test-compressedArray.out $(LM)
	$(CC) $(CFLAGS) avl/test_AVLTree.cc -o test-avlTree.out $(LM)
	$(CC) $(CFLAGS) test_DynamicBitVector.cc -o test-dynamicBitVector.out $(LM)
	$(CC) $(CFLAGS) datastructures/test_SetBitVector.cc -o test-SetBitVector.out $(LM)
	$(CC) $(CFLAGS) test_MemoryManager.cc -o test-memoryManager.out $(LM)
	$(CC) $(CFLAGS) test_simd_benchmark.cc -o test-simd_benchmark.out $(LM)

experiment: experiment_time.cc
	$(CC) $(CFLAGS) experiment_time.cc  $(LM) $(LIB_SNAPPY) -o experiment-time.out

test-allocator: test_allocatorSpeed.cc
	$(CC) $(CFLAGS) test_allocatorSpeed.cc -o test-allocator_speed.out $(LM)

test-helper: Helper.cc test_helper.cc
	$(CC) $(CFLAGS) test_helper.cc -o test-helper.out $(LM)

gexperiment:
	$(CC) -g  $(CFLAGS) experiment_time.cc -o experiment-time.out $(LM)

se-experiment-fbv: experiment_stackexchange_fbv.cc
	$(CC) -g  $(CFLAGS) experiment_stackexchange_fbv.cc -o fbv.out $(LM)

dyn:
	$(CC) $(CFLAGS) -g test_DynamicBitVector.cc -o test-dynamicBitVector.out $(LM)


pat: test_AdvancedMemoryManager.cc
	g++ -O2 -std=c++11  -o test-memorymanager.out test_AdvancedMemoryManager.cc -pthread -L../gtest/ -lgtest
	g++ -O2 -std=c++11  -o test-compressedArray.out test_CompressedArray.cc -pthread -L../gtest/ -lgtest -llz4

lib/libsnappy.a:
	@echo "building snappy"
	cd ./external_lib/snappy &&  bash autogen.sh &&  make clean && make
	cp ./external_lib/snappy/.libs/lib* ./lib/

lib/libgtest.a:
	@echo "building gtest"
	@echo "removing CmakeCache";
	cd ./external_lib/gtest && rm -f "CMakeCache.txt" && cmake  -G "Unix Makefiles" && make clean && make
	cp ./external_lib/gtest/*.a lib/

lib/libsimdcomp.so:
	@echo "building simdcomp"
	cd ./external_lib/simdcomp && make
	cp ./external_lib/simdcomp/libsimdcomp.so.0.0.3 ./lib/libsimdcomp.so

lib/liblz4.a:
	@echo "building lz4"
	cd ./external_lib/lz4 && make clean && make && make install DESTDIR=.
	cp ./external_lib/lz4/lib/lib* ./lib/

lib/libvcompress.a:
	@echo "building vcompress"
	cd ./external_lib/integer_encoding_library && make clean && make && make lib
	cp ./external_lib/integer_encoding_library/libvcompress.a ./lib/


lib: lib/libvcompress.a lib/libgtest.a lib/liblz4.a lib/libsnappy.a lib/libsimdcomp.so test_AdvancedMemoryManager.cc
	@echo "Bulding lib";

mem_ex_2.out: ./experiments/memory_experiment.cc
	g++ -std=c++11 -O2  ./experiments/memory_experiment.cc -o mem_ex_2.out

stack: experiment_stackexchange_naive_fix.cc
	$(CC) $(CFLAGS) -std=c++11 -mpopcnt experiment_stackexchange_naive_fix.cc -o naive_fix.out $(LM)
	$(CC) $(CFLAGS) -std=c++11 -mpopcnt experiment_stackexchange_fbv_fix.cc -o fbv_fix.out $(LM)
