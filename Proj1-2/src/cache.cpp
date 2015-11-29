#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "cache.h"

void Cache::init(unsigned int size, unsigned int assoc, unsigned int set, unsigned int tag) {
    SIZE = size;
    ASSOC = assoc;
    SET = set;
    TAG = tag;

    LRUC = (unsigned int*)malloc( (TAG*sizeof(unsigned int)) );
    TAGS = (unsigned int*)malloc( (TAG*sizeof(unsigned int)) );
    DIRTY = (unsigned int*)malloc( (TAG*sizeof(unsigned int)) );
    VALID = (unsigned int*)malloc( (TAG*sizeof(unsigned int)) );

    memset( LRUC, 0, (sizeof(LRUC[0])*TAG) );
    memset( TAGS, 0, (sizeof(TAGS[0])*TAG) );
    memset( DIRTY, 0, (sizeof(DIRTY[0])*TAG) );
    memset( VALID, 0, (sizeof(VALID[0])*TAG) );

    NUM_READ = 0;
    NUM_WRITE = 0;
    NUM_READ_MISS = 0;
    NUM_WRITE_MISS = 0;
    NUM_ACCESS = 0;
    NUM_WRITEBACK = 0;
    MISS_RATE = 0;
    MISS_PENALTY = 0;
    HIT_TIME = 0;
    ACCESS_TIME = 0;
    nextLevel = NULL;
}

void CACHE::init(unsigned int block, unsigned int size1, unsigned int assoc1, unsigned int sizev, unsigned int size2, unsigned int assoc2, char *trace) {
    TRACE_FILE = trace;
    BLOCKSIZE = block;
    L1.init(size1, assoc1, size1/(block*assoc1), size1/block);
    if( size2 > 0 ){
        L1.nextLevel = &L2;
        L2.init(size2, assoc2, size2/(block*assoc2), size2/block);
    }
    if( sizev > 0 )
        Victim.init(sizev, sizev/block, 1, sizev/block);
}

void CACHE::input() {
    char rw;
    unsigned int address;
    while( scanf(" %c %x",&rw,&address)!=EOF ) {
        //transAddress()
        if( rw == 'r' || rw == 'R')
            readFromAddress(L1, address, Victim.SIZE);
        else if( rw == 'w' || rw == 'W')
            writeToAddress(L1, address, Victim.SIZE);
    }
}

