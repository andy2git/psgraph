#include "consumer.h"

/* ---------------------------------------------------------*
 *
 * @param rank - local rank
 *
 *
 * ---------------------------------------------------------*/
void consumer(int master, int groupID, int gSize, int pdSize, int rank, char *seqFile, int nSeqs, char *cfgFile, char *outPath,
                 MPI_Datatype msgMdt, MPI_Comm *comm){

    MPI_Request request;
    MPI_Status status;

    MSG *msg = NULL;
    MSG cReq;
    int isSent = 0;
    MPI_Request sdReq;
    MPI_Status sdStat;

    int csSize; /* consumer size within each subgroup */

    int rvMaxBatch;
    int rvMsgCnt;
    int completed;
    int bufLimit;
    int swtch = 1;
    int shrinkSize = 1;

    FILE *fp = NULL;
    char outFile[MAX_NAME_LEN];


    PBUF pBuf;
    int pBufSize;     
    MSG pair;
    u64 nAln = 0UL;
    
    SEQ *seqs = NULL;
    int maxSeqLen = 0;

    /* statically cached seqs */
    int *cIds = NULL; 
    int cSeqSize;

    /* dynamically loaded seqs */
    int *dIds = NULL; 
    int dBatchSize;
    int dCnt = 0;  /* seqs not in *cIds */
    int pCnt = 0;

    CELL result;
    int f1, f2;
    int flag = 0;

    CELL **tbl = NULL;
    int **del = NULL;
    int **ins = NULL;
    PARAM param;


    /*----------------*
     * time profiling *
     *----------------*/
    double wTime = 0.0f;  /* waiting time */
    double aTime = 0.0f;  /* alignment time */
    double ioTime = 0.0f; /* io time */
    double asTime = 0.0f;  /* MPI asyn comm time */
    double sTime = 0.0f;  /* MPI syn comm time */
    double tTime = 0.0f;  /* MPI Test time */
    double t1, t2;


    csSize = gSize - pdSize - 1;

    /* cached seqs size */
    //cSeqSize = getCfgVal(cfgFile, "CS_NumOfPreCachedSeqs");
    cSeqSize = nSeqs/csSize;
    dBatchSize = getCfgVal(cfgFile, "CS_MaxDynamicSeqs");
    rvMaxBatch = getCfgVal(cfgFile, "CS_PairRecvMaxBatch");
    shrinkSize = getCfgVal(cfgFile, "Buf_ShrinkSize");
    param.AOL = getCfgVal(cfgFile, "AlignOverLongerSeq");
    param.SIM = getCfgVal(cfgFile, "MatchSimilarity");
    param.OS = getCfgVal(cfgFile, "OptimalScoreOverSelfScore");
    
    
    bufLimit = rvMaxBatch;
    pBufSize = 8*rvMaxBatch;  /* maximum buffer size */
    initBuf(&pBuf, pBufSize);

    seqs = emalloc(nSeqs*(sizeof *seqs));
    cIds = emalloc(nSeqs*(sizeof *cIds));
    dIds = ecalloc(nSeqs, sizeof *dIds);  /* ids need to be loaded in */
    msg = emalloc(rvMaxBatch*(sizeof *msg));

    /* open file for output */
    sprintf(outFile, "%s/pout_%d_%d", outPath, groupID, rank);
    fp = efopen(outFile, "w");


    /* cache some static random seqs */
    randIds(cIds, nSeqs, cSeqSize);
    maxSeqLen = cacheSeqs(seqFile, seqs, nSeqs, cIds, cSeqSize, 1); 

    assert(NROW == 2);
    tbl = allocTBL(NROW, maxSeqLen);
    del = allocINT(NROW, maxSeqLen);
    ins = allocINT(NROW, maxSeqLen);


    printf("Group[%d] - cs%d, My master is %d\n", groupID, rank, master);
    t1 = cTime();
    MPI_Irecv(msg, rvMaxBatch, msgMdt, MPI_ANY_SOURCE, MSG_MC_TAG, *comm, &request);
    t2 = cTime();
    asTime += (t2 - t1);

    flag = 1;
    while(1){

        if(pBuf.data > 0){
            
            if(pCnt == 0){
                t1 = cTime();

                /* initially nothing will happen for freeSeqs() */
                freeSeqs(seqs, dIds, nSeqs);

                dCnt = filterIds(cIds, dIds, nSeqs, &pBuf, pBufSize, dBatchSize);

                /* cacheSeqs will call fopen(), so try to avoid cacheSeqs() */
                if(dCnt > 0){
                    //printf("Group[%d] - cs%d: need to cache %d seqs\n", groupID, rank, dCnt);
                    cacheSeqs(seqFile, seqs, nSeqs, dIds, dCnt, 0);
                }

                t2 = cTime();
                ioTime += (t2 - t1);

                /* #pairs's seqs have been covered */
                pCnt = (pBuf.data >= dBatchSize)? dBatchSize : pBuf.data;;
            }
            

            /* Align sequences */
            t1 = cTime();
            pCnt--;
            deBuf(&pBuf, pBufSize, &pair, 1, NULL);


            /* end of program, send all left alignment results */
            if(pair.tag == TAG_S){
                cReq.tag = TAG_S;
                cReq.id1 = 0;
                cReq.id2 = 0;
                
                printf("Group[%d] - CS%d sending stop signal to Master=%d\n", groupID, rank, master);
                MPI_Ssend(&cReq, 1, msgMdt, master, MSG_CM_TAG, *comm);
                
                printf("Group[%d] - CS%d, Idle <%lf> secs, I/O <%.2lf> secs, Align [%llu] pairs in <%.2lf> secs, asyn <%.2lf> secs, syn <%.2lf> secs, test <%.2lf> secs\n",
                         groupID, rank, wTime, ioTime, nAln, aTime, asTime, sTime, tTime);

                /* END OF THE PROGRAM */
                break;
            }

            f1 = pair.id1;
            f2 = pair.id2;
            
            #ifdef DEBUG
            printf("--------------------------\n");
            printf("[%d=%d, %d=%d]\n", f1, seqs[f1].strLen, f2, seqs[f2].strLen);
            printf("%s\n", seqs[f1].str);
            printf("%s\n", seqs[f2].str);
            printf("--------------------------\n");
            #endif
            
            assert(seqs[f1].strLen < maxSeqLen);
            assert(seqs[f2].strLen < maxSeqLen);

            affineGapAlign(seqs[f1].str, seqs[f1].strLen, seqs[f2].str, seqs[f2].strLen, &result, tbl, del, ins);
            t2 = cTime();
            aTime += (t2 - t1);

            if(isEdge(&result, seqs[f1].str, seqs[f1].strLen, seqs[f2].str, seqs[f2].strLen, &param)){
                t1 = cTime();
                fprintf(fp, "%d#%d\n", f1, f2);
                t2 = cTime();
                ioTime += (t2 - t1);
            }

        }

        /* proc has nothing to do, so it will wait there */
        if(pBuf.data <= 0 && completed == 0){ 
            
            if(flag == 1){
                flag = 0;

                t1 = cTime();
                MPI_Wait(&request, &status);
                completed = 1;
                t2 = cTime();
                wTime += (t2-t1);
                printf("Group[%d] - cs%d waiting recv for <%.2lf>secs\n", groupID, rank, t2-t1);

            }else{

                /* wait for 1/2 req sent out */
                if(isSent == 1) {
                    t1 = cTime();
                    MPI_Wait(&sdReq, &sdStat);
                    isSent = 0;
                    t2 = cTime();
                    wTime += (t2-t1);
                    printf("Group[%d] - cs%d waiting 1/2 req <%.2lf>secs\n", groupID, rank, t2-t1);
                }
                
                
                /* waiting for recving pairs from master */
                t1 = cTime();
                MPI_Wait(&request, &status);
                completed = 1;
                t2 = cTime();
                wTime += (t2-t1);
                printf("Group[%d] - cs%d waiting recv for <%.2lf>secs\n", groupID, rank, t2-t1);



                /* -------------------------------*
                 *       0/0 request sending 
                 * -------------------------------*/
                cReq.tag = TAG_C;
                cReq.id1 = rank;
                cReq.id2 = R_NONE;
    
                t1 = cTime();
                MPI_Issend(&cReq, 1, msgMdt, master, MSG_CM_TAG, *comm, &sdReq);
                isSent = 1;
                t2 = cTime();
                sTime += (t2 - t1);

                printf("Group[%d] - cs%d: 0/0 req send out to Master%d in <%.2lf>\n", groupID, rank, master, t2 - t1);
            }
        }else{
            t1 = cTime();
            MPI_Test(&request, &completed, &status);
            t2 = cTime();
            tTime += (t2 - t1);
        }

    
        if(completed == 0){ /* Irecv not completed */
            if(2*pBuf.data == bufLimit){

                /* wait for 0/0 req sent out */
                if(isSent == 1) {
                    t1 = cTime();
                    MPI_Wait(&sdReq, &sdStat);
                    isSent = 0;
                    t2 = cTime();
                    wTime += (t2-t1);
                    printf("Group[%d] - cs%d waiting 0/0 req <%.2lf>secs\n", groupID, rank, t2-t1);
                }


                /* -------------------------------*
                 *       1/2 request sending 
                 * -------------------------------*/
                cReq.tag = TAG_C;
                cReq.id1 = rank;
                cReq.id2 = R_HALF;
                
                t1 = cTime();
                MPI_Issend(&cReq, 1, msgMdt, master, MSG_CM_TAG, *comm, &sdReq);
                isSent = 1;
                t2 = cTime();
                sTime += (t2 - t1);
            
                printf("Group[%d] - cs%d: 1/2 req send out to Master%d in <%.2lf> secs\n", groupID, rank, master, t2 - t1);
    
            } 
        }else if(completed == 1){
            
            t1 = cTime();
            MPI_Get_count(&status, msgMdt, &rvMsgCnt);
            t2 = cTime();
            tTime += (t2 - t1);

            printf("Group[%d] - cs%d: recved %d from Master%d, data=%d, PBUFSize=%d\n",
                     groupID, rank, rvMsgCnt, master, pBuf.data, pBufSize);


            enBuf(&pBuf, pBufSize, msg, rvMsgCnt);
            nAln += rvMsgCnt;

            /* shrink rvMaxBatch Size for better Load Balancing */
            if(swtch == 1 && (shrinkSize*rvMsgCnt == rvMaxBatch || shrinkSize*rvMsgCnt*2 == rvMaxBatch)) {
                bufLimit /= shrinkSize;
                swtch = 0;
            }


            t1 = cTime();
            MPI_Irecv(msg, rvMaxBatch, msgMdt, MPI_ANY_SOURCE, MSG_MC_TAG, *comm, &request);
            t2 = cTime();
            asTime += (t2 - t1);

            printf("Group[%d] - cs%d: posting Irecv\n", groupID, rank);
            completed = 0;
        }else{
            printf("Group[%d] - [cs%d] : something is WRONG!\n", groupID, rank);
            exit(0);
        }
    }

    
    printf("Group[%d] - [cs%d] : I AM DONE!\n", groupID, rank);

    /* free memory */
    freeSeqs(seqs, cIds, nSeqs);
    fclose(fp);

    freeTBL(tbl, NROW);
    freeINT(del, NROW);
    freeINT(ins, NROW);

    freeBuf(&pBuf);
    free(seqs);
    free(cIds);
    free(dIds);
    free(msg);
}
