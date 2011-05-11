/*
 * $Rev$ 
 * $Date$ 
 * $Author$
 *
 * Copyright 2010 Washington State University. All rights reserved.
 * ----------------------------------------------------------------
 *
 */

#ifndef CPOOL_H_
#define CPOOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "type.h"
#include "elib.h"


/* cs stat */
#define CS_FREE 0
#define CS_TOGO 1
#define CS_SENT 2

/**
 * Pool for consumer status report to master
 */
typedef struct csreq{
    char stat;
    MSG msg;
    MPI_Request req;
}CSREQ;

typedef struct cpool{
    int space;
    struct csreq *pool;
}CPOOL;

void initCPool(CPOOL *cPool, int size);
void freeCPool(CPOOL *cPool);
int deCPool(CPOOL *cPool, int size);

#endif /* end of cpool.h */
