#ifndef BUCKET_H_
#define BUCKET_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "type.h"
#include "elib.h"


void initBkt(SUFFIX **bkt, int bktSize);
int entryIndex(char *kmer, int k);
void slideWindow(char *str, int strLen, int sid, SUFFIX **bkt, int bktSize, SUFFIX *sf, int k);
void buildBkt(int rank, SEQ *seqs, int nseqs, int sLine, SUFFIX **bkt, int bktSize, SUFFIX *sf, int sfSize, int k);
void suf2tuple(int rank, SUFFIX **bkt, int bktSize, STP *stp, int stpSize, int *stpCnt);
int redistBkt(int rank, int procs, int *allCnt, int cntSize);


int printBktList(SUFFIX *bktList);

#endif /* end of bucket.h */
