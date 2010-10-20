/*
 * $Rev: 808 $ 
 * $Date: 2010-09-15 15:37:53 -0700 (Wed, 15 Sep 2010) $ 
 * $Author: Andy $
 *
 * Copyright 2010 Washington State University. All rights reserved.
 * ----------------------------------------------------------------
 * help functions for consumer.c
 */

#include "cslib.h"

/* ----------------------------------------------*
 * find the max. sum of fixed range
 * 
 * @param arr 
 * @param size - array size
 * @param range - fixed slided range
 * ----------------------------------------------*/
int maxRangeSum(SEQ *seqs, int size, int range){
    int sum = 0;
    int max = 0;
    int i;
    
    for(i = 0; i < size; i++){
        if(i%range == 0) {
            if(max < sum) max = sum;
            /* +1 for '\0' */
            sum = seqs[i].strLen + 1; 
        }else{
            /* +1 for '\0' */
            sum += (seqs[i].strLen + 1);
        }
    }

    if (max < sum) max = sum;

    return max;
}

/*################################################*
 *       FOR SENDING SEQ. REQ. USING RPOOL
 *################################################*/

/* ----------------------------------------------*
 * After recv msg[msgSize] pairs, s
 *
 * @param mapSize - groupSize
 * ----------------------------------------------*/
void prepSeqReq(int sRank, MSG *msg, int msgSize, SEQ *seqs, int nSeqs, int *map, int mapSize, 
                RPOOL *rPool, int rPoolSize, int maxSeqRange, u64 *nFetIds, u64 *nStat, u64 *nSave){
    int i;
    int f1, f2;
    
    /* init map */
    for(i = 0; i < mapSize; i++){
        map[i] = -1;
    }
    

    /* iterate over all the pairs */
    for(i = 0; i < msgSize; i++){
        f1 = msg[i].id1;
        switch(seqs[f1].stat){
            case SEQ_N:
                enSeqReq(sRank, rPool, rPoolSize, f1, maxSeqRange, map, mapSize);                  
                seqs[f1].stat = SEQ_R;
                (seqs[f1].cnt)++;
                (*nFetIds)++;
                break; 
            case SEQ_S: /* statically cached */
                (*nStat)++;
                break;
            case SEQ_R: /* seq requested already */
            case SEQ_F: /* seq fetched already */
                (*nSave)++;
                (seqs[f1].cnt)++;
                break;
            default:
                printf("WRONG stat for seqs[%d].stat=%d\n", f1, seqs[f1].stat);
                exit(0);
        }


        f2 = msg[i].id2;
        switch(seqs[f2].stat){
            case SEQ_N:
                enSeqReq(sRank, rPool, rPoolSize, f2, maxSeqRange, map, mapSize);                  
                seqs[f2].stat = SEQ_R;
                (seqs[f2].cnt)++;
                (*nFetIds)++;
                break; 
            case SEQ_S: /* statically cached */
                break;
            case SEQ_R: /* seq requested already */
            case SEQ_F: /* seq fetched already */
                (seqs[f2].cnt)++;
                break;
            default:
                printf("WRONG stat for seqs[%d].stat=%d\n", f2, seqs[f2].stat);
                exit(0);
        }
    }
}


/* -------------------------------------------------- *
 *
 * @param map - map is a index for all possible res.
 * @param mapSize - groupSize
 * -------------------------------------------------- */
void enSeqReq(int sRank, RPOOL *rPool, int rPoolSize, int sid, int maxSeqRange, int *map, int mapSize){
    int tRank; /* target rank */
    int pInd;
    SREQ *sreq = NULL;
    
    tRank = sid/maxSeqRange + sRank; 
    if(map[tRank] == -1){
        /* first time insert, need to get res from Pool */
        pInd = deRPool(rPool, rPoolSize);
        if(pInd == -1){ /* no more res in Pool left */
            printf("NO resource in RPOOL left, please increase RPOOLSIZE!\n"); 
            exit(1);
        }

        /* mark it down in *map */
        map[tRank] = pInd;
    }

    pInd = map[tRank];
    sreq = rPool->pool + pInd;
    (sreq->ids)[(sreq->cnt)++] = sid;
}



void sendSeqReq(int rank, int sRank, RPOOL *rPool, int rPoolSize, int *map, int mapSize, MPI_Comm *comm){
    int i;
    int pInd;
    SREQ *sreq = NULL;

    assert(map[0] == -1 && map[1] == -1);
    for(i = sRank; i < mapSize; i++){
        pInd = map[i];
        if(pInd == -1) continue;

        sreq = rPool->pool + pInd;
        assert(rank != i);
        MPI_Issend(sreq->ids, sreq->cnt, MPI_INT, i, MSG_CR_TAG, *comm, &(sreq->req));
        sreq->stat = SREQ_SENT;
    }

    /* return res. back to rPool if any Issend() is finished */
    freeSeqReq(rPool, rPoolSize);
}



