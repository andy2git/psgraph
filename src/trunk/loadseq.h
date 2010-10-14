#ifndef LOAD_SEQ_H_
#define LOAD_SEQ_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include "type.h"
#include "elib.h"
#include "lib.h"


int loadStaticSeqs(int rank, int sRank, char *seqFile, SEQ *seqs, int nSeqs, int maxSeqRange, int *maxSeqLen);
void freeSeqs(SEQ *seqs, int nSeqs, int tag);

#endif /* enf of loadseq.h */
