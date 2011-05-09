#ifndef pBufFER_H_
#define pBufFER_H_

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#include "type.h"
#include "elib.h"
#include "union.h"

void initBuf(PBUF *pBuf, int bufSize);
int enBuf(PBUF *pBuf, int bufSize, MSG *msg, int msgSize);
int deBuf(PBUF *pBuf, int bufSize, MSG *msg, int msgSize, int *isEnd);
void freeBuf(PBUF *pBuf);

void printBuf(PBUF *pBuf, int bufSize);

#endif /* end of pBuf.h */
