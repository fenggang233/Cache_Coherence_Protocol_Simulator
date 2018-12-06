#!/bin/bash

RESULTDIR=msi_results
mkdir $RESULTDIR

#Sanity Check

CACHE_SIZE=8192
CACHE_ASSOC=8
CACHE_BLKSIZE=64

echo "Size:$CACHE_SIZE, Associativity:$CACHE_ASSOC, Block Size:$CACHE_BLKSIZE, Processor(s):4, Trace:canneal.04t.longTrace"

echo "./smp_cache $CACHE_SIZE $CACHE_ASSOC $CACHE_BLKSIZE 4 0 canneal.04t.longTrace > ./$RESULTDIR/msi_test.txt"
./smp_cache $CACHE_SIZE $CACHE_ASSOC $CACHE_BLKSIZE 4 0 ./canneal.04t.longTrace > ./$RESULTDIR/msi_test.txt

#Cache Size Varied

CACHE_SIZE=( 262144 524288 1048576 2097152 )
CACHE_ASSOC=8
CACHE_BLKSIZE=64

echo "Size: 256kB 512kB 1MB 2MB, Associativity:$CACHE_ASSOC, Block Size:$CACHE_BLKSIZE, Processor(s):4, Trace:canneal.04t.longTrace"

for cache_size in "${CACHE_SIZE[@]}"
do
    echo "./smp_cache $cache_size $CACHE_ASSOC $CACHE_BLKSIZE 4 0 canneal.04t.longTrace > ./$RESULTDIR/msi_size_$cache_size.txt"
    ./smp_cache $cache_size $CACHE_ASSOC $CACHE_BLKSIZE 4 0 ./canneal.04t.longTrace > ./$RESULTDIR/msi_size_$cache_size.txt
done

#Cache Associativity Varied

CACHE_SIZE=1048576
CACHE_ASSOC=( 4 8 16 )
CACHE_BLKSIZE=64

echo "Size: $CACHE_SIZE, Associativity:4 8 16, Block Size:$CACHE_BLKSIZE, Processor(s):4, Trace:canneal.04t.longTrace"

for cache_assc in "${CACHE_ASSOC[@]}"
do
    echo "./smp_cache $CACHE_SIZE $cache_assc $CACHE_BLKSIZE 4 0 canneal.04t.longTrace > ./$RESULTDIR/msi_assc_$cache_assc.txt"
    ./smp_cache $CACHE_SIZE $cache_assc $CACHE_BLKSIZE 4 0 ./canneal.04t.longTrace > ./$RESULTDIR/msi_assc_$cache_assc.txt
done

#Cache Block Size Varied

CACHE_SIZE=1048576
CACHE_ASSOC=8
CACHE_BLKSIZE=( 64 128 256 )

echo "Size: $CACHE_BLKSIZE, Associativity:$CACHE_ASSOC, Block Size:64 128 256, Processor(s):4, Trace:canneal.04t.longTrace"

for cache_blks in "${CACHE_BLKSIZE[@]}"
do
    echo "./smp_cache $CACHE_SIZE $CACHE_ASSOC $cache_blks 4 0 canneal.04t.longTrace > ./$RESULTDIR/msi_blks_$cache_blks.txt"
    ./smp_cache $CACHE_SIZE $CACHE_ASSOC $cache_blks 4 0 ./canneal.04t.longTrace > ./$RESULTDIR/msi_blks_$cache_blks.txt
done

echo "MSI EXPERIMENT DONE"
