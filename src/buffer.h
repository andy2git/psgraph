/*
 * $Rev$ 
 * $Date$ 
 * $Author$
 *
 * Copyright 2010 Washington State University. All rights reserved.
 * ----------------------------------------------------------------
 *
 */

#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdio.h>
#include <stdlib.h>

#include "type.h"
#include "elib.h"

/* pair buffer to hold pairs in supermaster, producer, master, and consumer.
   It is a circular buffer. */
typedef struct pbuf{
    int head;
    int tail;
    int data;
    struct msg *buf;
}PBUF;


void initBuf(PBUF *pBuf, int bufSize);
int enBuf(PBUF *pBuf, int bufSize, MSG *msg, int msgSize);
int deBuf(PBUF *pBuf, int bufSize, MSG *msg, int msgSize, int *isEnd);
void freeBuf(PBUF *pBuf);

void printBuf(PBUF *pBuf, int bufSize);

#endif /* end of buffer.h */
