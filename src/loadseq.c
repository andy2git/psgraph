#include "loadseq.h"

/* -----------------------------------------------------------------------*
 * cache fasta sequences into *seqs wrt *idList
 *
 * @param seqFile - fasta sequence file
 * @param nSeqs - total #sequences in fasta file
 * @param idList - an array indicating which seqs need to be loaded
 * @param seqs - sequence struct array
 * @param seqSize - #(seqs) to be loaded in *seqs
 *
 * -----------------------------------------------------------------------*/
int cacheSeqs(char *seqFile, SEQ *seqs, int nSeqs, int *idList, int seqSize, int msl){
    char *line = NULL;
    int strLen;
    int index = -1;
    int flag = 0;
    int seqCnt = 0;
    FILE *fp = NULL;
    int maxSeqLen = 0;

    fp = efopen(seqFile, "r");
    line = emalloc(MAX_FASTA_LINE_LEN*(sizeof *line));

    while(fgets(line, MAX_FASTA_LINE_LEN, fp)){
        strLen = strlen(line);
        assert(strLen < MAX_FASTA_LINE_LEN);

        line[strLen-1] = '\0';
        if(msl == 1){
            if(strLen > maxSeqLen) maxSeqLen = strLen;
        }
        
        if(line[0] == FASTA_FLAG){
            index++;
            if(idList[index] == 1){
                flag = 1;
            }

        } else if(flag == 1 && isalpha(line[0])){
            seqs[index].strLen = strLen - 1; 
            seqs[index].str = estrdup(line);
            seqCnt++;

            /* loading finished here, no need to continue */
            if(seqCnt == seqSize && msl == 0) break;
            flag = 0;
        }
    }

    free(line);
    fclose(fp);

    return maxSeqLen;
}



/* ---------------------------------------------------*
 * free memory used by *seqs
 * 
 * @param seqs - sequence structure
 * @param ids - free seq[i] iff (ids[i] == 1)
 * @param nSeqs - #(seqs)
 *
 * ---------------------------------------------------*/
void freeSeqs(SEQ *seqs, int *ids, int nSeqs){
    int i;
    int cnt = 0;
    
    for(i = 0; i < nSeqs; i++){
        if(ids[i] == 1){
            cnt++;
            free(seqs[i].str);
            ids[i] = 0;
        }
    }

}


/* ---------------------------------------------------*
 * generate random *ids for loading seqs
 *
 * @param ids - which elems need to be loaded
 * @param idSize - the size of *ids
 * @param idCnt - #(elems) needs to be filled
 * 
 * ---------------------------------------------------*/
void randIds(int *ids, int idSize, int idCnt){
    int i; 
    int r;
    int j;
    
    /* need to cache all seqs */
    if(idCnt >= idSize){
        for(i = 0; i < idSize; i++)
            ids[i] = 1;

        return;
    }

    /* init all ids */
    for(i = 0; i < idSize; i++){
        ids[i] = 0;
    }

    for(i = 0; i < idCnt; i++){
        r = rand()%idSize;
        if(ids[r] == 0) ids[r] = 1;
        else{
            /* linear probing to find first empty spot */
            for(j = 0; j < idSize; j++){

                /* find first spot */
                if(ids[j] == 0){
                    ids[j] = 1;
                    break; 
                }
            }
        }
    }
}

/* -------------------------------------------------------------*
 * check against statically cached seqs in *cIds, then mark down
 * which seqs are required to be loaded from disk. 
 * 
 * NOTE: every time check only min(batch, pBuf->data) in pBuf
 *
 * @param cIds - a map indicating statically cached seqs
 * @param dIds - a map indicating the dynamically required seqs
 * @param nSeqs - total #(fasta seqs)
 * @param pBuf - pair buf 
 * @param pBufSize - pair buffer size
 * @param batch - expected batch size need to check *pBuf
 *
 * -------------------------------------------------------------*/
int filterIds(int *cIds, int *dIds, int nSeqs, PBUF *pBuf, int pBufSize, int batch){
    int i;
    int tail;
    int elems;

    /* init dIds into 0 */
    for(i = 0; i < nSeqs; i++){
        dIds[i] = 0;
    }

    elems = (pBuf->data >= batch)? batch : pBuf->data;

    tail = pBuf->tail;
    for(i = 0; i < elems; i++){
        if(pBuf->buf[tail].tag == TAG_E) continue;
        if(pBuf->buf[tail].tag == TAG_S) continue;

        if(cIds[pBuf->buf[tail].id1] == 0) dIds[pBuf->buf[tail].id1] = 1;
        if(cIds[pBuf->buf[tail].id2] == 0) dIds[pBuf->buf[tail].id2] = 1;

        tail++;
        tail %= pBufSize;
    }
    
    int cnt = 0;
    for(i = 0; i < nSeqs; i++){
        if(dIds[i] == 1) cnt++;
    }

    return cnt;
}
