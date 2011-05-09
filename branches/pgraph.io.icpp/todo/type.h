#ifndef TYPE_H_
#define TYPE_H_

#include <semaphore.h>

#define DOLLAR 'U' /* dollar delimiter */
#define BEGIN 'O'  /* it is oh, not zero */
#define SIGMA 26   /* size of alphabet, '*'->'J' */


#define PRODUCER 0
#define MASTER   1
#define CONSUMER 2

#define PM_MSG_TAG 1
#define MC_MSG_TAG 2
#define CM_MSG_TAG 3

typedef unsigned int uint;
typedef unsigned long ulong;
enum {NO = 0, YES = 1};
enum {FALSE = 0, TRUE = 1};


/**
 * seq info for fasta file
 */
typedef struct seq{
    char *gid;  /* gid of fasta file */
    char *str;  /* actual string of fasta file */
    int strLen; /* string length */
}SEQ;


/**
 * lookup table, used to represent suffix of 
 * every sequences
 */
typedef struct suff{
    int sid;           /* string id */
    int pid;           /* position id */
    struct suff *next; /* ptr to next suff */
}SUFFIX;

/* struct for flatten suffix */
typedef struct stuple{
    int sid;
    int pid;
}STP;

/**
 * pair struct
 */
typedef struct pair{
    int s1;
    int s2;
}PAIR;


typedef struct pbuf{
    int head;
    int tail;
    int data;
    
    sem_t space;
    sem_t items;
    sem_t lock;
    struct pair *buf;
}PBUF;

/**
 * suffix tree node
 */
typedef struct stnode{
    int depth;                   /* depth since the root, not including the initial size k */
    int rLeaf;                   /* right most leaf index */
    struct suff *lset[SIGMA];    /* subtree's nodes branched according to left characters */
}STNODE;


/**
 * CELL for dynamic alignment
 */
typedef struct cell{
    int score;           /* alignment score */
    int ndig;            /* #(matches) */
    int alen;            /* alignment length */
}CELL;


/**
 * parameters packed in struct
 */
typedef struct param{
    int k;               /* slide window size */
    int AOL;             /* AlignOverLongerSeq */
    int SIM;             /* MatchSimilarity */
    int OS;              /* OptimalScoreOverSelfScore */
    int exactMatchLen;   /* exact Match Length cutoff */
}PARAM;

/**
 * statistics packed in struct 
 */
typedef struct work{
    int nAlign;
    int nPairs;
}WORK;

#endif /* end of type.h */
