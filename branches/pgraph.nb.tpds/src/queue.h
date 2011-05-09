#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdio.h>
#include <assert.h>
#include "type.h"
#include "elib.h"

typedef struct req{
    int rank;
    struct req *next;
}REQ;

typedef struct cReq{
    struct req *queue;
    struct req *stMem; /* pre-allocated static mem */
}CREQ;

void initReq(CREQ *cReq, int cReqSize, int sRank, int eRank, int procs);
void relseReq(CREQ *cReq, int cReqSize);
void printReq(CREQ *cReq, int cReqSize);
void enReq(CREQ *cReq, int cReqSize, MSG *msg);
void rmReq(CREQ *cReq, int cReqSize, MSG *msg);
REQ *deReq(CREQ *cReq, int cReqSize, int *prio);

#endif /* end of queue.h */
