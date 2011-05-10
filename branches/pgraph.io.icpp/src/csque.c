#include "csque.h"

/* ---------------------------------------------------*
 * initilize consumer queue
 * 
 * @param cs - consumers
 * @param mIn - subgroup index
 *   
 *    ----|*****|---|*****|---|*****|---|*****|--- CS
 *          sb0       sb1       sb2       sb3
 *         mIn[0]    mIn[1]    mIn[2]    mIn[3]
 * 
 * @param nGroup - #(subgroups)
 * @param gSize - group size
 * @param procs - total number of procs
 * ---------------------------------------------------*/
void initQue(CSM *cs, CSM **mIn, int nGroup, int gSize, int procs){
    int i;
    int j;
    int k;
    int sRank;
    int eRank;
    
    k = 0;

    /* init **mIn */
    for(i = 0; i < nGroup; i++){
        mIn[i] = NULL;
    }

    /* init consumers */
    for(i = 0; i < nGroup; i++){
        sRank = i*gSize + 2;
        eRank = (i+1)*gSize - 1;
        eRank = (eRank > (procs-2)) ? (procs-2) : eRank;

        for(j = sRank; j <= eRank; j++){
            cs[k].id = j;
            cs[k].next = mIn[i];
            mIn[i] = &(cs[k]);
            k++;
        }
    }

    assert(k == procs - 1 - 2*nGroup);
}

/* ----------------------------------------------------*
 * reassigned consumers of the retired master to other
 * groups in a ROUND-and-ROBIN way.
 * 
 * @param mIn - subgroup index
 * @param gSize - group size
 * @param nGroup - number of groups 
 * @param retired - the index of retired master node
 * @param msgMdt - MPI msg type
 * ----------------------------------------------------*/
void assignQue(CSM **mIn, int gSize, int nGroup, int retired, MPI_Datatype msgMdt){
    int i;
    int k;
    int cnt = 0;
    CSM *p = NULL;
    CSM *q = NULL;
    MSG cReq;
    int leftGroups = 0;
    

    /* #active groups */
    for(i = 0; i < nGroup; i++){
        if(mIn[i]) cnt++;
    }
    
    leftGroups = (nGroup/16 > 1)? nGroup/16 : 1;
    printf("SM: %d subgroups are going to be KEEPED!\n", leftGroups);

    /* NO need to re-merge subgroups at this point */
    if(cnt == leftGroups){
        printf("SM: ONLY <%d> subgroups left, consumers NOT be reassigned\n", cnt);
        return;
    }


    k = 0;
    for(p = mIn[retired]; p != NULL; ){
        q = p;
        p = p->next;
        while(k%nGroup == retired || mIn[k%nGroup] == NULL){
            k++;
        }
        
        k %= nGroup;

        cReq.tag = TAG_C;
        cReq.id1 = q->id;
        cReq.id2 = R_HALF;

        printf("SM: sending req <%d> to master%d!\n", q->id, k*gSize);
        MPI_Ssend(&cReq, 1, msgMdt, k*gSize, MSG_CM_TAG, MPI_COMM_WORLD);
        q->next = mIn[k];
        mIn[k] = q;
        k++;
    }

    mIn[retired] = NULL;
    return;
}


void printQue(CSM **mIn, int nGroup){
    int i;
    CSM *p = NULL;
    
    for(i = 0; i < nGroup; i++){
        printf("mIn[%d]:", i);
        for(p = mIn[i]; p != NULL; p = p->next){
            printf("=>[%d]", p->id);
        } 
        printf("=>NULL\n");
    }
}

