/*
 * $Rev$ 
 * $Date$ 
 * $Author$
 *
 * Copyright 2010 Washington State University. All rights reserved.
 * ----------------------------------------------------------------
 *
 */

#include "loadseq.h"

/* -----------------------------------------------------------------------*
 * cache part of seqs according to its rank
 *
 * @param rank - local rank 
 * @param groupSize - gourp size
 * @param sRank - starting rank of consumer
 * @param seqFile - fasta sequence file
 * @param seqs - fasta seqs structure 
 * @param nSeqs - total #sequences in fasta file
 *
 * @returns #seqs loaded locally
 * -----------------------------------------------------------------------*/

int loadStaticSeqs(int rank, int sRank, char *seqFile, SEQ *seqs, int nSeqs, int maxSeqRange, int *maxSeqLen){
    FILE *fp = NULL;
    char *line = NULL;
    int strLen;

    int ind;        /* seq. ind across the fasta file */
    int sInd;       /* starting seq. index */
    int eInd;       /* ending seq. index */
    

    sInd = maxSeqRange*(rank - sRank);
    eInd = sInd + maxSeqRange - 1;
    eInd = (eInd > nSeqs-1) ? (nSeqs-1) : eInd;

    printf("rank=%d, sInd=%d, eInd=%d\n", rank, sInd, eInd);

    fp = efopen(seqFile, "r");

    ind = 0;
    *maxSeqLen = 0;
    line = emalloc(MAX_FASTA_LINE_LEN*(sizeof *line));
    while(fgets(line, MAX_FASTA_LINE_LEN, fp)){

        /* skip fasta file header */
        if(line[0] == FASTA_TAG) continue;

        strLen = strlen(line) - 1 ; 
        assert(strLen < MAX_FASTA_LINE_LEN);
        line[strLen] = '\0';

        if(*maxSeqLen < strLen) *maxSeqLen = strLen;

        if(ind >= sInd && ind <= eInd){
            seqs[ind].stat = SEQ_S;
            seqs[ind].str = estrdup(line);
            seqs[ind].strLen = strLen;
            seqs[ind].cnt = 0;
        }else{
            seqs[ind].stat = SEQ_N;
            seqs[ind].str = NULL;
            seqs[ind].strLen = strLen;
            seqs[ind].cnt = 0;
        }
        ind++;
    }

    free(line);
    fclose(fp);

    return (eInd - sInd + 1);
}

/* -----------------------------------------------------------------------*
 * free seqs according to passing tag to release some memory
 *
 * @param seqs - 
 * @param nSeqs - #seqs
 * @param tag - mark which seqs need to be released
 * -----------------------------------------------------------------------*/
void freeSeqs(SEQ *seqs, int nSeqs, int tag){
    int i;
    
    /* free dynamically fetched seqs */
    if(tag == 0){
        for(i = 0; i < nSeqs; i++){
            if(seqs[i].stat == SEQ_F && seqs[i].cnt == 0){
                free(seqs[i].str);
                seqs[i].stat = SEQ_N;
            }
        }
    /* free statically cached seqs */
    }else if(tag == 1){
        for(i = 0; i < nSeqs; i++){
            if(seqs[i].stat == SEQ_S) free(seqs[i].str);
        }
    }
}


