#include "producer.h"

int producer(int master, int spMaster, int groupID, char *frPath, char *cfgFile, int nSeqs, 
                MPI_Datatype msgMdt, MPI_Comm *comm){
    PBUF pBuf;
    int pBufSize;
    int sdMaxBatch;

    FILE *fp = NULL;
    char frFile[MAX_FILENAME_LEN];
    int fIndex;
    MPI_Status fStat;
    MSG fEndMsg;
    

    MSG endMsg;
    MPI_Request request;
    MPI_Status status;
    int isStart = 1;

    MSG *chunk = NULL;
    int chunkSize;
    int sdMsgCnt;
    int mDone = 1;        /* sending to master done? */


    MPI_Request sReq;
    MPI_Status sStat;
    int isSPStart = 1;

    MSG *sBuf = NULL;    /* used to send pairs to supermaster */
    int sBufSize;
    int sBufCnt;
    int sDone = 1;       /* sending to supermaster done? */

    double t1, t2;
    double pTime = 0.0f;
    double sTime = 0.0f;
    double iTime = 0.0f;
    double t3, t4;
    

    chunkSize = getCfgVal(cfgFile, "PD_SendBatch");
    pBufSize = getCfgVal(cfgFile, "PD_PBufSize");
    sdMaxBatch = getCfgVal(cfgFile, "PD_SendBatch");
    sBufSize = chunkSize;
    

    initBuf(&pBuf, pBufSize);
    chunk = emalloc(chunkSize*(sizeof *chunk));
    sBuf = emalloc(sBufSize*(sizeof *sBuf));

    fEndMsg.tag = TAG_F;
    fEndMsg.id1 = 0;
    fEndMsg.id2 = 0;

    MPI_Ssend(&fEndMsg, 1, msgMdt, spMaster, MSG_PS_TAG, MPI_COMM_WORLD);

    t1 = cTime();
    while(1){
        printf("Group[%d] - Producer waiting for fd from sp!\n", groupID);
        MPI_Recv(&fIndex, 1, MPI_INT, spMaster, MSG_SP_TAG, MPI_COMM_WORLD, &fStat);
        printf("Group[%d] - Producer got fd[%d] from sp!\n", groupID, fIndex);
        if(fIndex == FILE_STOP) break;
        
        sprintf(frFile, "%s/forest_%d", frPath, fIndex);

        printf("Group[%d] - Producer working on file %s\n", groupID, frFile);
        /* check file exists */
        fp = fopen(frFile, "r");
        if(fp == NULL){
            printf("Group[%d] - Producer: file <%s> does not exist\n", groupID, frFile);
            break;
        }
        fclose(fp);
        
        t3 = cTime();
        processForest(groupID, master, frFile, cfgFile, nSeqs, &pBuf, pBufSize, &isStart, chunk, chunkSize, &iTime, &request, msgMdt, comm);
        t4 = cTime();
        printf("Group[%d] - process fd[%d] in <%.2lf> secs!\n", groupID, fIndex, t4 - t3);

        printf("Group[%d] - PD sending done signal for fd[%d] to sp!\n", groupID, fIndex);
        MPI_Ssend(&fEndMsg, 1, msgMdt, spMaster, MSG_PS_TAG, MPI_COMM_WORLD);
        printf("Group[%d] - PD send out done signal for fd[%d] to sp!\n", groupID, fIndex);

    }
    t2 = cTime();

    pTime += (t2 - t1);

    /* -------------------------------------------------------------------*/
    /* pair generation task are done, and need to empty the pairs in pBuf */
    /* -------------------------------------------------------------------*/
    printf("Group[%d] - PD done with pair generation! pBuf.data = %d\n", groupID, pBuf.data);


    t1 = cTime();
    while(1){

        /* initial special case */
        if(isStart == 1){
            mDone = 1;
            isStart = 0;
        }else MPI_Test(&request, &mDone, &status);

        if(mDone == 1){
            sdMsgCnt = deBuf(&pBuf, pBufSize, chunk, chunkSize, NULL);
            if(sdMsgCnt > 0){
                printf("Group[%d] - PD sending %d pairs to master\n", groupID, sdMsgCnt);
                MPI_Issend(chunk, sdMsgCnt, msgMdt, master, MSG_PM_TAG, *comm, &request);
            }else{
                break;
            }
        }


        /* initial special case */
        if(isSPStart == 1){
            sDone = 1;
            isSPStart = 0;
        }else MPI_Test(&sReq, &sDone, &sStat);  

        if(sDone == 1){
            sBufCnt = deBuf(&pBuf, pBufSize, sBuf, sBufSize, NULL);
            if(sBufCnt > 0){
                printf("Group[%d] - PD sending %d pairs to SP\n", groupID, sBufCnt);
                MPI_Issend(sBuf, sBufCnt, msgMdt, spMaster, MSG_PS_TAG, MPI_COMM_WORLD, &sReq);
            }else{
                break;
            }
       }
    }

    printf("Group[%d] - PD DONE with pair sending!\n", groupID);

    if(mDone == 0) MPI_Wait(&request, &status); 
    if(sDone == 0) MPI_Wait(&sReq, &sStat);

    t2 = cTime();
    sTime += (t2 - t1);

    /* indiciate the end of this producer */
    endMsg.tag = TAG_E;
    endMsg.id1 = 0;
    endMsg.id2 = 0;
    
    /* producer ONLY send END msg to supermaster */
    printf("Group[%d] - PD sending END msg to SP!\n", groupID);
    MPI_Ssend(&endMsg, 1, msgMdt, spMaster, MSG_PS_TAG, MPI_COMM_WORLD);

    free(chunk);
    free(sBuf);
    freeBuf(&pBuf);
    printf("Group[%d] - PRODUCER DONE, pair generate in <%.2lf> secs, ending send <%.2lf> secs, idle <%.2lf> secs \n", groupID, pTime, sTime, iTime);

    return 1;
}
