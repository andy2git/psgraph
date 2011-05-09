/*
 * $Rev: 806 $ 
 * $Date: 2010-09-12 12:57:22 -0700 (Sun, 12 Sep 2010) $ 
 * $Author: Andy $
 *
 * Copyright 2010 Washington State University. All rights reserved.
 * ----------------------------------------------------------------
 *
 */

#include "forest.h"

/* ----------------------------------------------------------------*
 * load forest in a batched way, and return 0 when no stree left.
 * 
 * @param fp - file pointer
 * @param forest - forest needs to be filled in
 * @param frSize - forest size needs to be processed once
 * @param sIndex - start index of strees
 * @param tIndex - indicate where fp was left last time
 * 
 * @return #strees loaded in
 * 
 * ----------------------------------------------------------------*/
int loadForest(FILE *fp, FOREST *forest, int frSize, char *line, int sIndex, int *tIndex){
    int fIndex = -1;
    int eIndex = sIndex + frSize - 1;

    /* allocate it on heap to save valuable stack space */
    int stSize;
    int stIndex = -1;
    int sfIndex = 0;
    int sfSize;
    int m;   /* index for lset */
    char *tmp;
    int strLen;

    STNODE *st = NULL;
    SUFFIX *sf = NULL;


    while(fgets(line, STREE_MAX_LEN, fp)){
        strLen = strlen(line);
        line[strLen - 1] = '\0';
        assert(strLen < STREE_MAX_LEN);

        if(line[0] == '<'){

            (*tIndex)++;
            if(*tIndex >= sIndex && *tIndex <= eIndex){
                sscanf(line, "<stSize: %d, bktSize: %d\n", &stSize, &sfSize);
                fIndex++;
                forest[fIndex].stree = ecalloc(stSize, sizeof *(forest[fIndex].stree));
                forest[fIndex].sf = emalloc(sfSize*(sizeof *(forest[fIndex].sf)));
                forest[fIndex].stSize = stSize;
                forest[fIndex].maxDepth = 0;
            
                stIndex = -1;
                sfIndex = 0;
            }else if (*tIndex < sIndex){
                continue;
            }else if (*tIndex > eIndex){
                fseek(fp, strLen*(-1), SEEK_CUR);
                (*tIndex)--;
                break;
            }
        }
        
        if(*tIndex >= sIndex && *tIndex <= eIndex){
            if(line[0] == '=') {
                stIndex++;
                st = &(forest[fIndex].stree[stIndex]);
                sscanf(line, "=%d, %d\n", &(st->depth), &(st->rLeaf));

                if(st->depth > forest[fIndex].maxDepth){
                    forest[fIndex].maxDepth = st->depth;
                }
            }

            if(isdigit(line[0])){
                m = atoi(strtok(line, " "));
                while((tmp = strtok(NULL, " "))){
                    st = &(forest[fIndex].stree[stIndex]);
                    sf = &(forest[fIndex].sf[sfIndex]);
                    sscanf(tmp, "[%d,%d]", &(sf->sid), &(sf->pid));

                    /* prefix at the lset[m] */
                    sf->next = st->lset[m];
                    st->lset[m] = sf;
                    sfIndex++;
                }
            }
        }else if (*tIndex > eIndex){ /* no need to check further */
            break;
        }
    }

    if(sfIndex != 0 && stIndex != -1){
        assert(sfIndex == sfSize);
        assert(stIndex == stSize - 1);
    }


    return (fIndex+1);
} 

/* ----------------------------------------------------------------*
 * process forest tree by tree
 * 
 * @param frFile - forest file storing strees 
 * @param cfgFile - configure file for parameters
 * @param nSeqs - total #seqs, used for duplicatation reduction
 * @param pBuf - pair buffer
 * @param pBufSize - pair buffer size
 *
 * ----------------------------------------------------------------*/
void processForest(int groupID, int master, char *frFile, char *cfgFile, int nSeqs, int *dup, char *line, PBUF *pBuf, int pBufSize, int *isStart, 
                        MSG *chunk, int chunkSize, double *iTime, double *aTime, MPI_Request *request, MPI_Datatype msgMdt, MPI_Comm *comm){

    int frSize = getCfgVal(cfgFile, "PD_ForestSize");
    int exactMatch = getCfgVal(cfgFile, "ExactMatchLen");
    FOREST forest[frSize];
    int sIndex = 0;
    int cnt = 0;
    FILE *fp = NULL;
    int tIndex = -1;
    int *srtIndex = NULL;  /* sorted array based on stnode.depth */
    int i;
    double t1, t2;
    double t3, t4;
    u64 nPairs;

    t3 = cTime();

    fp = efopen(frFile, "r");

    while(1){
        t1 = cTime();
        cnt = loadForest(fp, forest, frSize, line, sIndex, &tIndex);
        t2 = cTime();

        if(cnt == 0) break; /* break of while(1) loop */

        printf("Group[%d] - PD: %d strees loaded from <%s> in <%.2lf> secs, cnt=%d\n", groupID, cnt, frFile, (t2-t1), cnt);

        for(i = 0; i < cnt; i++){
            srtIndex = emalloc(forest[i].stSize*(sizeof *srtIndex));

            t1 = cTime();
            countSort(forest[i].stree, srtIndex, forest[i].stSize, forest[i].maxDepth+1);
            t2 = cTime();
            printf("Group[%d] - PD: stNodes sorted in <%.2lf> secs\n", groupID, (t2-t1));

            t1 = cTime();
            nPairs = genPairs(master, forest[i].stree, srtIndex, forest[i].stSize, nSeqs, exactMatch, 
                            dup, pBuf, pBufSize, chunk, chunkSize, isStart, iTime, aTime, msgMdt, request, comm);
            t2 = cTime();

            printf("Group[%d] - PD: <%llu> pairs generated from <%s> in <%.2lf>secs\n", groupID, nPairs, frFile, (t2-t1));
            

            free(srtIndex);
        }

        freeForest(forest, cnt);
        sIndex += cnt;
    }

    fclose(fp);

    t4 = cTime();

    printf("Group[%d] - PD: DONE process <%s> in <%.2lf> secs\n", groupID, frFile, (t4-t3));

}


/* ----------------------------------------------------------------*
 * free memory allocated for forest.stree/suffix
 *
 * @param forest - forest pointers
 * @param frSize - forest size
 *
 * ----------------------------------------------------------------*/

void freeForest(FOREST *forest, int frSize){
    int i;

    for(i = 0; i < frSize; i++){
        free(forest[i].stree);
        free(forest[i].sf);
    }
}

