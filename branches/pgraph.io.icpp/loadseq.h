#ifndef LOAD_SEQ_H_
#define LOAD_SEQ_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include "type.h"
#include "elib.h"

#define MAX_FASTA_LINE_LEN 100000
#define FASTA_FLAG '>'

int cacheSeqs(char *seqFile, SEQ *seqs, int nSeqs, int *idList, int seqSize, int msl);
void freeSeqs(SEQ *seqs, int *ids, int nSeqs);

void randIds(int *ids, int idSize, int idCnt);
int filterIds(int *cIds, int *dIds,int nSeqs, PBUF *pBuf, int pBufSize, int batch);

#endif /* enf of loadseq.h */
