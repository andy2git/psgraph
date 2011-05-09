#include "producer.h"

int producer(int key, int master, int procs, int ntseqs, int k, MPI_Comm *comm){
    SEQ *seqs = NULL;
    int nseqs;
    int sLine;
    ulong AA = 0;

    SUFFIX *sf = NULL;
    ulong sfSize;

    SUFFIX **bkt = NULL;
    int bktSize;
    int tid; 
    int nThreads = 1;
     
    PBUF pBuf;
    int pBufSize;
    

    /* supposed to read from *.cfg file */
    nThreads = 2; 
    pBufSize = 10;

    initBuf(&pBuf, pBufSize);
    omp_set_num_threads(nThreads);


    #pragma omp parallel private(tid)
    {
        tid = omp_get_thread_num();
        if(tid == 0){
            /*nseqs = loadSeqs(key, &seqs, ntseqs, procs, "/home/cwu2/pclust/test.dat", &AA, &sLine);
            printSeq(seqs, 0);
             
            sfSize = AA - nseqs*k;
            sf = emalloc(sfSize*(sizeof *sf));        

            bktSize = power(SIGMA, k);
            bkt = emalloc(bktSize*(sizeof *bkt));
            
            buildBkt(key, seqs, nseqs, sLine, bkt, bktSize, sf, sfSize, k);
            printBktList(bkt[0]); 

            STP *stp = NULL;
            ulong stpSize;

            stpSize = sfSize;
            stp = emalloc(stpSize*(sizeof *stp));
             
            int *stpCnt = NULL;
            int *allCnt = NULL;
            int cntSize = bktSize;

            stpCnt = emalloc(cntSize*(sizeof *stpCnt));
            allCnt = emalloc(cntSize*(sizeof *allCnt));
            suf2tuple(key, bkt, bktSize, stp, stpSize, stpCnt);

            free(sf);
            free(bkt);

            MPI_Allreduce(stpCnt, allCnt, cntSize, MPI_INTEGER, MPI_SUM, *comm);
            printf("allCnt[0]=%d\n", allCnt[0]);


            STP *ntp = NULL;
            int ntpSize;

            ntpSize = redistBkt(key, procs, allCnt, cntSize);
            ntp = emalloc(ntpSize*(sizeof *ntp));

            STP stpType;
            MPI_Datatype stpMdt;
            build_stp_type(&stpType, &stpMdt);        

            //int *sdisp = NULL;
            //int *rdisp = NULL;*/

            /* simulated work */
            int i;
            int cnt;
            PAIR pairs[5] = {{0,1}, {0,2}, {0,3}, {0,4}, {0,5}};
            for(i = 0; i < 2; i++){
                cnt = enBuf(&pBuf, pBufSize, pairs, 5);
                printf("t[%d,%d] writes %d in \n", tid, key, cnt);
            }


            /* free goes here */
            /*freeSeqs(&seqs, nseqs);
            free(stp);
            free(ntp);
            free(stpCnt);
            free(allCnt);*/
        }

        if(tid == 1){
            int cnt; 
            int i;
            PAIR pairs[5];             
            MPI_Datatype pairMdt;
            PAIR pairType;

            build_pair_type(&pairType, &pairMdt);
            for(i = 0; i < 2; i++){
                cnt = deBuf(&pBuf, pBufSize, pairs, 5);
                printf("t[%d,%d] reads out cnt=%d\n", tid, key, cnt);
                MPI_Send(pairs, 5, pairMdt, master, PM_MSG_TAG, MPI_COMM_WORLD);
            }
             
        }
    }

    return 1;
}
