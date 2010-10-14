/*
 * $Rev: 810 $ 
 * $Date: 2010-09-15 22:40:44 -0700 (Wed, 15 Sep 2010) $ 
 * $Author: Andy $
 *
 * Copyright 2010 Washington State University. All rights reserved.
 * ----------------------------------------------------------------
 *
 * Each subgroup has ONLY one master, whose reponsibilities are:
 *  1. recv pairs from producers, and supermaster
 *  2. send pairs to consumers based on their pBuf status
 */

#include "master.h"

/* -----------------------------------------------------------------------------*
 *
 * NOTE:when enBuf() pairs from producer's side. All pairs including the TAG_E pairs
 * will be put into pBuf. However, when deBuf() from pBuf, ending pairs TAG_E 
 * will not be poped out.
 *
 * @param master  -
 * @param groupID -
 * @param gSize   -
 * @param procs   -
 * @param nSeqs   -
 * @param cfgFile - 
 * @param msgMdt  -
 * @param comm    -
 * ----------------------------------------------------------------------------*/
void master(int master, int groupID, int gSize, int pdSize, int procs, int nSeqs, char *cfgFile, MPI_Datatype msgMdt, MPI_Comm *comm){
    PBUF pBuf;
    int pBufSize;

    CREQ cReq[R_SIZE];
    REQ *req = NULL;

    MSG *rvMsg = NULL;
    int rvMaxBatch;
    int rvMsgCnt;
    int rvEp = 0; 
    int ENDSIG = 1;   /* #(ending signals) to be received */
    MPI_Request lReq;
    MPI_Status lStat;
    int lDone;

    MSG *spMsg = NULL;  /* recv pairs from supermaster */
    int spMaxBatch;
    int spMsgCnt;
    int sp = procs - 1;
    MPI_Request spReq;
    MPI_Status spStat;
    int spDone;
    int swtch = 1;


    MSG *sdMsg = NULL;
    int sdMaxBatch;
    int sdBatch;
    int sdMsgCnt;
    int prio;
    int sdEp = 0; /* #(send ended producers) */
    int isEnd = 0;
    int shrinkSize = 1;


    int i;
    MSG endMsg;
    int endCnt;
    int consumer;

    int cmSize;
    int sRank; /* start rank for consumer */
    int eRank; /* end rank for consumer */
    int nGroups;

    u64 nAlg = 0;
    u64 nGen = 0;
    
    double cpTime = 0.0f; /* computation time: union find update + buf ops */
    double cmTime = 0.0f; /* communication time */
    double rvTime = 0.0f; /* recv time */
 
    /* read in configure file value */
    pBufSize = getCfgVal(cfgFile, "MS_CBufSize");
    rvMaxBatch = getCfgVal(cfgFile, "MS_PairRecvBatch");
    sdMaxBatch = getCfgVal(cfgFile, "MS_PairSendBatch");
    shrinkSize = getCfgVal(cfgFile, "Buf_ShrinkSize");
    spMaxBatch = rvMaxBatch;
    

    rvMsg = emalloc(rvMaxBatch*(sizeof *rvMsg));
    sdMsg = emalloc(sdMaxBatch*(sizeof *sdMsg));
    spMsg = emalloc(spMaxBatch*(sizeof *spMsg));


    sRank = master + pdSize + 1;
    eRank = master + gSize - 1;

    if(procs%gSize == 0) nGroups = procs/gSize;
    else nGroups = procs/gSize + 1;

    /* procs-1 is super master node */
    /* NOTE: check for LAST group */

    if(groupID >= nGroups){
        eRank = (eRank > ((procs-2)%gSize)) ? ((procs-2)%gSize) : eRank;
    }
    cmSize = eRank - sRank + 1;
    

    printf("Group[%d] - master[%d]: sRank=%d, eRank=%d\n", groupID, master, sRank, eRank);

    /* initialization */
    initBuf(&pBuf, pBufSize);
    initReq(cReq, R_SIZE, sRank, eRank, procs);

    double t1, t2;

    /* ----------------------------------------------*
     *   Recv pairs/END msg from supermaster
     * ----------------------------------------------*/
    assert(sp == procs -1);
    MPI_Irecv(spMsg, spMaxBatch, msgMdt, sp, MSG_PM_TAG, MPI_COMM_WORLD, &spReq);
    MPI_Irecv(rvMsg, rvMaxBatch, msgMdt, MPI_ANY_SOURCE, MPI_ANY_TAG, *comm, &lReq);
    while(1){
        if(pBuf.data >= rvMaxBatch){
            /* ----------------------------------------------*
             *  supposed to recv from consumers ONLY
             * ----------------------------------------------*/
            MPI_Test(&lReq, &lDone, &lStat);
            if(lDone == 1){
                if(rvMsg[0].tag == TAG_P){
                    MPI_Get_count(&lStat, msgMdt, &rvMsgCnt);
                    printf("Group[%d] - master[%d]: recv %d pairs from -PD%d!\n", groupID, master, rvMsgCnt, lStat.MPI_SOURCE);

                    enBuf(&pBuf, pBufSize, rvMsg, rvMsgCnt);
                    nGen += rvMsgCnt;
                }else if(rvMsg[0].tag == TAG_C){ 
                    printf("Group[%d] - master[%d]: recv request from -CS%d!\n", groupID, master, lStat.MPI_SOURCE);

                    enReq(cReq, R_SIZE, &(rvMsg[0]));
                }else{
                    printf("Group[%d] - master[%d]: something is WRONG!\n", groupID, master);
                    exit(0);
                }

                MPI_Irecv(rvMsg, rvMaxBatch, msgMdt, MPI_ANY_SOURCE, MSG_CM_TAG, *comm, &lReq);
            }

        }else{
            /* ----------------------------------------------*
             *   Recv pairs/END msg from supermaster
             * ----------------------------------------------*/
            MPI_Test(&spReq, &spDone, &spStat);

            if(spDone == 1){
                if(spMsg[0].tag == TAG_P){
                    MPI_Get_count(&spStat, msgMdt, &spMsgCnt);
                    printf("Group[%d] - master[%d]: recv %d pairs from SP!\n", groupID, master, spMsgCnt);


                    /* -------------------------------------------*
                     * SHRINK "spMaxBatch" and "sdMaxBatch"
                     * -------------------------------------------*/
                     
                    /* emptying buffer from super master! PRODUCERS are done NOW
                       WILL ONLY RECV PAIRS FROM SPMASTER */
                    if(swtch == 1 && shrinkSize*spMsgCnt == spMaxBatch){
                        spMaxBatch /= shrinkSize;  /* for local buffer limit */
                        sdMaxBatch /= shrinkSize;  /* batchSize to consumers */
                        swtch = 0;
                    }


                    enBuf(&pBuf, pBufSize, spMsg, spMsgCnt);
                    nGen += spMsgCnt;
                }else if(spMsg[0].tag == TAG_E){
                    printf("Group[%d] - master[%d]: recv END signal from SP!\n", groupID, master);

                    rvEp++; 
                    enBuf(&pBuf, pBufSize, spMsg, 1);
                    nGen += 1;
                }else{
                    printf("Group[%d] - master[%d]: something is WRONG!\n", groupID, master);
                    exit(0);
                }

                /* post a new Irecv for supermaster */
                MPI_Irecv(spMsg, spMaxBatch, msgMdt, sp, MSG_PM_TAG, MPI_COMM_WORLD, &spReq);
            }

            /* ----------------------------------------------*
             *   Recv pairs from producer
             *   Recv request from consumers 
             * ----------------------------------------------*/
            MPI_Test(&lReq, &lDone, &lStat);
                
            if(lDone == 1){
                if(rvMsg[0].tag == TAG_P){           
                    MPI_Get_count(&lStat, msgMdt, &rvMsgCnt);
                    printf("Group[%d] - master[%d]: recv %d pairs from PD%d!\n", groupID, master, rvMsgCnt, lStat.MPI_SOURCE);

                    enBuf(&pBuf, pBufSize, rvMsg, rvMsgCnt);
                    nGen += rvMsgCnt;
                }else if(rvMsg[0].tag == TAG_C){ 
                    printf("Group[%d] - master[%d]: recv request from CS%d!\n", groupID, master, lStat.MPI_SOURCE);

                    enReq(cReq, R_SIZE, &(rvMsg[0]));
                }else{
                    printf("Group[%d] - master[%d]: something is WRONG!\n", groupID, master);
                    exit(0);
                }

                MPI_Irecv(rvMsg, rvMaxBatch, msgMdt, MPI_ANY_SOURCE, MPI_ANY_TAG, *comm, &lReq);
            }
        }

        /* NOTES:
         * Irecv() is pre-posted on consumer node, so Send() will 
         * NEVER block at this point */

        /* send out pairs as much as possible */
        while(pBuf.data >= sdMaxBatch && (req = deReq(cReq, R_SIZE, &prio))){

            switch(prio){
                case R_NONE: sdBatch = sdMaxBatch; break;
                case R_QUAT: sdBatch = sdMaxBatch*3/4; break;
                case R_HALF: sdBatch = sdMaxBatch/2; break;
                default: printf("Group[%d] - master[%d] something is wrong\n", groupID, master); exit(0); break;
            }

            /* pair ending msg will not be send to consumers */
            t1 = cTime();
            sdMsgCnt = deBuf(&pBuf, pBufSize, sdMsg, sdBatch, &isEnd);
            t2 = cTime();
            cpTime += (t2 - t1);


            nAlg += sdMsgCnt;

            if(isEnd == 1){
                sdEp++;
                isEnd = 0;
            }

            if(sdMsgCnt > 0){
                consumer = req->rank;
                printf("Group[%d] - master[%d]: pBuf.data = %d, consumer=%d, sdMsgCnt=%d\n", 
                        groupID, master, pBuf.data, consumer, sdMsgCnt);

                t1 = cTime();
                MPI_Send(sdMsg, sdMsgCnt, msgMdt, consumer, MSG_MC_TAG, *comm);
                t2 = cTime();
                cmTime += (t2 - t1);

            }else{ 
                fprintf(stderr, "Group[%d] - master[%d]: No DATA TO SEND!\n", groupID, master);
                exit(0);
            }
        }


        /* -------------------------------------------------------*
         *                   PRODUCER is DONE WITH 
         *                PAIR GENERATION AND SENDING
         * -------------------------------------------------------*/   


        /*
         * circular buffer format at ending stage:
         * 
         * -----|----> >=sdMaxBatch <----|---|-|---| 
         *      E                        E   E E   E
         */
        /* no more pairs will come */
        while(rvEp == ENDSIG && sdEp != ENDSIG){
            if((req = deReq(cReq, R_SIZE, &prio))){
                switch(prio){
                    case R_NONE: sdBatch = sdMaxBatch; break;
                    case R_QUAT: sdBatch = sdMaxBatch*3/4; break;
                    case R_HALF: sdBatch = sdMaxBatch/2; break;
                    default: printf("Group[%d] - master[%d] something is wrong!\n", groupID, master); exit(0); break;
                }

                //sdBatch = 200;

                /* pair ending msg will not be send to consumers */
                t1 = cTime();
                sdMsgCnt = deBuf(&pBuf, pBufSize, sdMsg, sdBatch, &isEnd);
                t2 = cTime();
                cpTime += (t2 - t1);


                nAlg += sdMsgCnt;

                if(isEnd == 1){
                    sdEp++;
                    isEnd = 0;
                }

                if(sdMsgCnt > 0){
                    consumer = req->rank;
                    printf("Group[%d] - master[%d]: ending: pBuf.data = %d, consumer=%d, sdEp=%d\n", groupID, master, pBuf.data, consumer, sdEp);
                    
                    t1 = cTime();
                    MPI_Send(sdMsg, sdMsgCnt, msgMdt, consumer, MSG_MC_TAG, *comm);
                    t2 = cTime();
                    cmTime += (t2 - t1);

                }else{
                    /* NOTE: only ONE ending pair left in buffer. IT IS NOT AN ERROR CASE */
                    assert(sdEp == ENDSIG);
                }

            }else{
                MPI_Test(&lReq, &lDone, &lStat);
                if(lDone == 1){
                    if(rvMsg[0].tag == TAG_P){           
                        MPI_Get_count(&lStat, msgMdt, &rvMsgCnt);
                        printf("Group[%d] - master[%d]: ending: recv %d pairs from PD%d!\n", groupID, master, rvMsgCnt, lStat.MPI_SOURCE);

                        enBuf(&pBuf, pBufSize, rvMsg, rvMsgCnt);
                        nGen += rvMsgCnt;
                    }else if(rvMsg[0].tag == TAG_C){
                        enReq(cReq, R_SIZE, &(rvMsg[0]));
                    }else{
                        fprintf(stderr, "Group[%d] - master[%d]: ending: WRONG MSG!\n", groupID, master);
                        exit(0);
                    }
                    MPI_Irecv(rvMsg, rvMaxBatch, msgMdt, MPI_ANY_SOURCE, MPI_ANY_TAG, *comm, &lReq);
                }
            }
        }

        if(rvEp == ENDSIG && sdEp == ENDSIG){
            
            endMsg.tag = TAG_S;
            endMsg.id1 = 0; /* rank of master */
            endMsg.id2 = 0;

            for(i = sRank; i <= eRank; i++){
                t1 = cTime();
                MPI_Ssend(&endMsg, 1, msgMdt, i, MSG_MC_TAG, *comm);
                t2 = cTime();
                cmTime += (t2 - t1);

                printf("Group[%d] - master[%d] sending END msg to cs[%d]!\n", groupID, master, i);
            }
            
            
            endCnt = 0;
            do{
                MPI_Test(&lReq, &lDone, &lStat);

                if(lDone == 1){
                    if(rvMsg[0].tag == TAG_S){
                        endCnt++;
                        printf("Group[%d] - master[%d] recv END msg from cs[%d], endCnt=%d!\n", 
                                groupID, master, lStat.MPI_SOURCE, endCnt);
                    }

                    MPI_Irecv(rvMsg, rvMaxBatch, msgMdt, MPI_ANY_SOURCE, MPI_ANY_TAG, *comm, &lReq);
                }
            }while(endCnt != cmSize);

            
            endMsg.tag = TAG_K;
            endMsg.id1 = 0; /* rank of master */
            endMsg.id2 = 0;

            for(i = sRank; i <= eRank; i++){
                t1 = cTime();
                MPI_Ssend(&endMsg, 1, msgMdt, i, MSG_MC_TAG, *comm);
                t2 = cTime();
                cmTime += (t2 - t1);

                printf("Group[%d] - master[%d] sending KILL msg to cs[%d]!\n", groupID, master, i);
            }

            break; /* break the while(1) loop */
        }
    }

    printf("Group[%d] - master[%d]: recv <%llu> pairs, aligned <%llu> pairs, cpTime <%.2lf> secs, cmTime <%.2lf> secs, rvTime <%.2lf> secs\n", 
            groupID, master, nGen, nAlg, cpTime, cmTime, rvTime);

    /* free memory */
    free(rvMsg);
    free(spMsg);
    free(sdMsg);
    freeBuf(&pBuf);
    relseReq(cReq, R_SIZE);

}
