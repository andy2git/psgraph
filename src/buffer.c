/*
 * $Rev$ 
 * $Date$ 
 * $Author$
 *
 * Copyright 2010 Washington State University. All rights reserved.
 * ----------------------------------------------------------------
 *
 */

#include "buffer.h"

/* ----------------------------------------------*
 * init circular pair buffer
 *
 * @param pBuf - pair buffer
 * @param bufSize - pair buffer size
 *
 * ----------------------------------------------*/
void initBuf(PBUF *pBuf, int bufSize){

    pBuf->buf = ecalloc(bufSize, sizeof *(pBuf->buf));
    pBuf->head = 0;
    pBuf->tail = 0;
    pBuf->data = 0;
}


/* ----------------------------------------------*
 * put msg[msgSize] into pair buffer
 *
 * @param pBuf - pair buffer
 * @param bufSize - pair buffer size
 * @param msg - pairs to be put in
 * @param msgSize - #(pairs)
 *
 * ----------------------------------------------*/
int enBuf(PBUF *pBuf, int bufSize, MSG *msg, int msgSize){
    int i = 0;

    if((msgSize+pBuf->data) > bufSize){
        fprintf(stderr, "msgSize=%d, pBuf->data=%d > bufSize=%d\n", msgSize, pBuf->data, bufSize);
        exit(0);
    }
    
    while(i < msgSize){
        /* struct copy */
        pBuf->buf[pBuf->head].tag = msg[i].tag;
        pBuf->buf[pBuf->head].id1 = msg[i].id1; 
        pBuf->buf[pBuf->head].id2 = msg[i].id2;

        pBuf->head++;
        pBuf->head %= bufSize;
        pBuf->data++;
        i++;
    }

    return i;
}


/* --------------------------------------------------------------------------*
 * take msgSize pairs out from pair buffer
 * 
 * @param pBuf - pair buffer
 * @param bufSize - pair buffer size
 * @param msg - pointer to store the pairs
 * @param msgSize - #(pairs) expected to take out
 * @param uf - union find used as filter
 * @param isEnd - does the returned batch hit the ending signal(when producer
 *                is done with its pair generation, it will send ending signal 
 *                to master indicating its ending ?
 *
 * NOTE: pass NULL in for *uf, *isEnd.
 *
 * ENDING MSG WILL NOT BE POPED OUT!!!!!!!!!!!!!!!
 * 
 * ---------------------------------------------------------------------------*/ 
int deBuf(PBUF *pBuf, int bufSize, MSG *msg, int msgSize, int *isEnd){
    int i = 0;
    
    while(i < msgSize && pBuf->data > 0){
        if(pBuf->buf[pBuf->tail].tag == TAG_E){
            pBuf->tail++;
            pBuf->tail %= bufSize;
            pBuf->data--;
            *isEnd = 1;
            return i;
        }

        msg[i].tag = pBuf->buf[pBuf->tail].tag;
        msg[i].id1 = pBuf->buf[pBuf->tail].id1;
        msg[i].id2 = pBuf->buf[pBuf->tail].id2;

        pBuf->tail++;
        pBuf->tail %= bufSize;
        pBuf->data--;
        i++;
    }

    if(isEnd) *isEnd = 0;
    
    return i;
}


/* ----------------------------------------------*
 * print out pair buffer
 *
 * @param pBuf - pair buffer
 * @param bufSize - pair buffer size
 *
 * ----------------------------------------------*/
void printBuf(PBUF *pBuf, int bufSize){
    int i;
    int j;

    printf("head=%d, tail=%d, data=%d\n", pBuf->head, pBuf->tail, pBuf->data);

    j = pBuf->tail;
    for(i = 0; i < pBuf->data; i++){
        printf("[%c,%d,%d]", pBuf->buf[j].tag, pBuf->buf[j].id1, pBuf->buf[j].id2);
        j++;
        j %= bufSize;
    }
}


/* ----------------------------------------------*
 * free memory allocated for pair buffer
 * 
 * @param pBuf - pair buffer
 * 
 * ----------------------------------------------*/
void freeBuf(PBUF *pBuf){
    free(pBuf->buf);
}