void CACHE::output() {
    int i,j,k;
    printf("===== Simulator configuration =====\n");
    printf("BLOCKSIZE:                    %d\n", BLOCKSIZE);
    printf("L1_SIZE:                      %d\n", L1.SIZE);
    printf("L1_ASSOC:                     %d\n", L1.ASSOC);
    printf("Victim_Cache_SIZE:            %d\n", Victim.SIZE);
    printf("L2_SIZE:                      %d\n", L2.SIZE);
    printf("L2_ASSOC:                     %d\n", L2.ASSOC);
    printf("trace_file:                   %s\n", TRACE_FILE);
    printf("===================================\n");
    printf("===== L1 contents =====\n");

    for(i=0; i<L1.SET; i++) {
        //sort L1
        for(j=1; j<L1.ASSOC; j++) {
            LRU_KEY = L1.LRUC[i+(j*L1.SET)];
            TAG_KEY = L1.TAGS[i+(j*L1.SET)];
            DIR_KEY = L1.DIRTY[i+(j*L1.SET)];
            for(k = j-1; k>=0 && LRU_KEY<L1.LRUC[i+(k*L1.SET)]; k--) {
                L1.LRUC[i+((k+1)*L1.SET)] = L1.LRUC[i+(k*L1.SET)];
                L1.TAGS[i+((k+1)*L1.SET)] = L1.TAGS[i+(k*L1.SET)];
                L1.DIRTY[i+((k+1)*L1.SET)] = L1.DIRTY[i+(k*L1.SET)];
            }
            L1.LRUC[i+((k+1)*L1.SET)] = LRU_KEY;
            L1.TAGS[i+((k+1)*L1.SET)] = TAG_KEY;
            L1.DIRTY[i+((k+1)*L1.SET)] = DIR_KEY;
        }
        printf("set %d: ", i);
        for(j=0; j<L1.ASSOC; j++)
            printf("%x %c  ",L1.TAGS[i+(j*L1.SET)], L1.DIRTY[i+(j*L1.SET)]?'D':' ');
        puts("");
    }

    if( Victim.SIZE ) {
        printf("===== Victim Cache contents =====\n");
        printf("set 0: ");
        //sort Victim
        for( i=1; i<Victim.ASSOC; i++) {
            LRU_KEY = Victim.LRUC[i];
            TAG_KEY = Victim.TAGS[i];
            DIR_KEY = Victim.DIRTY[i];
            for( j = i-1; j>=0 && LRU_KEY<Victim.LRUC[j];j--) {
                Victim.LRUC[j+1] = Victim.LRUC[j];
                Victim.TAGS[j+1] = Victim.TAGS[j];
                Victim.DIRTY[j+1] = Victim.DIRTY[j];
            }
            Victim.LRUC[j+1] = LRU_KEY;
            Victim.TAGS[j+1] = TAG_KEY;
            Victim.DIRTY[j+1] = DIR_KEY;
        }
        for(i=0; i<Victim.ASSOC; i++)
            printf("%x %c  ",Victim.TAGS[i], Victim.DIRTY[i]?'D':' ');
        puts("");
    }

    if( L2.SIZE ) {
        printf("===== L2 contents =====\n");
        for( i=0; i<L2.SET; i++) {
            //sort L2
            for( j=1; j<L2.ASSOC; j++) {
                LRU_KEY = L2.LRUC[i+(j*L2.SET)];
                TAG_KEY = L2.TAGS[i+(j*L2.SET)];
                DIR_KEY = L2.DIRTY[i+(j*L2.SET)];
                for( k = j-1; k>=0 && LRU_KEY<L2.LRUC[i+(k*L2.SET)]; k-- ) {
                    L2.LRUC[i+((k+1)*L2.SET)] = L2.LRUC[i+(k*L2.SET)];
                    L2.TAGS[i+((k+1)*L2.SET)] = L2.TAGS[i+(k*L2.SET)];
                    L2.DIRTY[i+((k+1)*L2.SET)] = L2.DIRTY[i+(k*L2.SET)];
                }
                L2.LRUC[i+((k+1)*L2.SET)] = LRU_KEY;
                L2.TAGS[i+((k+1)*L2.SET)] = TAG_KEY;
                L2.DIRTY[i+((k+1)*L2.SET)] = DIR_KEY;

            }
            printf("set %d: ", i);
            for(j=0; j<L2.ASSOC; j++)
                printf("%x %c  ",L2.TAGS[i+(j*L2.SET)], L2.DIRTY[i+(j*L2.SET)]?'D':' ');
            puts("");
        }
    }

    //L1 calculation
    L1.MISS_RATE = double(L1.NUM_READ_MISS + L1.NUM_WRITE_MISS)/double(L1.NUM_READ + L1.NUM_WRITE );
    L1.MISS_PENALTY = 20 + 0.5*BLOCKSIZE/16;
    L1.HIT_TIME = 0.25 + 2.5*L1.SIZE/(512*1024) + 0.025*BLOCKSIZE/16 + 0.025*L1.ASSOC;
    L1.ACCESS_TIME = L1.HIT_TIME + L1.MISS_RATE*L1.MISS_PENALTY;

    printf("====== Simulation results (raw) ======\n");
    printf("a. number of L1 reads:                %d\n", L1.NUM_READ);
    printf("b. number of L1 read misses:          %d\n", L1.NUM_READ_MISS);
    printf("c. number of L1 writes:\t\t     %d\n",       L1.NUM_WRITE);
    printf("d. number of L1 write misses:         %d\n", L1.NUM_WRITE_MISS);
    printf("e. L1 miss rate:                      %.4f\n", L1.MISS_RATE);
    printf("f. number of swaps:                   %d\n", Victim.NUM_SWAP);
    printf("g. number of victim cache writeback:  %d\n", Victim.NUM_WRITEBACK);

    //L2 calculation
    L2.MISS_RATE = (double)L2.NUM_READ_MISS/(double)L2.NUM_READ;
    L2.MISS_PENALTY = 20 + 0.5*BLOCKSIZE/16;
    L2.HIT_TIME = 2.5 + 2.5*L2.SIZE/(512*1024) + 0.025*BLOCKSIZE/16 + 0.025*L2.ASSOC;
    L2.ACCESS_TIME = L1.HIT_TIME + L1.MISS_RATE*( L2.HIT_TIME + L2.MISS_RATE*L2.MISS_PENALTY );

    printf("h. number of L2 reads:                %d\n", L2.NUM_READ);
    printf("i. number of L2 read misses:          %d\n", L2.NUM_READ_MISS);
    printf("j. number of L2 writes:               %d\n", L2.NUM_WRITE);
    printf("k. number of L2 write misses:         %d\n", L2.NUM_WRITE_MISS);
    if( L2.SIZE )
        printf("l. L2 miss rate:                      %.4f\n", L2.MISS_RATE);
    else
        printf("l. L2 miss rate:                      0\n");
    printf("m. number of L2 writebacks:           %d\n", L2.NUM_WRITEBACK);
    printf("n. total memory traffic:              %d\n", L2.SIZE? L2.NUM_ACCESS:L1.NUM_ACCESS);
    printf("==== Simulation results (performance) ====\n");
    printf("1. average access time:         %.4f ns\n", L2.SIZE ? L2.ACCESS_TIME : L1.ACCESS_TIME);
}

