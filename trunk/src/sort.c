/*
 * $Rev$ 
 * $Date$ 
 * $Author$
 *
 * Copyright 2010 Washington State University. All rights reserved.
 * ----------------------------------------------------------------
 *
 */

#include "sort.h"

/* -----------------------------------------------------------*
 * counting sort based on the depth of the stNodes[i].
 * 
 * @param stNodes - stree nodes
 * @param srtIndex - sorted index of stnodes w. depth
 * @param nStNodes - #(stnodes) in stree
 * @param maxDepth - maxium depth of stree nodes
 * -----------------------------------------------------------*/
void countSort(STNODE *stNodes, int *srtIndex, int nStNodes, int maxDepth){
    int i;
    int *cnt = NULL;
    int depth;

    cnt = emalloc(maxDepth*(sizeof *cnt));

    /* init counters */
    for(i = 0; i < maxDepth; i++){
        cnt[i] = 0;
    }   

    /* fill in counters */
    for(i = 0; i < nStNodes; i++){
        depth = stNodes[i].depth; /* depth starts from 0 */
        assert(depth < maxDepth);
        cnt[depth]++;
    }   
    
    /* suffix sum to sort in a reverse way */
    for(i = maxDepth-2; i >= 0; i--){
       cnt[i] += cnt[i+1]; 
    }   
    
    /* store the sorted index into srtIndex */
    for(i = 0; i < nStNodes; i++){
        depth = stNodes[i].depth;
        srtIndex[cnt[depth]-1] = i;
        cnt[depth]--;
    }   

    free(cnt);
}

