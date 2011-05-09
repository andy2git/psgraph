#ifndef SR_POOL_H_
#define SR_POOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "type.h"
#include "elib.h"


/* seq. req. stat */
#define SREQ_FREE 0
#define SREQ_TOGO 1
#define SREQ_SENT 2


/**
 * pool of seq. reqs resources
 */
typedef struct sreq{
    /* #define SREQ_FREE 0
       #define SREQ_TOGO 1
       #define SREQ_SENT 2
     */
    char stat;        /* if this *ids (res. used for sending req.) is used or not? */
    int *ids;         /* list of requested seq ids */
    int cnt;          /* #(requested ids) */
    MPI_Request req;  /* associated with each *ids */
}SREQ;  

typedef struct rpool{
    int space;
    struct sreq *pool;
}RPOOL;

void initRPool(RPOOL *rPool, int size, int maxIdsLen);
void freeRPool(RPOOL *rPool, int size);
int deRPool(RPOOL *rPool, int size);

#endif /* end of srpool.h */
