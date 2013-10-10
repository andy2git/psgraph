/*
 * $Rev$ 
 * $Date$ 
 * $Author$
 *
 * Copyright 2010 Washington State University. All rights reserved.
 * ----------------------------------------------------------------
 *
 * Supermaster is in charge of suffix trees dispatch and pairs 
 * redistribution among subgroups!
 */

#include "spmaster.h"

/* -----------------------------------------------------*
 * @param gSize -
 * @param nGroup -
 * @param procs -
 * @param cfgFile -
 * @param msgMdt -
 * @param nFiles -
 * -----------------------------------------------------*/
void spMaster(int gSize, int pdSize, int nGroup, int procs, char *cfgFile, MPI_Datatype msgMdt, int nFiles){
    MPI_Request rvReq;
    MPI_Status rvStat; 
    int rvMaxBatch;
    int rvMsgCnt;
    int completed;

    MPI_Request *sdReq = NULL;
    MPI_Status *sdStat = NULL;
    int shrinkSize = 1;
    
    PBUF pBuf;
    int pBufSize;

    MSG **sBuf = NULL;  /* sBuf is allocated for all master nodes in each subgroup */
    int *start = NULL;
    int *done = NULL;
    int *sent = NULL;
    int *sdMsgCnt = NULL;
    int sdMaxBatch;

    int i;
    int ind = 0;
    int isEnd;
    int rvEnd = 0;
    int sdEnd = 0;
    MSG endMsg;
    MSG *msg = NULL;


    int fIn = 1;
    int fStop;
    int producer;

    double t1, t2;
    double iTime = 0.0f;
    double cpTime = 0.0f;
    double cmTime = 0.0f;


    pBufSize = getCfgVal(cfgFile, "SP_CBufSize");
    sdMaxBatch = getCfgVal(cfgFile, "SP_SendBatch");
    rvMaxBatch = getCfgVal(cfgFile, "SP_RecvBatch");
    shrinkSize = getCfgVal(cfgFile, "Buf_ShrinkSize");


    sdReq = emalloc(nGroup*(sizeof *sdReq));
    sdStat = emalloc(nGroup*(sizeof *sdStat));
    sBuf = emalloc(nGroup*(sizeof *sBuf));
    start = emalloc(nGroup*(sizeof *start));
    done = emalloc(nGroup*(sizeof *done));
    sent = emalloc(nGroup*(sizeof *sent));
    sdMsgCnt = emalloc(nGroup*(sizeof *sdMsgCnt));
    msg = emalloc(rvMaxBatch*(sizeof *msg));



    /* allocate sending buffer */
    for(i = 0; i < nGroup; i++){
        sBuf[i] = emalloc(sdMaxBatch*(sizeof *(sBuf[i])));
    }

    /* init vars */
    for(i = 0; i < nGroup; i++){
        start[i] = 1;
        sent[i] = 0;
    }


    initBuf(&pBuf, pBufSize);
    

    /* ---------------------------------------------------------------*
     *                 start to recv pairs from pd
     * ---------------------------------------------------------------*/
    ind = 0;
    fIn = 1;
    fStop = FILE_STOP;
    
    t1 = cTime();
    MPI_Irecv(msg, rvMaxBatch, msgMdt, MPI_ANY_SOURCE, MSG_PS_TAG, MPI_COMM_WORLD, &rvReq);
    t2 = cTime();
    cmTime += (t2 - t1);

    while(1){
        ind %= nGroup;
        
        /* sending pairs to subgroups */
        if(pBuf.data > 0){
            if(start[ind] == 1){
                done[ind] = 1;
                start[ind] = 0;
            }else{
                t1 = cTime();
                MPI_Test(&(sdReq[ind]), &(done[ind]), &(sdStat[ind]));
                t2 = cTime();
                cmTime += (t2 - t1);
            }
        

            if(done[ind] == 1){
                t1 = cTime();
                sdMsgCnt[ind] = deBuf(&pBuf, pBufSize, sBuf[ind], sdMaxBatch, &isEnd);
                t2 = cTime();
                cpTime += (t2 -t1);

                if(isEnd == 1) sdEnd++;

                if(sdMsgCnt[ind] > 0){
                    /* ind*gSize means master ID of the subgroup */
                    printf("SM: sending %d pairs to master[%d], pBuf.data=%d\n", sdMsgCnt[ind], ind*gSize, pBuf.data);
                    t1 = cTime();
                    MPI_Issend(sBuf[ind], sdMsgCnt[ind], msgMdt, ind*gSize, MSG_PM_TAG, MPI_COMM_WORLD, &sdReq[ind]);
                    sent[ind] = 1;
                    t2 = cTime();
                    cmTime += (t2 - t1);
                    done[ind] = 0;
                    ind++;
                }
            }else ind++;


            /* overflow could happen, so no need to pick Irecv()
             * TODO RISK: other producers might be sending their file done 
             *       msg out, but will not return until sp recvs */
            if(pBuf.data+rvMaxBatch >= pBufSize) continue;
            

            /* recv pairs */
            t1 = cTime();
            MPI_Test(&rvReq, &completed, &rvStat);
            t2 = cTime();
            cmTime += (t2 - t1);

            if(completed == 1){
                t1 = cTime();
                MPI_Get_count(&rvStat, msgMdt, &rvMsgCnt);
                t2 = cTime();
                cmTime += (t2 - t1);

                printf("SM: recv %d pairs from producer[%d], pBuf.data=%d\n", rvMsgCnt, rvStat.MPI_SOURCE, pBuf.data);

                if(msg[0].tag == TAG_P){
                    enBuf(&pBuf, pBufSize, msg, rvMsgCnt);

                }else if(msg[0].tag == TAG_E){
                    enBuf(&pBuf, pBufSize, msg, rvMsgCnt);

                    rvEnd++;                
                    /* no new pairs will flush in */
                    if(rvEnd == pdSize*nGroup) break;

                }else if(msg[0].tag == TAG_F){
                    producer = rvStat.MPI_SOURCE;
                    if(fIn <= nFiles){
                        printf("SM - send fd[%d] to pd[%d]\n", fIn, producer);
                        t1 = cTime();
                        MPI_Ssend(&fIn, 1, MPI_INT, producer, MSG_SP_TAG, MPI_COMM_WORLD);
                        t2 = cTime();
                        cmTime += (t2 - t1);

                        fIn++;
                    }else{
                        printf("SM - send file stop signal to pd[%d]\n", producer);
                        t1 = cTime();
                        MPI_Ssend(&fStop, 1, MPI_INT, producer, MSG_SP_TAG, MPI_COMM_WORLD);
                        t2 = cTime();
                        cmTime += (t2 - t1);
                    }
                }else{
                    printf("SM - SOME THING IS WRONG!\n");
                }

                t1 = cTime();
                MPI_Irecv(msg, rvMaxBatch, msgMdt, MPI_ANY_SOURCE, MSG_PS_TAG, MPI_COMM_WORLD, &rvReq);
                t2 = cTime();
                cmTime += (t2 - t1);
            }
        }else{
            t1 = cTime();
            MPI_Wait(&rvReq, &rvStat);
            t2 = cTime();
            iTime += (t2 - t1);
            

            MPI_Get_count(&rvStat, msgMdt, &rvMsgCnt);
            printf("SM: recv %d pairs from producer[%d], pBuf.data=%d\n", rvMsgCnt, rvStat.MPI_SOURCE, pBuf.data);
            assert(rvMsgCnt >= 1);
            
            if(msg[0].tag == TAG_P){
                enBuf(&pBuf, pBufSize, msg, rvMsgCnt);

            }else if(msg[0].tag == TAG_E){
                enBuf(&pBuf, pBufSize, msg, 1);
                rvEnd++;                

                /* no new pairs will flush in */
                if(rvEnd == pdSize*nGroup) break;

            }else if(msg[0].tag == TAG_F){
                producer = rvStat.MPI_SOURCE;
                if(fIn <= nFiles){
                    printf("SM - send fd[%d] to pd[%d]\n", fIn, producer);
                    t1 = cTime();
                    MPI_Ssend(&fIn, 1, MPI_INT, producer, MSG_SP_TAG, MPI_COMM_WORLD);
                    t2 = cTime();
                    cmTime += (t2 - t1);

                    fIn++;
                }else{
                    printf("SM - send file stop signal to pd[%d]\n", producer);
                    t1 = cTime();
                    MPI_Ssend(&fStop, 1, MPI_INT, producer, MSG_SP_TAG, MPI_COMM_WORLD);
                    t2 = cTime();
                    cmTime += (t2 - t1);
                }
            }else{
                printf("SM - SOMETHING IS WRONG!\n");
            }

            t1 = cTime();
            MPI_Irecv(msg, rvMaxBatch, msgMdt, MPI_ANY_SOURCE, MSG_PS_TAG, MPI_COMM_WORLD, &rvReq);
            t2 = cTime();
            cmTime += (t2 - t1);
        }
    }



    /* -----------------------------------------------------------*
     * process the left pairs in supermaster
     * a shrinked sdMaxBatch might help for ending load balancing 
     * -----------------------------------------------------------*/
    
    sdMaxBatch /= shrinkSize;
    while(sdEnd != pdSize*nGroup){
        ind %= nGroup;

        if(start[ind] == 1){
            done[ind] = 1;
            start[ind] = 0;
        }else {
            t1 = cTime();
            MPI_Test(&sdReq[ind], &done[ind], &sdStat[ind]);
            t2 = cTime();
            cmTime += (t2 - t1);
        }

        if(done[ind] == 1){
            sdMsgCnt[ind] = deBuf(&pBuf, pBufSize, sBuf[ind], sdMaxBatch, &isEnd);
            if(isEnd == 1) sdEnd++; 

            if(sdMsgCnt[ind] > 0){
                /* ind*gSize means master ID of the subgroup */
                printf("SM: ending - sending %d pairs to master[%d], pBuf.data=%d\n", sdMsgCnt[ind], ind*gSize, pBuf.data);
                t1 = cTime();
                MPI_Issend(sBuf[ind], sdMsgCnt[ind], msgMdt, ind*gSize, MSG_PM_TAG, MPI_COMM_WORLD, &sdReq[ind]);
                sent[ind] = 1;
                t2 = cTime();
                cmTime += (t2 - t1);

                done[ind] = 0;
                ind++;
            }
        }else ind++;
    }



    /* make sure all pairs are sent out to subgroups */
    for(i = 0; i < nGroup; i++){
        t1 = cTime();
        if(sent[i] == 1) MPI_Wait(&sdReq[i], &sdStat[i]);
        t2 = cTime();
        printf("SM: waiting master[%d] for <%.2lf> secs\n", i*gSize, t2 - t1);
        iTime += (t2 - t1);
    }
    //MPI_Waitall(nGroup, sdReq, sdStat);


    /* send ending msg to subgroups */
    endMsg.tag = TAG_E;
    endMsg.id1 = 0;
    endMsg.id2 = 0;
    
    for(i = 0; i < nGroup; i++){
        printf("SM: sending ENDING pairs to master[%d]\n", i*gSize);
        t1 = cTime();
        MPI_Ssend(&endMsg, 1, msgMdt, i*gSize, MSG_PM_TAG, MPI_COMM_WORLD);
        t2 = cTime();
        cmTime += (t2 - t1);
    }


    /* free memory */
    free(sdReq);
    free(sdStat);
    for(i = 0; i < nGroup; i++){
        free(sBuf[i]);
    }    
    free(sBuf); 
    free(start);
    free(done);
    free(sent);
    free(sdMsgCnt);
    freeBuf(&pBuf);
    free(msg);
    
    printf("SM: I AM DONE, Idle <%.2lf> secs, cmTime <%.2lf> secs, cpTime <%.2lf> secs:D\n", iTime, cmTime, cpTime);
}
