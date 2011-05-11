/*
 * $Rev: 804 $ 
 * $Date: 2010-09-12 12:19:33 -0700 (Sun, 12 Sep 2010) $ 
 * $Author$
 *
 * Copyright 2010 Washington State University. All rights reserved.
 * ----------------------------------------------------------------
 *
 */

#include "pairs.h"

/*---------------------------------------------------------------*
 * Generate promising pairs, and enBuf them into pBuf[]
 *
 * @param stNodes -
 * @param srtIndex -
 * @param nStNodes -
 * @param nSeqs -
 * @param EM - exact match length cutoff
 * @param dup - duplication arrary for redundant pairs checking
 * @param pBuf - pair buffer
 * @param pBufSize - pair buffer maximum size
 * @param chunk - msg sending buffer
 * @param chunkSize - msg sending buffer size
 * @param isStart - if it is initial start, for MPI_Test()
 * @param msgMdt - derived mpi data type
 * @param request - mpi request used for MPI_Test()
 *
 *---------------------------------------------------------------*/
u64 genPairs(int master, STNODE *stNodes, int *srtIndex, int nStNodes, int nSeqs, int EM, int *dup, PBUF *pBuf, int pBufSize, 
            MSG *chunk, int chunkSize, int *isStart, double *iTime, double *aTime, MPI_Datatype msgMdt, MPI_Request *request, MPI_Comm *comm){
    int i;
    int j;
    int r;
    STNODE *stnode = NULL;
    int sIndex, eIndex;
    int m, n, s, t;
    SUFFIX *p = NULL;
    SUFFIX *q = NULL;
    int f1, f2;
    MSG msg;
    int completed;
    MPI_Status status;
    u64 nPairs = 0;
    double t1, t2;


    /* srtIndex maintain an order of NON-increasing depth of stNodes[] */
    for(i = 0; i < nStNodes; i++){
        sIndex = srtIndex[i];
        stnode = &stNodes[sIndex];

        #ifdef DEBUG
        printf("stNode->depth=%d, stnode->rLeaf=%ld, sIndex=%ld\n", stnode->depth, stnode->rLeaf, sIndex);
        #endif

        if(stnode->depth >= EM-1){
            if(stnode->rLeaf == sIndex){ /* leaf node */
                procLeaf(master, stnode->lset, pBuf, pBufSize, chunk, chunkSize, isStart, &nPairs, iTime, aTime, msgMdt, request, comm);
            }else{                       /* internal node */
                eIndex = stnode->rLeaf;

                /* init dup[] for the internal node */
                for(r = 0; r < nSeqs; r++) dup[r] = -1;
                
                /* pairs generation loop for internal node */
                for(m = sIndex+1; m < eIndex; m = stNodes[m].rLeaf+1){
                    for(n = stNodes[m].rLeaf+1; n <= eIndex; n = stNodes[n].rLeaf+1){
                        for(s = 0; s < SIGMA; s++){
                            if(stNodes[m].lset[s]){
                                for(t = 0; t < SIGMA; t++){
                                    if(stNodes[n].lset[t]){
                                       if(s != t){
                                            for(p = stNodes[m].lset[s]; p != NULL; p = p->next){
                                            
                                                /* eliminate pairs */ 
                                                if(dup[p->sid] == -1){
                                                    dup[p->sid] = p->sid;
                                                }else{
                                                    continue;
                                                }


                                                for(q = stNodes[n].lset[t]; q != NULL; q = q->next){
                                                    f1 = p->sid;
                                                    f2 = q->sid;
                                                    
                                                    if(f1 == f2) continue;

                                                    msg.tag = TAG_P;
                                                    msg.id1 = f1;
                                                    msg.id2 = f2;
                                                    nPairs++;
                                                        
                                                    if(pBuf->data < pBufSize){
                                                        enBuf(pBuf, pBufSize, &msg, 1);
                                                    }else{
                                                        t1 = cTime();    
                                                        MPI_Wait(request, &status);               
                                                        t2 = cTime();    
                                                        *iTime += (t2 - t1);

                                                        /* buf is almost full, so deBuf will definitely get some pairs out */
                                                        deBuf(pBuf, pBufSize, chunk, chunkSize, NULL);
                                                        t1 = cTime();    
                                                        MPI_Issend(chunk, chunkSize, msgMdt, master, MSG_PM_TAG, *comm, request);
                                                        t2 = cTime();    
                                                        *aTime += (t2 - t1);
                                                        enBuf(pBuf, pBufSize, &msg, 1);
                                                        continue;
                                                    }

                                                    if(pBuf->data >= chunkSize){
                                                        if(*isStart == 1){
                                                            completed = 1;
                                                            *isStart = 0;
                                                        }else{
                                                            MPI_Test(request, &completed, &status);
                                                        }

                                                        if(completed == 1){
                                                            deBuf(pBuf, pBufSize, chunk, chunkSize, NULL);
                                                            MPI_Issend(chunk, chunkSize, msgMdt, master, MSG_PM_TAG, *comm, request);
                                                            completed = 0;
                                                        }
                                                    }
                                                }
                                            }
                                        } 
                                    }
                                }
                            }
                        }
                    }
                }
    

                /* merge the lsets of subtree */ 
                for(m = 0; m < SIGMA; m++){
                    p = NULL;
                    for(j = sIndex+1; j <= eIndex; j++){
                        if((q = stNodes[j].lset[m])){

                            /* empty the subtree's ptrs array */
                            stNodes[j].lset[m] = NULL;
                            if(p == NULL) {
                                p = q;
                                stNodes[sIndex].lset[m] = q;
                            } else p->next = q;

                            /* walk to the end */
                            while(p->next) p = p->next;
                        }  
                    }
                }
            }
        }else{
            /* stnodes are sorted, so later part 
             * will not satisfy EM cutoff */        
            break;
        }
    }  

    return nPairs;
}

