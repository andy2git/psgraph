#ifndef SR_POOL_H_
#define SR_POOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "type.h"
#include "elib.h"

void initRPool(RPOOL *rPool, int size, int maxIdsLen);
void freeRPool(RPOOL *rPool, int size);
int deRPool(RPOOL *rPool, int size);

#endif /* end of srpool.h */