void CACHE::extractAddressParams(unsigned int address, Cache cache_ds, unsigned int* INDEX, unsigned int* TAG_ADD)
{
    int noOfBlockBits = 0, noOfIndexBits = 0, tempIndexNo = 0, i=0;

    noOfBlockBits = (int)log2(BLOCKSIZE);
    noOfIndexBits = (int)log2(cache_ds.SET);

    *INDEX = address>>noOfBlockBits;

    for( i=0; i<noOfIndexBits; i++)
    {
        tempIndexNo = ( 1 | tempIndexNo<<1 );
    }

    *INDEX = ( *INDEX & tempIndexNo );
    *TAG_ADD = address>>(noOfBlockBits + noOfIndexBits);
}

/*void CACHE::transAddress(Cache &cache_ds, unsigned int address) {
    int block = (int)log2(BLOCKSIZE);
    int index = (int)log2(cache_ds.SET);
    int tmp = 0;
    for( int i=0; i<index; i++)
        tmp = tmp<<1 | 1;
    INDEX = address>>block & tmp;
    TAG_LOC = address>>(block+index);
}*/



//Recursive solution

void CACHE::readFromAddress(Cache &cache_ds, unsigned int address, unsigned int vc_size)
{
    int i=0, foundInvalidEntry = 0, noOfBlockBits = 0, noOfIndexBits = 0;
    unsigned int TAG_LOC = 0, INDEX = 0, TAG_ADD = 0, TMP_ADD=0, TMP_LOC = 0;

    noOfBlockBits = (int)log2(BLOCKSIZE);
    noOfIndexBits = (int)log2(cache_ds.SET);


    cache_ds.NUM_READ +=1;
    extractAddressParams(address, cache_ds, &INDEX, &TAG_ADD);


    for( i=0; i< (int)cache_ds.ASSOC; i++)
    {
        if( cache_ds.TAGS[INDEX + (i*cache_ds.SET)] == TAG_ADD )	//Checking Tag Entries
        {

            if( cache_ds.VALID[INDEX + (i*cache_ds.SET)] != 0 )
            {
                LRUForHit(cache_ds, INDEX, ( INDEX + (i*cache_ds.SET) ) );

                return;
            }
        }
    }


    for( i=0; i< (int)cache_ds.ASSOC; i++)
    {
        if( cache_ds.VALID[INDEX + (i*cache_ds.SET)] == 0 )
        {
            cache_ds.VALID[INDEX + (i*cache_ds.SET)] = 1;
            TAG_LOC =  INDEX + (i*cache_ds.SET);
            foundInvalidEntry = 1;
            break;
        }
    }

    if( foundInvalidEntry == 1 )
    {
        cache_ds.NUM_READ_MISS += 1;

        if( cache_ds.nextLevel!=NULL )
        {
            readFromAddress(*cache_ds.nextLevel, address, 0);
        }

        cache_ds.NUM_ACCESS += 1;		//increase the memory traffic counter
        cache_ds.TAGS[TAG_LOC] = TAG_ADD;
        cache_ds.DIRTY[TAG_LOC] = 0;
        LRUForMiss(cache_ds, INDEX, &TMP_LOC);
        cache_ds.LRUC[TAG_LOC] = 0;
        return;
    }



    if( vc_size != 0 )
    {

        LRUForMiss(cache_ds, INDEX, &TAG_LOC);
        cache_ds.LRUC[TAG_LOC] = 0;

        readFromVictim(cache_ds, address, TAG_LOC, 'r');

        return;
    }

    cache_ds.NUM_READ_MISS += 1;

    cache_ds.NUM_ACCESS += 1;		//increase the memory traffic counter
    LRUForMiss(cache_ds, INDEX, &TAG_LOC);
    cache_ds.LRUC[TAG_LOC] = 0;


    if( (int)cache_ds.DIRTY[TAG_LOC] == 1 )
    {

        cache_ds.NUM_ACCESS += 1;
        if( cache_ds.nextLevel!=NULL )
        {
            TMP_ADD = cache_ds.TAGS[TAG_LOC];
            TMP_ADD = ( ( (TMP_ADD<<noOfIndexBits)|(TAG_LOC%cache_ds.SET) )<<noOfBlockBits );
            writeToAddress(*cache_ds.nextLevel, TMP_ADD, 0);
        }
        cache_ds.NUM_WRITEBACK += 1;
        cache_ds.DIRTY[TAG_LOC] = 0;
    }


    if( cache_ds.nextLevel!=NULL )
    {
        readFromAddress(*cache_ds.nextLevel, address, 0);
    }

    cache_ds.TAGS[TAG_LOC] = TAG_ADD;

    return;
}