void freeSeqReq(RPOOL *rPool, int rPoolSize){
    int i;
    SREQ *sreq = NULL;
    int completed = 0;
    MPI_Status status;
    
    /* linearly scan for completed seq. req. */
    for(i = 0; i < rPoolSize; i++){
        sreq = rPool->pool + i;
        if(sreq->stat == SREQ_SENT){
            MPI_Test(&(sreq->req), &completed, &status);
            if(completed){
                sreq->stat = SREQ_FREE;
                (rPool->space)++;
            }
        }
    }
}


/*################################################*
 *       FOR SENDING SEQ. STR. USING SPOOL
 *################################################*/

void prepStr(int rank, int *rvBuf, int rvBufSize, SEQ *seqs, int nSeqs, int tRank,
                SPOOL *sPool, int sPoolSize, int maxStrSize, MPI_Comm *comm){
    int i;
    int id;
    STR *st = NULL;
    int sInd;
    int position = 0;

    sInd = deSPool(sPool, sPoolSize);
    st = sPool->pool + sInd;

    MPI_Pack(&rvBufSize, 1, MPI_INT, st->str, maxStrSize, &position, *comm);
    //printf("rank=%d, sInd=%d, rvBufSize=%d, st->str=%s -- st->stat=%d, maxStrSize=%d, position=%d\n", 
    //            rank, sInd, rvBufSize, st->str, st->stat, maxStrSize,position);

    for(i = 0; i < rvBufSize; i++){
        id = rvBuf[i];
        assert(id < nSeqs);
         
        //printf("rank=%d,id=%d, str=%s\n", rank, id, seqs[id].str);
        MPI_Pack(&id, 1, MPI_INT, st->str, maxStrSize, &position, *comm);
        MPI_Pack(seqs[id].str, seqs[id].strLen+1, MPI_CHAR, st->str, maxStrSize, &position, *comm);
    }

    MPI_Issend(st->str, position, MPI_PACKED, tRank, MSG_CS_TAG, *comm, &(st->req));
    st->stat = STR_SENT;

    /* free some completed res. if any Issend finished already */
    freeStr(sPool, sPoolSize);
}


void freeStr(SPOOL *sPool, int sPoolSize){
    int i;
    STR *st = NULL;
    int completed = 0;
    MPI_Status status;

    for(i = 0; i < sPoolSize; i++){
        st = sPool->pool + i;
        if(st->stat == STR_SENT){
            MPI_Test(&(st->req), &completed, &status);
            if(completed){
                st->stat = STR_FREE;
                (sPool->space)++;
            }
        }
    }
}


void upackStr(char *rvStr, int rvStrSize, SEQ *seqs, int nSeqs, char *upBuf, u64 *nByte, MPI_Comm *comm){
    int i;
    int id;
    int seqCnt;
    int position = 0;
    int strLen;

    
    MPI_Unpack(rvStr, rvStrSize, &position, &seqCnt, 1, MPI_INT, *comm);

    for(i = 0; i < seqCnt; i++){
        MPI_Unpack(rvStr, rvStrSize, &position, &id, 1, MPI_INT, *comm);
        assert(id < nSeqs);
        assert(seqs[id].stat == SEQ_R);

        strLen = seqs[id].strLen + 1;
        *nByte += strLen;
        MPI_Unpack(rvStr, rvStrSize, &position, upBuf, strLen, MPI_CHAR, *comm);
        seqs[id].str = estrdup(upBuf);
        seqs[id].stat = SEQ_F;
    }

}

/* ----------------------------------------------------------*
 * 
 * @param rank - rank of calling consumer in each subgroup
 * @param master - master node rank in each subgroup
 * @param stat - status indicator for #pairs on consumer
 * ----------------------------------------------------------*/ 

void sendStat(int rank, int master, int stat, CPOOL *cPool, int cPoolSize, MPI_Datatype msgMdt, MPI_Comm *comm){
    int cInd; 
    CSREQ *csreq = NULL; 
    
    cInd = deCPool(cPool, cPoolSize);
    csreq = cPool->pool + cInd;

    (csreq->msg).tag = TAG_C; 
    (csreq->msg).id1 = rank;
    (csreq->msg).id2 = stat;

    MPI_Issend(&(csreq->msg), 1, msgMdt, master, MSG_CM_TAG, *comm, &(csreq->req));
    csreq->stat = CS_SENT;

    /* release res. if possible */
    releaseCSReq(cPool, cPoolSize);

}

void releaseCSReq(CPOOL *cPool, int cPoolSize){
    int i;
    CSREQ *csreq = NULL;
    int completed = 0;
    MPI_Status status;

    for(i = 0; i < cPoolSize; i++){
        csreq = cPool->pool + i;
        
        if(csreq->stat == CS_SENT){
            MPI_Test(&(csreq->req), &completed, &status);
            if(completed){
                csreq->stat = CS_FREE;
                (cPool->space)++;
            }
        }
    }
}
