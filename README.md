
# A General Framework for Succinct and Compressed datastructures

This repository contains the essential part of the source code to reproduce the results of the paper "A General Framework for Succinct and Compressed datastructures" by Patrick Klitzke and Patrick Nicholson, represented at ALENEX 2016 (http://epubs.siam.org/doi/abs/10.1137/1.9781611974317.14).


## Environment

The repository has been tested on both Linux and Mac.

## Install Requirements

The following programs should be installed globally.

* [Clang](http://clang.llvm.org): clang is a modern C++ compiler. In theory also g++ should work to compile the files.
* [Fish](http://www.fishshell.com/): Fish is a shell like bash, with a more human syntax and is used for the experimental scripts.
* [Valgrind](http://valgrind.org/): Valgrind is a tool for measuring memory


## Configure the libraries

This step is only necessary if the program that should be run actually needs these libraries (compression) or one wants to reproduce the results in the paper.

In the folder `memorymanager` there is a file called `make.config`, which configures the path of the library as well as the include files and has the following default content:

```
LIB_GTEST = ~/googletest/googlemock/gtest/libgtest.a  ~/googletest/googlemock/gtest/libgtest_main.a
INC_GTEST = ~/googletest/googletest/include

LIB_SNAPPY = ~/snappy/.libs/libsnappy.a
INC_SNAPPY = ~/snappy

LIB_LZ4 = ~/lz4/lib/liblz4.a
INC_LZ4 = ~/lz4/lib/

LIB_SIMDCOMP = ~/simdcomp/libsimdcomp.so.0.0.3
INC_SIMDCOMP = ~/simdcomp/include

LIB_VCOMPRESS = ~/integer_encoding_library/libvcompress.a
INC_VCOMPRESS = ~/integer_encoding_library/include
```

The libraries to be added are
* gtest
* snappy
* lz4
* simdcomp
* vcompress

** Example on building for gtest

Lets see how to build the library gtest as an example for the other files: In the make.config there are two variables that one needs to set: The library path stored in the variable  `LIB_GTEST` and the header path with the variable `INC_GTEST`.

As of August 2016, we can do the following works the following way:

```
$ cd ~/external_lib
$ git clone https://github.com/google/googletest.git
$ cd googletest
$ cmake -G "Unix Makefiles"
$ make
```

After these steps we should have build gtest and now can update the two lines in our `make.config`:

LIB_GTEST = ./external_lib/googletest/googlemock/gtest/libgtest.a ./external_lib/googletest/googlemock/gtest/libgtest_main.a
INC_GTEST = ./external_lib/googletest/googletest/include

The other libraries work in a similar way.


## Reproducing the Experiment

At a first step to reproduce the experiments, the executables need to be build. The best way how to do that is by going in the memorymanager folder and running `make`:

```
$ make
```

After that we can run our experiments. The following instructions will describe how to create the data for the experiments not the graph representation.

** Experiment Allocator

The data produced by these experiments can be used to create the figures 2 and 3 of the paper: The comparison between malloc vs zonealloc.

To create the data for zonealloc we do the following commands:

```
$ cd ./memorymanager/multiphase
$ make
$ bash run_experiment.sh

```

To create the data for malloc do the followning commands:


```
$ cd ./memorymanager/multiphase_rev
$ make
$ bash run_experiment.sh

```

** Experiment Allocator

The data produced by these experiments can be used to create the figure 5 of the paper: The timing and space measurement of the operation rank and access for the implemented structures. To run these data do the following:

```
$ cd memorymanager
$ make experiment
$ mkdir data
$ fish test.fish # for timing measurements
$ mkdir data_space
$ fish test3.fish # for space measurements
```

## Experiments for huge graphs

The data produced by these experiments can be used to create the figure 5 of the paper: The time and space measurements of the graphs.
For the input  graphs one needs to first download the public data-dump from https://archive.org/details/stackexchange. After downloading the file we extract it and put all the files in folder. After this we convert it to a more readable format using a script:

```
$ cd memorymanager
$ python StackExchange2Graph.py [folder_with_xml_files] graph.txt
```

After the conversion we can run our experiments:

```
$ cd memorymanager
$ make stack
$ ./naive_fix.out graph.txt graph-naive.txt # for naive approach
$ ./fbv_fix.out  lz4 graph.txt graph-fbv.txt # for our structure approach

```

Other options than `lz4` are `nocomp`,`lz4`,`lzhc`,`simd`,`stl` for the first argument of `fbv_fix.out`.


## How to use this library

The source code is structured into three categories:

* The folder `datastructures` that includes the data structure for the such as the flexible bit vector.
* The folder `compression` that includes the implementations of the compression interface
* The MemoryManager.cc and `SimpleMemoryManager.cc`  file that is the implementation of the memory managers

To use one of the datastructures, one needs to include files from all three areas. Here is a small example:

```
#include "datastructures/FlexibleBitVector.cc" // flexible bit vector implementation
#include "compression/LZ4Compressor.cc" // lz4 compression
#include "MemoryManager.cc" // normal memorymanager


#define FLEX FlexibleBitVector<LZ4Compressor, MemoryManager, MemoryPointer>

...
FLEX bitvector(1, 2 * sizeof(uint32_t), 248);

uint32_t arr[2];

arr[0] = 0;
arr[1] = 2;

// creat a structre with a one block which contains M = 2 * sizeof(uint32_t) bits and S = 248
FLEX bitvector(1, 2 * sizeof(uint32_t), 248);

bitvector.addBlock(arr);

// insert an element in the block
bitvector.insertUint32_t(0, 12);

//  delete first elemenent
bitvector.deleteUint32_t(0);

// modify second element
bitvector.modifyUint32_t(1, 3);

// get rang(0)
bitvector.rank(0)

// select (1)
bitvector.select(1);

```

The rest can be seen by exploring the source code
