#!/bin/bash

RESULTDIR=results
mkdir $RESULTDIR

CSVFILEN=cachecoherence.csv
CCPROTOCOLS=( MSI MESI DRAGON )
CCPROTOCOLSN=( 0 1 2 )

rm ./$RESULTDIR/$CSVFILEN

#Sanity Check

CACHE_SIZE=8192
CACHE_ASSOC=8
CACHE_BLKSIZE=64

for ccprot in "${CCPROTOCOLSN[@]}"
do
	echo "Size:$CACHE_SIZE, Associativity:$CACHE_ASSOC, Block Size:$CACHE_BLKSIZE, Processor(s):4, Protocol:${CCPROTOCOLS[$ccprot]}, Trace:canneal.04t.longTrace"

	echo "./smp_cache $CACHE_SIZE $CACHE_ASSOC $CACHE_BLKSIZE 4 $ccprot canneal.04t.longTrace > ./$RESULTDIR/${CCPROTOCOLS[$ccprot]}_test.txt"
	./smp_cache $CACHE_SIZE $CACHE_ASSOC $CACHE_BLKSIZE 4 $ccprot ./canneal.04t.longTrace > ./$RESULTDIR/${CCPROTOCOLS[$ccprot]}_test.txti
done

#Cache Size Varied

CACHE_SIZE=( 262144 524288 1048576 2097152 )
CACHE_ASSOC=8
CACHE_BLKSIZE=64

for ccprot in "${CCPROTOCOLSN[@]}"
do
	for cache_size in "${CACHE_SIZE[@]}"
	do
		echo "Size: $cache_size, Associativity:$CACHE_ASSOC, Block Size:$CACHE_BLKSIZE, Processor(s):4, Protocol:${CCPROTOCOLS[$ccprot]}, Trace:canneal.04t.longTrace"

    		echo "./smp_cache $cache_size $CACHE_ASSOC $CACHE_BLKSIZE 4 $ccprot canneal.04t.longTrace >> ./$RESULTDIR/$CSVFILEN"
    		./smp_cache $cache_size $CACHE_ASSOC $CACHE_BLKSIZE 4 $ccprot ./canneal.04t.longTrace >> ./$RESULTDIR/$CSVFILEN
	done
done
echo "" >> ./$RESULTDIR/$CSVFILEN

#Cache Associativity Varied

CACHE_SIZE=1048576
CACHE_ASSOC=( 4 8 16 )
CACHE_BLKSIZE=64

for ccprot in "${CCPROTOCOLSN[@]}"
do
	for cache_assc in "${CACHE_ASSOC[@]}"
	do
		echo "Size: $CACHE_SIZE, Associativity:$cache_assc, Block Size:$CACHE_BLKSIZE, Processor(s):4, Protocol:${CCPROTOCOLS[$ccprot]}, Trace:canneal.04t.longTrace"

    		echo "./smp_cache $CACHE_SIZE $cache_assc $CACHE_BLKSIZE 4 $ccproc canneal.04t.longTrace >> ./$RESULTDIR/$CSVFILEN"
    		./smp_cache $CACHE_SIZE $cache_assc $CACHE_BLKSIZE 4 $ccprot ./canneal.04t.longTrace >> ./$RESULTDIR/$CSVFILEN
	done
done
echo "" >> ./$RESULTDIR/$CSVFILEN

#Cache Block Size Varied

CACHE_SIZE=1048576
CACHE_ASSOC=8
CACHE_BLKSIZE=( 64 128 256 )

for ccproc in "${CCPROTOCOLSN[@]}"
do
	for cache_blks in "${CACHE_BLKSIZE[@]}"
	do
		echo "Size: $CACHE_BLKSIZE, Associativity:$CACHE_ASSOC, Block Size:$cache_blks, Processor(s):4, Protocol:${CCPROTOCOLS[$ccprot]}, Trace:canneal.04t.longTrace"

    		echo "./smp_cache $CACHE_SIZE $CACHE_ASSOC $cache_blks 4 $ccproc canneal.04t.longTrace >> ./$RESULTDIR/$CSVFILEN"
    		./smp_cache $CACHE_SIZE $CACHE_ASSOC $cache_blks 4 $ccprot ./canneal.04t.longTrace >> ./$RESULTDIR/$CSVFILEN
	done
done

echo "EXPERIMENTS DONE"
