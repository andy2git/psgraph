#include "loadseq.h"

/*---------------------------------------------------*
 * Load partationed seqs into individual procs
 *
 * @param rank - procs rank
 * @param seqs - seqs 
 * @param nseqs - total number of given input seqs
 * @param procs - #procs in this communicator
 * @param fastaFile - Fasta file name
 * 
 * @return 
 *      #seqs loaded into <rank>
 *      #AA for <rank> 
 *---------------------------------------------------*/

int loadSeqs(int rank, SEQ **seqs, int ntseqs, int procs, char *fastaFile, ulong *AA, int *asLine){
    int batch;
    int sLine = 0;
    int eLine = 0;
    int cLine = -1;  /* current fasta file index */

    FILE *fp = NULL;
    char line[FASTA_MAX_LEN];
    int lineLen = 0;
    int flag = 0;    /* indicate if the fasta seq is valid for read */
    int index = 0;


    batch = (ntseqs%procs == 0) ? ntseqs/procs : (ntseqs/procs+1);

    sLine = rank * batch;
    eLine = (rank+1) * batch - 1;
    if(eLine >= ntseqs) eLine = ntseqs - 1;

    /* #seqs to be loaded on this proc */
    *seqs = emalloc((eLine-sLine+1)*(sizeof **seqs));

    /* load seqs */
    fp = efopen(fastaFile, "r");

    *AA = 0L;
    while(fgets(line, FASTA_MAX_LEN, fp)){
        lineLen = strlen(line);
        assert(lineLen < FASTA_MAX_LEN);
        
        if(line[0] == FASTA_HEAD_FLAG){
            cLine++;
            if(cLine >= sLine && cLine <= eLine){
                (*seqs)[index].gid = estrdup(line);
                flag = 1;
            }else{
                continue;
            }
        }else if(flag == 1 && isalpha(line[0])){
            line[lineLen - 1] = DOLLAR;  /* replace '\n" w/ 'U' */
            line[lineLen] = '\0';
            *AA += lineLen; 
    
            /* strLen includes the '$' */
            (*seqs)[index].strLen = lineLen; 
            (*seqs)[index++].str = estrdup(line);
            flag = 0;
        }
    }

    fclose(fp);

    assert(index == (eLine-sLine+1));
    /* start line offset for proc <rank> */
    *asLine = sLine;
    return index;
}


void freeSeqs(SEQ **seqs, size_t size){
    int i;
    for(i = 0; i < size; i++){
        efree((*seqs)[i].gid);
        efree((*seqs)[i].str);
    }

    efree(*seqs);
}


void printSeq(SEQ *seqs, int index){
    printf("-------Seqs[%3d]------\n", index);
    printf("%s - %d\n%s\n", seqs[index].gid, seqs[index].strLen, seqs[index].str);
}
