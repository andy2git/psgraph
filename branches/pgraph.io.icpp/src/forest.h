#ifndef STREE_H_
#define STREE_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "timer.h"
#include "type.h"
#include "elib.h"
#include "cfg.h"
#include "sort.h"
#include "pairs.h"

#define STREE_MAX_LEN 100000


int loadForest(FILE *fp, FOREST *forest, int frSize, int sIndex, int *tIndex);
void processForest(int groupID, int master, char *frFile, char *cfgFile, int nSeqs, PBUF *pBuf, int pBufSize, int *isStart, 
                        MSG *chunk, int chunkSize, double *iTime, MPI_Request *request, MPI_Datatype msgMdt, MPI_Comm *comm);
void freeForest(FOREST *forest, int frSize);
void printStnodes(STNODE *stNodes, int stIndex, int blSize);

#endif /* end of forest.h */