void CACHE::readFromVictim(Cache &cache_ds, unsigned int address, unsigned int tagLocationCache, char rw)
{
    int noOfBlockBits = 0, noOfIndexBitsCache = 0, i=0;
    unsigned int TAG_ADD = 0, INDEX = 0;// TAG_LOC = 0;
    unsigned int tagAddressOfCache, TMP_TAG, constructedAddress = 0;

    noOfBlockBits = (int)log2(BLOCKSIZE);
    TAG_ADD = address>>noOfBlockBits;

    noOfIndexBitsCache = (int)log2(cache_ds.SET);

    extractAddressParams(address, cache_ds, &INDEX, &tagAddressOfCache);

    for( i=0; i< (int)Victim.ASSOC; i++)
    {
        if( Victim.TAGS[i] == TAG_ADD )	//Checking Tag Entries
        {
            if( Victim.VALID[i] != 0 )
            {

                TMP_TAG = cache_ds.TAGS[tagLocationCache];
                cache_ds.TAGS[tagLocationCache] = tagAddressOfCache;
                Victim.TAGS[i] = ( (TMP_TAG<<noOfIndexBitsCache)|(INDEX%cache_ds.SET) );

                TMP_TAG =  cache_ds.DIRTY[tagLocationCache];
                cache_ds.DIRTY[tagLocationCache] = Victim.DIRTY[i];
                Victim.DIRTY[i] = TMP_TAG;
                LRUForHitVC(i);
                Victim.NUM_SWAP ++;
                if( rw == 'w' )
                    cache_ds.DIRTY[tagLocationCache] = 1;
                return;
            }
        }
    }

    //It's a Cache Miss
    if( rw == 'r' )
        cache_ds.NUM_READ_MISS ++;
    else
        cache_ds.NUM_WRITE_MISS ++;

    cache_ds.NUM_ACCESS ++;


    LRUForMissVC();

    if( (int)Victim.DIRTY[Victim.TAG_LOC] == 1)
    {
        //printf("\nVICTIM CACHE WRITE BACK");

        if( cache_ds.nextLevel!=NULL )
        {
            constructedAddress = Victim.TAGS[Victim.TAG_LOC]<<noOfBlockBits;
            writeToAddress(*cache_ds.nextLevel, constructedAddress, 0);

        }
        Victim.NUM_WRITEBACK ++;
        cache_ds.NUM_ACCESS += 1;
    }


    if( cache_ds.nextLevel!=NULL )
    {
        readFromAddress(*cache_ds.nextLevel, address, 0);
    }

    TMP_TAG = cache_ds.TAGS[tagLocationCache];
    Victim.TAGS[Victim.TAG_LOC] = ( (TMP_TAG<<noOfIndexBitsCache)|(INDEX%cache_ds.SET) );

    TMP_TAG =  cache_ds.DIRTY[tagLocationCache];
    Victim.DIRTY[Victim.TAG_LOC] = TMP_TAG;

    Victim.VALID[Victim.TAG_LOC] = 1;
    cache_ds.TAGS[tagLocationCache] = tagAddressOfCache;
    cache_ds.DIRTY[tagLocationCache] = rw-'r'?1:0;
}





