#ifndef CSM_QUE_H_
#define CSM_QUE_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "mpi.h"
#include "type.h"

void initQue(CSM *ms, CSM **mIn, int nGroup, int gSize, int procs);
void assignQue(CSM **mIn, int gSize, int nGroup, int retired, MPI_Datatype msgMdt);
void printQue(CSM **mIn, int nGroup);

#endif /* end of csmqueue.h */
