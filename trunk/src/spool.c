/*
 * $Rev$ 
 * $Date$ 
 * $Author$
 *
 * Copyright 2010 Washington State University. All rights reserved.
 * ----------------------------------------------------------------
 *
 */

#include "spool.h"

void initSPool(SPOOL *sPool, int size, int maxStrSize){
    int i;
    STR *st = NULL;

    sPool->pool = emalloc(size*(sizeof *(sPool->pool)));
    sPool->space = size;

    for(i = 0; i < size; i++){
        st = sPool->pool + i;
        st->str = emalloc(maxStrSize*(sizeof *(st->str)));
        st->stat = STR_FREE;
    }

}


void freeSPool(SPOOL *sPool, int size){
    int i;

    for(i = 0; i < size; i++){
        free((sPool->pool)[i].str);
    }

    free(sPool->pool);
}


int deSPool(SPOOL *sPool, int size){
    int i; 
    STR *st = NULL;

    if(sPool->space > 0){
        for(i = 0; i < size; i++){
            st = sPool->pool + i;
            if(st->stat == STR_FREE){
                st->stat = STR_TOGO;
                (sPool->space)--;
                
                return i;
            }
        }
    }

    return -1;
}

