#ifndef PAIRS_H_
#define PAIRS_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "mpi.h"
#include "timer.h"
#include "type.h"
#include "buffer.h"

void procLeaf(int master, SUFFIX **lset, PBUF *pBuf, int pBufSize, MSG *chunk, int chunkSize, int *isStart, u64 *nPairs, double *iTime,
            MPI_Datatype msgMdt, MPI_Request *request, MPI_Comm *comm);

u64 genPairs(int master, STNODE *stNodes, int *srtIndex, int nStNodes, int nSeqs, int EM, int *dup, PBUF *pBuf, int pBufSize, 
            MSG *chunk, int chunkSize, int *isStart, double *iTime, MPI_Datatype msgMdt, MPI_Request *request, MPI_Comm *comm);

#endif /* end of pairs.h */