/*---------------------------------------------------------------*
 * This function implements the pair generation algorithm for leaf
 * nodes. 
 *
 *    BEGIN - intra/inter cross. O/W - intra cross. 
 *
 * @param lset -
 * @param pBuf -
 * @param pBufSize - 
 *---------------------------------------------------------------*/
void procLeaf(int master, SUFFIX **lset, PBUF *pBuf, int pBufSize, MSG *chunk, int chunkSize, int *isStart, u64 *nPairs, 
                    double *iTime, double *aTime, MPI_Datatype msgMdt, MPI_Request *request, MPI_Comm *comm){
    int i;
    int j;
    SUFFIX *p = NULL;
    SUFFIX *q = NULL;
    int f1, f2;
    MSG msg;
    int completed;
    MPI_Status status;
    double t1, t2;

    for(i = 0; i < SIGMA; i++){
        if(lset[i]){
            if(i == BEGIN - 'A'){ /* inter cross */
                for(p = lset[i]; p != NULL; p = p->next){
                    for(q = p->next; q != NULL; q = q->next){
                        f1 = p->sid;
                        f2 = q->sid;

                        if(f1 == f2) continue;

                        msg.tag = TAG_P;
                        msg.id1 = f1;
                        msg.id2 = f2;

                        (*nPairs)++;

                        if(pBuf->data < pBufSize){
                            enBuf(pBuf, pBufSize, &msg, 1);
                        }else{
                            t1 = cTime();   
                            MPI_Wait(request, &status);
                            t2 = cTime();
                            *iTime += (t2 - t1);

                            /* cbuf is almost full, so deBuf() will definitely get some pairs */
                            deBuf(pBuf, pBufSize, chunk, chunkSize, NULL);
                            t1 = cTime();   
                            MPI_Issend(chunk, chunkSize, msgMdt, master, MSG_PM_TAG, *comm, request);
                            t2 = cTime();
                            *aTime += (t2 - t1);
                            enBuf(pBuf, pBufSize, &msg, 1);
                            continue;
                        }

                        if(pBuf->data >= chunkSize){
                            /* special case, MPI_Test with MPI_I* will cause error */
                            if(*isStart == 1){
                                completed = 1;
                                *isStart = 0;
                            }else{
                                MPI_Test(request, &completed, &status);
                            }

                            if(completed == 1){
                                deBuf(pBuf, pBufSize, chunk, chunkSize, NULL);
                                MPI_Issend(chunk, chunkSize, msgMdt, master, MSG_PM_TAG, *comm, request);
                            }
                        }
                    }
                } 
            }        
           
            /* intra cross */
            for(j = i+1; j < SIGMA; j++){
                if(lset[j]){
                    for(p = lset[i]; p != NULL; p = p->next){
                        for(q = lset[j]; q != NULL; q = q->next){
                            f1 = p->sid;
                            f2 = q->sid;

                            if(f1 == f2) continue;

                            msg.tag = TAG_P;
                            msg.id1 = f1;
                            msg.id2 = f2;

                            (*nPairs)++;

                            if(pBuf->data < pBufSize){
                                enBuf(pBuf, pBufSize, &msg, 1);
                            }else{
                                t1 = cTime();
                                MPI_Wait(request, &status);
                                t2 = cTime();
                                *iTime += (t2 - t1);

                                /* buf is almost full, so deBuf() will definitely get some pairs */
                                deBuf(pBuf, pBufSize, chunk, chunkSize, NULL);
                                t1 = cTime();
                                MPI_Issend(chunk, chunkSize, msgMdt, master, MSG_PM_TAG, *comm, request);
                                t2 = cTime();
                                *aTime += (t2 - t1);
                                enBuf(pBuf, pBufSize, &msg, 1);
                                continue;
                            }

                            if(pBuf->data >= chunkSize){
                                if(*isStart == 1){
                                    completed = 1;
                                    *isStart = 0;
                                }else{
                                    MPI_Test(request, &completed, &status);
                                }

                                if(completed == 1){
                                    deBuf(pBuf, pBufSize, chunk, chunkSize, NULL);
                                    MPI_Issend(chunk, chunkSize, msgMdt, master, MSG_PM_TAG, *comm, request);
                                }
                            }
                        }
                    }
                }
            }
        }
    }    
}

