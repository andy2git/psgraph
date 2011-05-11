/*
 * $Rev$ 
 * $Date$ 
 * $Author$
 *
 * Copyright 2010 Washington State University. All rights reserved.
 * ----------------------------------------------------------------
 *
 * This file ONLY performs consumers task!
 *  1. recv pairs from master node
 *  2. report status to master node
 *  3. send strs to other consumers if needed
 *  4. pairwise seq. alignment! - most import one!!
 */

#include "consumer.h"

/* 
 * @param master
 * @param groupID
 * @param groupSize
 * @param rank
 * @param seqFile
 * @param nSeqs
 * @param cfgFile
 * @param outPath
 * @param msgMdt
 * @param comm
 *
 */
void consumer(int master, int groupID, int groupSize, int pdSize, int rank, char *seqFile, int nSeqs, char *cfgFile, char *outPath,
                 MPI_Datatype msgMdt, MPI_Comm *comm){

    int isEnd = 0;
    int edge;
    int completed;
    int csSize;
    int sRank;
    int shrinkSize = 1;
    int swtch = 1;

    FILE *fp = NULL;
    char outFile[MAX_NAME_LEN];

    PBUF pBuf;
    int pBufSize;     
    int pBufLimit;

    /* seqs structure */
    SEQ *seqs = NULL;
    int maxSeqRange;
    int maxSeqLen = 0;

    /* -------------------------- *
     *         recv info
     * -------------------------- */

    /* recved pairs from master */
    MSG *msg = NULL;
    int rvMaxBatch;
    int rvMsgCnt;

    /* seqReq buffer for recving seq request */
    int *rvReqBuf = NULL; 
    int rvReqBufSize;      
    int rvReqBufCnt;

    /* recved seq strs */
    char *rvStr = NULL;
    int maxStrSize;
    int rvStrSize;
    char *upBuf = NULL;

    /* ------------ *
       0 - msg
       1 - seq req
       2 - seq 
     * ------------ */
    MPI_Request rvReq[IRECV_NUM];
    MPI_Status rvStat;
    int rvInd;


    /* -------------------------- *
     *         send info
     * -------------------------- */
    SPOOL sPool;
    int sPoolSize;

    RPOOL rPool;
    int rPoolSize;
    int *map = NULL; /* for track down res. */
    int mapSize;

    CPOOL cPool;
    int cPoolSize;

    MSG csMsg;

    /* -------------------------- *
     *       seq. alignment 
     * -------------------------- */
    CELL result;
    MSG pair;
    int f1, f2;

    CELL **tbl = NULL;
    int **del = NULL;
    int **ins = NULL;
    PARAM param;

    double aTime = 0.0f;    /* alignment time */
    double a2aTime = 0.0f;  /* all-to-all overhead */
    double ioTime = 0.0f;   /* output edges into disk */
    double wTime = 0.0f;    /* loop waiting time */
    double t1, t2;
    u64 nAln = 0ul;
    u64 nFetIds = 0ul;
    u64 nLoop = 0ul;
    u64 nByte = 0ul;
    u64 nStat = 0ul;
    u64 nSave = 0ul;


    /* cached seqs size */
    shrinkSize = getCfgVal(cfgFile, "Buf_ShrinkSize");
    rvMaxBatch = getCfgVal(cfgFile, "CS_PairRecvMaxBatch");
    param.AOL = getCfgVal(cfgFile, "AlignOverLongerSeq");
    param.SIM = getCfgVal(cfgFile, "MatchSimilarity");
    param.OS = getCfgVal(cfgFile, "OptimalScoreOverSelfScore");
    

    sRank = master + pdSize + 1;    
    csSize = groupSize - pdSize - 1; 
    pBufLimit = rvMaxBatch;
    maxSeqRange = flr(nSeqs, csSize);
    rvReqBufSize = maxSeqRange;

    sPoolSize = 2*csSize;
    rPoolSize = 4*csSize;
    cPoolSize = 4*R_SIZE;
    mapSize = groupSize;
    pBufSize = 8*rvMaxBatch;  /* maximum buffer size */

    seqs = emalloc(nSeqs*(sizeof *seqs));

    /* load partial seqs locally */
    loadStaticSeqs(rank, sRank, seqFile, seqs, nSeqs, maxSeqRange, &maxSeqLen);
    
    /* STR FORMAT: CNT id str\0 id str\0 .... */
    maxStrSize = maxRangeSum(seqs, nSeqs, maxSeqRange);
    maxStrSize += ((maxSeqRange+1)*sizeof(int)); /* +1 for CNT */


    initBuf(&pBuf, pBufSize);
    initSPool(&sPool, sPoolSize, maxStrSize);
    map = malloc(mapSize*(sizeof *map));
    initRPool(&rPool, rPoolSize, maxSeqRange);
    initCPool(&cPool, cPoolSize);


    msg = emalloc(rvMaxBatch*(sizeof *msg));
    rvStr = emalloc(maxStrSize*(sizeof *rvStr));
    rvReqBuf = emalloc(rvReqBufSize*(sizeof *rvReqBuf));
    upBuf = emalloc((maxSeqLen+1)*(sizeof *upBuf)); /* +1 for '\0' at the end */


    /* open file for output */
    sprintf(outFile, "%s/pout_%d_%d", outPath, groupID, rank);
    fp = efopen(outFile, "w");

    /* allocate mem for seq alignment */
    assert(NROW == 2);
    tbl = allocTBL(NROW, maxSeqLen);
    del = allocINT(NROW, maxSeqLen);
    ins = allocINT(NROW, maxSeqLen);


    printf("Group[%d] - cs%d, My master is %d, csSize=%d\n", groupID, rank, master, csSize);
    printf("Group[%d] - cs%d, rvmaxBatch=%d, rvReqBufSize=%d, maxStrSize=%d\n", groupID, rank, rvMaxBatch, rvReqBufSize, maxStrSize);

    MPI_Irecv(msg, rvMaxBatch, msgMdt, MPI_ANY_SOURCE, MSG_MC_TAG, *comm, rvReq);
    MPI_Irecv(rvReqBuf, rvReqBufSize, MPI_INT, MPI_ANY_SOURCE, MSG_CR_TAG, *comm, rvReq+1);
    MPI_Irecv(rvStr, maxStrSize, MPI_PACKED, MPI_ANY_SOURCE, MSG_CS_TAG, *comm, rvReq+2);

    while(1){
        t1 = cTime();
        MPI_Testany(IRECV_NUM, rvReq, &rvInd, &completed, &rvStat);
        t2 = cTime();
        wTime += (t2 -t1);
        if(completed) {
            switch(rvInd){
                /* -----------------------------*
                 * pairs from master
                 * -----------------------------*/
                case 0:
                    MPI_Get_count(&rvStat, msgMdt, &rvMsgCnt);
                    //printf("Group[%d] - cs%d, recv %d pairs from master\n", groupID, rank, rvMsgCnt);
                    enBuf(&pBuf, pBufSize, msg, rvMsgCnt);

                    /* shrink rvMaxBatch Size for better Load Balancing */
                    if(swtch == 1 && (shrinkSize*rvMsgCnt == rvMaxBatch || shrinkSize*rvMsgCnt*2 == rvMaxBatch)) {
                        pBufLimit /= shrinkSize;
                        swtch = 0;
                    }  

                    if(msg[0].tag == TAG_S || msg[0].tag == TAG_K){
                        /* NO NEED TO PREPARE SEQS FOR THIS MSG */
                    }else{
                        t1 = cTime();
                        prepSeqReq(sRank, msg, rvMsgCnt, seqs, nSeqs, map, mapSize, &rPool, rPoolSize, maxSeqRange, &nFetIds, &nStat, &nSave);
                        sendSeqReq(rank, sRank, &rPool, rPoolSize, map, mapSize, comm);

                        /* free those unneeded seqs */
                        freeSeqs(seqs, nSeqs, 0);
                        t2 = cTime();
                        a2aTime += (t2 - t1);
                    }

                    MPI_Irecv(msg, rvMaxBatch, msgMdt, MPI_ANY_SOURCE, MSG_MC_TAG, *comm, rvReq);
                    break;
                /* -----------------------------*
                 * seq reqs from other consumer
                 * -----------------------------*/
                case 1:
                    t1 = cTime();
                    MPI_Get_count(&rvStat, MPI_INT, &rvReqBufCnt);
                    //printf("Group[%d] - cs%d, recv %d seq. Req. from other consumers\n", groupID, rank, rvReqBufCnt);

                    prepStr(rank, rvReqBuf, rvReqBufCnt, seqs, nSeqs, rvStat.MPI_SOURCE, &sPool, sPoolSize, maxStrSize, comm);
                    t2 = cTime();
                    a2aTime += (t2 - t1);
                    
                    //printf("Group[%d] - cs%d, send str to cs%d\n", groupID, rank, rvStat.MPI_SOURCE);

                    MPI_Irecv(rvReqBuf, rvReqBufSize, MPI_INT, MPI_ANY_SOURCE, MSG_CR_TAG, *comm, rvReq+1);
                    break;
                /* -----------------------------*
                 * seq str from other consumer
                 * -----------------------------*/
                case 2:
                    t1 = cTime();
                    MPI_Get_count(&rvStat, MPI_CHAR, &rvStrSize);
                    //printf("Group[%d] - cs%d, recv %d strs from cs%d\n", groupID, rank, rvStrSize, rvStat.MPI_SOURCE);

                    upackStr(rvStr, rvStrSize, seqs, nSeqs, upBuf, &nByte, comm);
                    t2 = cTime();
                    a2aTime += (t2 - t1);

                    MPI_Irecv(rvStr, maxStrSize, MPI_PACKED, MPI_ANY_SOURCE, MSG_CS_TAG, *comm, rvReq+2);
                    break;
                default: 
                    eprintf("Group[%d] - rank %d recv some UNEXPECTED MSG from rank %d", groupID, rank, rvStat.MPI_SOURCE);
            }


        }else{
            if(pBuf.data > 0){
                deBuf(&pBuf, pBufSize, &pair, 1, NULL);            
                f1 = pair.id1;
                f2 = pair.id2;

                /* NO MORE PAIRS WILL COME IN FROM MASTER,
                 * BUT SOME PAIRS MIGHT STILL BE PENDED IN PBUF
                 */
                if(pair.tag == TAG_S) {
                    isEnd = 1;
                    continue;
                }

                /* NEED TO STOP THE PROGRAM */
                if(pair.tag == TAG_K){
                    printf("Group[%d] - cs%d, recv KILL signal from master\n", groupID, rank);
                    printf("Group[%d] - CS%d, Align [%llu] pairs <%.2lf> secs."
                            " nFetIds=%llu  nStat=%llu nSave=%llu -A2A <%.2lf> secs. [%llu] of wasted Loops. nBytes=%llu. "
                            "wTime=%.2lf secs, ioTime = %.2lf secs\n",
                                groupID, rank, nAln, aTime, 
                                nFetIds, nStat, nSave, a2aTime, nLoop, nByte,
                                wTime, ioTime);
                    break; /* BREAK WHIILE(1) LOOP TO END THE PROGRAM */
                }

                /* append the unready pair at the end of the pBuf */
                if(seqs[f1].stat == SEQ_R || seqs[f2].stat == SEQ_R) {
                    t1 = cTime();
                    enBuf(&pBuf, pBufSize, &pair, 1);
                    nLoop++;
                    t2 = cTime();
                    wTime += (t2 - t1);
                    continue; /* conitnue to  while(1) */
                }

                //printf("rank=%d [%d=%d, stat=%d, %d=%d, stat=%d]\n", rank, f1, seqs[f1].strLen, seqs[f1].stat, f2, seqs[f2].strLen, seqs[f2].stat);

                /*assert(seqs[f1].stat);
                assert(seqs[f2].stat);
                assert(seqs[f1].strLen <= maxSeqLen);
                assert(seqs[f2].strLen <= maxSeqLen);
                printf("--------------------------\n");
                printf("[%d=%d, stat=%d, %d=%d, stat=%d]\n", f1, seqs[f1].strLen, seqs[f1].stat, f2, seqs[f2].strLen, seqs[f2].stat);
                printf("%s\n", seqs[f1].str);
                printf("%s\n", seqs[f2].str);
                printf("--------------------------\n");*/

                seqs[f1].cnt--;
                seqs[f2].cnt--;
                nAln++;

                t1 = cTime();
                affineGapAlign(seqs[f1].str, seqs[f1].strLen, seqs[f2].str, seqs[f2].strLen, &result, tbl, del, ins);
                edge = isEdge(&result, seqs[f1].str, seqs[f1].strLen, seqs[f2].str, seqs[f2].strLen, &param);
                t2 = cTime();
                aTime += (t2 - t1);
                if(edge){
                    t1 = cTime();
                    fprintf(fp, "%d#%d\n", f1, f2);
                    t2 = cTime();
                    ioTime += (t2 - t1);
                } 


                /* ---------------------------------------------------*
                 * after a pair of seq. alignment, check pBuf stataus
                 * and send appropriate csReq to master for pairs 
                 * ---------------------------------------------------*/
                if(2*pBuf.data == pBufLimit){
                    /* send out 1/2 req. to master */ 
                    sendStat(rank, master, R_HALF, &cPool, cPoolSize, msgMdt, comm);
                    printf("Group[%d] - cs%d: 1/2 req send out to master\n", groupID, rank);
                }
            
                if(pBuf.data == 0){
                    /* send out 0/0 req. to master */
                    sendStat(rank, master, R_NONE, &cPool, cPoolSize, msgMdt, comm);
                    printf("Group[%d] - cs%d: 0/0 req send out to master\n", groupID, rank);
                }
            }else{
                /* No more pairs coming in, and pBuf.data = 0, 
                 * TIME TO STOP? NO WAY!!! YOU HAVE TO WAIT!!
                 * OTHER CONSUMERS MIGHT STILL NEED TO COMMUNICATE
                 * WITH YOU. SO YOU SHOULD KEEP ALIVE UNTIL MASTER
                 * NODE NOTIFIES YOU LATER!!!
                 */ 
                if(isEnd == 1){ 
                    csMsg.tag = TAG_S;
                    csMsg.id1 = 0;
                    csMsg.id2 = 0;
     
                    printf("Group[%d] - CS%d sending stop signal to Master=%d\n", groupID, rank, master);
                    MPI_Ssend(&csMsg, 1, msgMdt, master, MSG_CM_TAG, *comm);
                    isEnd = 0;
                }
            }
        }
    }


    /* free memory */
    freeRPool(&rPool, rPoolSize);
    free(map);
    freeSPool(&sPool, sPoolSize);
    freeCPool(&cPool);
    freeBuf(&pBuf);
    free(msg);

    freeSeqs(seqs, nSeqs, 1);
    free(seqs);

    free(rvStr);
    free(rvReqBuf);
    free(upBuf);


    freeTBL(tbl, NROW);
    freeINT(del, NROW);
    freeINT(ins, NROW);

    fclose(fp);
}