//Recursive solution

void CACHE::writeToAddress(Cache &cache_ds, unsigned int address, unsigned int vc_size)
{

    int i=0, foundInvalidEntry = 0;
    unsigned int TAG_LOC = 0, INDEX = 0, TAG_ADD = 0, TMP_ADD = 0, TMP_LOC = 0;

    int noOfBlockBits = 0, noOfIndexBits = 0;

    noOfBlockBits = (int)log2(BLOCKSIZE);
    noOfIndexBits = (int)log2(cache_ds.SET);

    cache_ds.NUM_WRITE += 1;
    extractAddressParams(address, cache_ds, &INDEX, &TAG_ADD);


    for( i=0; i< (int)cache_ds.ASSOC; i++)
    {
        if( cache_ds.TAGS[INDEX + (i*cache_ds.SET)] == TAG_ADD )	//Checking Tag Entries
        {
            if( cache_ds.VALID[INDEX + (i*cache_ds.SET)] != 0 )
            {

                cache_ds.DIRTY[INDEX + (i*cache_ds.SET)] = 1;
                LRUForHit(cache_ds, INDEX, ( INDEX + (i*cache_ds.SET) ) );

                return;
            }
        }
    }


    for( i=0; i< (int)cache_ds.ASSOC; i++)
    {
        if( cache_ds.VALID[INDEX + (i*cache_ds.SET)] == 0 )
        {
            cache_ds.VALID[INDEX + (i*cache_ds.SET)] = 1;
            TAG_LOC =  INDEX + (i*cache_ds.SET);
            foundInvalidEntry = 1;
            break;
        }
    }


    if( foundInvalidEntry == 1 )
    {
        cache_ds.NUM_WRITE_MISS += 1;

        if( cache_ds.nextLevel!=NULL )
        {
            readFromAddress(*cache_ds.nextLevel, address, 0);
        }
        cache_ds.NUM_ACCESS += 1;		//increase the memory traffic counter
        cache_ds.TAGS[TAG_LOC] = TAG_ADD;
        cache_ds.DIRTY[TAG_LOC] = 1;

        LRUForMiss(cache_ds, INDEX, &TMP_LOC);
        cache_ds.LRUC[TAG_LOC] = 0;
        return;
    }


    if( vc_size != 0 )
    {
        LRUForMiss(cache_ds, INDEX, &TAG_LOC);
        cache_ds.LRUC[TAG_LOC] = 0;

        readFromVictim(cache_ds, address, TAG_LOC, 'w');

        return;
    }

    cache_ds.NUM_WRITE_MISS += 1;

    cache_ds.NUM_ACCESS += 1;		//increase the memory traffic counter
    LRUForMiss(cache_ds, INDEX, &TAG_LOC);
    cache_ds.LRUC[TAG_LOC] = 0;

    if( (int)cache_ds.DIRTY[TAG_LOC] == 1 )
    {
        cache_ds.NUM_ACCESS += 1;
        if( cache_ds.nextLevel!=NULL )
        {
            TMP_ADD = cache_ds.TAGS[TAG_LOC];
            TMP_ADD = ( ( (TMP_ADD<<noOfIndexBits)|(TAG_LOC%cache_ds.SET) )<<noOfBlockBits );
            writeToAddress(*cache_ds.nextLevel, TMP_ADD, 0);
        }
        cache_ds.NUM_WRITEBACK += 1;

    }


    if( cache_ds.nextLevel!=NULL )
    {
        readFromAddress(*cache_ds.nextLevel, address, 0);
    }

    cache_ds.DIRTY[TAG_LOC] = 1;
    cache_ds.TAGS[TAG_LOC] = TAG_ADD;


    return;
}




