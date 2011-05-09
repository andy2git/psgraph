#ifndef LOAD_SEQS_H_
#define LOAD_SEQS_H_

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include "type.h"
#include "elib.h"

#define FASTA_MAX_LEN   100000
#define FASTA_HEAD_FLAG '>'

int loadSeqs(int rank, SEQ **seqs, int nseqs, int procs, char *fastaFile, ulong *AA, int *asLine);
void freeSeqs(SEQ **seqs, size_t size);
void printSeq(SEQ *seqs, int index);

#endif /* end of loadseq.h */
