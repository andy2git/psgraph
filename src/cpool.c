/*
 * $Rev: 793 $ 
 * $Date: 2010-09-02 22:08:52 -0700 (Thu, 02 Sep 2010) $ 
 * $Author$
 *
 * Copyright 2010 Washington State University. All rights reserved.
 * ----------------------------------------------------------------
 *
 */

#include "cpool.h"

void initCPool(CPOOL *cPool, int size){
    int i;
    
    cPool->pool = emalloc(size*(sizeof *(cPool->pool)));
    cPool->space = size;

    for(i = 0; i < size; i++){
        (cPool->pool)[i].stat = CS_FREE;
    }

}

void freeCPool(CPOOL *cPool){
    free(cPool->pool);
}

int deCPool(CPOOL *cPool, int size){
    int i;
    CSREQ *csreq = NULL;

    if(cPool->space > 0){
        for(i = 0; i < size; i++){
            csreq = cPool->pool + i;
            if(csreq->stat == CS_FREE){
                csreq->stat = CS_TOGO; 
                (cPool->space)--;
                return i;
            }
        }
    }

    return -1;
}
