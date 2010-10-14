#ifndef CS_LIB_H_
#define CS_LIB_H_

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <mpi.h>
#include "type.h"
#include "elib.h"
#include "rpool.h"
#include "spool.h"
#include "cpool.h"

int maxRangeSum(SEQ *seqs, int size, int range);

void prepSeqReq(int sRank, MSG *msg, int msgSize, SEQ *seqs, int nSeqs, int *map, int mapSize, 
                RPOOL *rPool, int rPoolSize, int maxSeqRange, u64 *nFetIds, u64 *nStat, u64 *nSave);
void enSeqReq(int sRank, RPOOL *rPool, int rPoolSize, int sid, int maxSeqRange, int *map, int mapSize);
void sendSeqReq(int rank, int sRank, RPOOL *rPool, int rPoolSize, int *map, int mapSize, MPI_Comm *comm);
void freeSeqReq(RPOOL *rPool, int rPoolSize);


void prepStr(int rank, int *rvBuf, int rvBufSize, SEQ *seqs, int nSeqs, int tRank,
                SPOOL *sPool, int sPoolSize, int maxStrSize, MPI_Comm *comm);
void freeStr(SPOOL *sPool, int sPoolSize);
void upackStr(char *rvStr, int rvStrSize, SEQ *seqs, int nSeqs, char *upBuf, u64 *nByte, MPI_Comm *comm);
void sendStat(int rank, int master, int stat, CPOOL *cPool, int cPoolSize, MPI_Datatype msgMdt, MPI_Comm *comm);
void releaseCSReq(CPOOL *cPool, int cPoolSize);
#endif /* end of cslib.h */