void CACHE::LRUForHit(Cache &l1Cache, unsigned int INDEX, unsigned int TAG_LOC)
{
    int i = 0;

    for( i=0; i< (int)l1Cache.ASSOC; i++)
    {
        if( l1Cache.LRUC[INDEX + (i*l1Cache.SET)] < l1Cache.LRUC[TAG_LOC] )
        {
            l1Cache.LRUC[INDEX + (i*l1Cache.SET)] = (l1Cache.LRUC[INDEX + (i*l1Cache.SET)]) + 1;
        }
    }

    l1Cache.LRUC[TAG_LOC] = 0;
}



void CACHE::LRUForMiss(Cache &l1Cache, unsigned int INDEX, unsigned int* TAG_LOC)
{
    unsigned int i = 0;
    int max = 0;
    *TAG_LOC = 0;
    //printf("\nL1 UPDATE LRU");
    for( i=0; i<l1Cache.ASSOC; i++)
    {
        if( l1Cache.LRUC[INDEX + (i*l1Cache.SET)] > max )
        {
            max = l1Cache.LRUC[INDEX + (i*l1Cache.SET)];
            *TAG_LOC = ( INDEX + (i*l1Cache.SET) );
        }
    }


    for( i=0; i<l1Cache.ASSOC; i++)
    {
        l1Cache.LRUC[INDEX + (i*l1Cache.SET)] = (l1Cache.LRUC[INDEX + (i*l1Cache.SET)]) + 1;
    }
}




void CACHE::LRUForHitVC(int index) {
    for(unsigned int i=0; i<Victim.ASSOC; i++)
        if( Victim.LRUC[i] < Victim.LRUC[index] )
            Victim.LRUC[i] = (Victim.LRUC[i] + 1);
    Victim.LRUC[index] = 0;
}

void CACHE::LRUForMissVC() {
    unsigned int max = 0, i;
    Victim.TAG_LOC = 0;
    for(i=0; i<Victim.ASSOC; i++)
        if( Victim.LRUC[i] > max ) {
            max = Victim.LRUC[i];
            Victim.TAG_LOC = i;
        }
    for(i=0; i<Victim.ASSOC; i++)
        Victim.LRUC[i] = (Victim.LRUC[i] + 1);
    Victim.LRUC[Victim.TAG_LOC] = 0;
}