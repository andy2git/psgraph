/*
 * $Rev$ 
 * $Date$ 
 * $Author$
 *
 * Copyright 2010 Washington State University. All rights reserved.
 * ----------------------------------------------------------------
 *
 */

#include "rpool.h"

/* ------------------------------------------------- *
 * 
 * @param size - maximum send req. pool size
 * @param maxIdsLen - maximum n/p seq. ids 
 * ------------------------------------------------- */
void initRPool(RPOOL *rPool, int size, int maxIdsLen){
    int i;
    SREQ *sreq = NULL; 

    rPool->pool = emalloc(size*(sizeof *(rPool->pool)));
    rPool->space = size;
    
    for(i = 0; i < size; i++){
        sreq = rPool->pool + i;
        sreq->ids = emalloc(maxIdsLen*(sizeof *(sreq->ids)));
        sreq->stat = SREQ_FREE;
        sreq->cnt = 0;
    }
}


void freeRPool(RPOOL *rPool, int size){
    int i;

    for(i = 0; i < size; i++){
        free((rPool->pool)[i].ids);
    }

    free(rPool->pool);
}


/* ------------------------------------------------- *
 * return - valid index if it has any left res. 
 *          -1 if nothing left in rPool.
 *          ------------------------------------
 *          the returned res. is initialized, too
 *
 * @param 
 * ------------------------------------------------- */
int deRPool(RPOOL *rPool, int size){
    int i;
    SREQ *sreq = NULL;

    if(rPool->space > 0){
        for(i = 0; i < size; i++){
            sreq = rPool->pool + i;
            if(sreq->stat == SREQ_FREE){
                sreq->stat = SREQ_TOGO;
                sreq->cnt = 0;
                (rPool->space)--;
                return i;
            }
        }
    }

    return -1;
}

