/*
 * $Rev: 776 $ 
 * $Date: 2010-09-01 10:40:43 -0700 (Wed, 01 Sep 2010) $ 
 * $Author$
 *
 * Copyright 2010 Washington State University. All rights reserved.
 * ----------------------------------------------------------------
 *
 */

#include "queue.h"

/* ---------------------------------------------*
 * Init consumer request queue cReq[]. All memory
 * for queue are statically allocated in a pool
 * mannel.
 *
 * @param cReq - consumer request queue
 * @param cReqSize - cosumer request queue size
 * @param cmStRank - consumer start rank
 * @param procs - #(consumers + producers)
 *
 * ---------------------------------------------*/
void initReq(CREQ *cReq, int cReqSize, int sRank, int eRank, int procs){
    int i;

    /* statically allocate maxium memory */
    for(i = 0; i < cReqSize; i++){
        cReq[i].stMem = ecalloc(procs, sizeof *(cReq[i].stMem));
        cReq[i].queue = NULL;
    }

    assert(cReqSize > 1);

    /* initialize all consumers in cReq[0] */
    for(i = sRank; i < eRank; i++){
        cReq[1].stMem[i].rank = i;
        cReq[1].stMem[i].next = &(cReq[1].stMem[i+1]);
    }

    /* head and tail cases */
    cReq[1].queue = &(cReq[1].stMem[sRank]);
    cReq[1].stMem[eRank].rank = eRank;
    cReq[1].stMem[eRank].next = NULL;
}


/* ----------------------------------------------*
 * Release the pre-allocated static memory
 * 
 * @param cReq - consumer request queue
 * @param cReqSize - consumer request queue size
 *
 * ----------------------------------------------*/
void relseReq(CREQ *cReq, int cReqSize){
    int i;

    for(i = 0; i < cReqSize; i++){
        free(cReq[i].stMem);
    }
}


/* ----------------------------------------------*
 * Print out consumer request queue
 *
 * @param cReq - consumer request queue
 * @param cReqSize - consumer request queue size
 * 
 * ----------------------------------------------*/
void printReq(CREQ *cReq, int cReqSize){
    int i;
    REQ *p = NULL;

    for(i = 0; i < cReqSize; i++){
        printf("Req[%d]=>", i);

        for(p = cReq[i].queue; p != NULL; p = p->next){
            printf("<%d>-", p->rank);
        }

        printf("NULL\n");
    }
}


/* -----------------------------------------------*
 * Delete old request if it exists in other queue,
 * Then put the new request in queue.
 * 
 * @param cReq -
 * @param cReqSize - 
 * @param msg -
 *
 * -----------------------------------------------*/
void enReq(CREQ *cReq, int cReqSize, MSG *msg){
    int prio;
    int rank;
    REQ *p = NULL;
    
    /* remove it from other queue if it exists */
    rmReq(cReq, cReqSize, msg);

    rank = msg->id1;
    prio = msg->id2;

    if(cReq[prio].queue == NULL){
        cReq[prio].stMem[rank].rank = rank;
        cReq[prio].queue = &(cReq[prio].stMem[rank]);
        cReq[prio].stMem[rank].next = NULL;
    }else{
        p = cReq[prio].queue;
        while(p->next) p = p->next;
        cReq[prio].stMem[rank].rank = rank;
        cReq[prio].stMem[rank].next = p->next;
        p->next = &(cReq[prio].stMem[rank]);
    }
}

/* -----------------------------------------------*
 * remove request *msg from cReq if it exists
 * already. Requests in cReq could be delayed, so
 * when new request comes in, the old needs to be
 * removed.
 *
 * NOTE: nothing will happen if the request *msg
 * does not exist at all.
 * 
 * @param cReq - consumer request queue
 * @param cReqSize - consumer request queue size
 * @param msg - request to be removed
 *
 *------------------------------------------------*/
void rmReq(CREQ *cReq, int cReqSize, MSG *msg){
    int rank;
    int i;
    REQ *p = NULL;

    if(msg == NULL) return;
    rank = msg->id1;

    for(i = 0; i < cReqSize; i++){
        if(cReq[i].queue){
            if(cReq[i].queue->rank == rank){
                cReq[i].queue = (cReq[i].queue)->next;
                return;
            }

            for(p = cReq[i].queue; p->next != NULL; p = p->next){
                if(p->next && p->next->rank == rank){
                    p->next = p->next->next; 
                    return;
                }
            }
        }
    }
}

/* ---------------------------------------------*
 * Return the first Request from the first queue
 * 
 * @param cReq - consumer request queue
 * @param cReqSize -  cReq size
 *
 * @return the first consumer reqeust
 * ---------------------------------------------*/ 
REQ *deReq(CREQ *cReq, int cReqSize, int *prio){
    int i;
    REQ *p = NULL;
    
    for(i = 0; i < cReqSize; i++){
        if(cReq[i].queue){
            p = cReq[i].queue;
            cReq[i].queue = (cReq[i].queue)->next;
            *prio = i;
            return p;
        }
    }

    return NULL;
}

