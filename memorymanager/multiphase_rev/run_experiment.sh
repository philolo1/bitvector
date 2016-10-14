#!/bin/bash

########################################################################
# To use, install control groups and set up a group called
# limitMemoryGroup with a limit of 4GB For example: 
# 
# sudo cgcreate -t panichol:panichol -a panichol:panichol -g memory:limitMemoryGroup 
# echo 4000000000 > /sys/fs/cgroup/memory/memory./memory.limit_in_bytes
########################################################################
echo "RUNNING Normal."
for S in `seq 16 18`;
do
    rm -f  ./results/MULTIPHASE-$S.dat
    for E in `seq 14 24`;
    do
        for B in `seq 6 12`;
        do
            echo "NORMAL $B $E"
            ./multiphase.out normal $B $S $E 0 >> ./results/MULTIPHASE-$S.dat
            echo "SIMPLE $B $E"
            ./multiphase.out simple $B $S $E 0 >> ./results/MULTIPHASE-$S.dat
        done
    done    
done


