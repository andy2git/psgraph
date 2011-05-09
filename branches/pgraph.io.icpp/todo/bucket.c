#include "bucket.h"

static int sfIndex = 0;

/* -----------------------------------------* 
 * Bkt here is essentially counters. Init 
 * them into zero.
 *
 * @param bkt - headers for *suff
 * @param bktSize - bucket size
 * -----------------------------------------*/

void initBkt(SUFFIX **bkt, int bktSize){
    int i;
    for(i = 0; i < bktSize; i++){
        bkt[i] = NULL; 
    }
}


/* -----------------------------------------* 
 * This function can be optimized if higher
 * performance is required.
 *
 * @param kmer - address of of k-mer string
 * @param k - slide window size
 * -----------------------------------------*/

int entryIndex(char *kmer, int k){
    int i;
    int value = 0;

    for(i = 0; i < k; i++){
        value = value*SIGMA + (kmer[i] - 'A');
    }
    
    return value;
}


/* ----------------------------------------------*
 * Bucketing for str <sid>.
 *
 * @param str - seqence itself
 * @param strlen - including ending '$'
 * @param sid - string id. eg. 0, 1, 2, ..., N-1
 * @param bkt - headers for each bucket
 * @param bktSize - bucket size
 * @param sf - mem allocated for all buckets
 * @param k - slide window size
 * ----------------------------------------------*/

void slideWindow(char *str, int strLen, int sid, SUFFIX **bkt, int bktSize, SUFFIX *sf, int k){
    int i;
    int stopIndex = strLen - k - 1;
    int bktIndex;
   
    for(i = 0; i <= stopIndex; i++){
        bktIndex = entryIndex(str+i, k);

        /* prefixed in the bucket list */
        sf[sfIndex].sid = sid;
        sf[sfIndex].pid = i;
        sf[sfIndex].next = bkt[bktIndex];
        bkt[bktIndex] = &sf[sfIndex];

        sfIndex++;
    }
}

/* ----------------------------------------------*
 * This function is to bucket all seqs in buckets
 *
 * @param seqs - fasta seqs
 * @param nseqs - #(fasta seqs)
 * @param bkt - 
 * @param bktSize - bucket size, which is <SIGMA^k>
 * @param suff - 
 * @param suffSize -
 * @param k - slide window size
 * ----------------------------------------------*/

void buildBkt(int rank, SEQ *seqs, int nseqs, int sLine, SUFFIX **bkt, int bktSize, SUFFIX *sf, int sfSize, int k){
    int i;
    initBkt(bkt, bktSize);
    
    /* slide k-mers for every seqs, and bucket them */
    for(i = 0; i < nseqs; i++){
        slideWindow(seqs[i].str, seqs[i].strLen, i+sLine, bkt, bktSize, sf, k);
    }

    printf("proc[%d]: sfIndex=%d\n", rank, sfIndex);
    assert(sfIndex == sfSize);
}



void suf2tuple(int rank, SUFFIX **bkt, int bktSize, STP *stp, int stpSize, int *stpCnt){
    int i;
    int stpInd = 0;
    SUFFIX *p = NULL;

    for(i = 0; i < bktSize; i++){
        stpCnt[i] = 0;

        if(bkt[i]){
            for(p = bkt[i]; p != NULL; p = p->next){        
                stp[stpInd].sid = p->sid;
                stp[stpInd++].pid = p->pid;
                stpCnt[i]++;
            }
        }
    }

    assert(stpInd = stpSize);
}


int redistBkt(int rank, int procs, int *allCnt, int cntSize){
    int i;
    int segment;
    int ntpSize = 0;
    int sSeg, eSeg;

    segment = (cntSize%procs == 0) ? cntSize/procs : (cntSize/procs + 1);

    sSeg = rank*segment;
    eSeg = (rank+1)*segment - 1;

    if(eSeg >= cntSize) eSeg = cntSize - 1;
    
    for(i = sSeg; i <= eSeg; i++){
        ntpSize += allCnt[i];
    }

    return ntpSize;
}


/* ----------------------------------------------*
 * Print a linked list.
 *
 * @param bkt - bucket pointers
 * @param bIndex - index for each bucket
 * ----------------------------------------------*/
int printBktList(SUFFIX *bktList){
    SUFFIX *p = NULL;
    int i = 0;

    printf("->");
    for(p = bktList; p != NULL; p=p->next){
        printf("[%d, %d]\t", p->sid, p->pid);
        i++;
    }
    printf("\n");
    return i;
}
