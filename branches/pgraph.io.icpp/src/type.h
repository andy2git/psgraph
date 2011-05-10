#ifndef TYPE_H_
#define TYPE_H_

#define COLOR_PD 0
#define COLOR_SM 1
#define COLOR_MS 2
#define COLOR_CS 3

#define MSG_PM_TAG 1
#define MSG_MC_TAG 2
#define MSG_CM_TAG 3
#define MSG_SM_TAG 4
#define MSG_PS_TAG 5
#define MSG_SP_TAG 6


/* for consumer request */
#define R_SIZE 3
#define R_HALF 2
#define R_QUAT 1
#define R_NONE 0


#define DOLLAR 'U' /* dollar delimiter */
#define BEGIN 'O'  /* it is oh, not zero */
#define SIGMA 26   /* size of alphabet, '*'->'J' */


#define TAG_P 'P'  /* pair workload from producer */
#define TAG_C 'C'  /* Job request from consumer */
#define TAG_E 'E'  /* ending of pairs */
#define TAG_S 'S'  /* stop signal for program */
#define TAG_T 'T'  /* ending signal of subgroup */
#define TAG_F 'F'  /* end signal for tree file */

#define MAX_FILENAME_LEN 200
#define FILE_STOP -1


typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned long long u64;
enum {NO = 0, YES = 1};
enum {FALSE = 0, TRUE = 1};

typedef struct msg {
    char tag;    /* 'P', 'C', 'E' or 'A' */
    int id1;     /* s1 & rank */
    int id2;     /* s2 & status */
}MSG;

typedef struct pbuf{
    int head;
    int tail;
    int data;
    struct msg *buf;
}PBUF;

typedef struct req{
    int rank;
    struct req *next;
}REQ;

typedef struct cReq{
    struct req *queue;
    struct req *stMem; /* pre-allocated static mem */
}CREQ;

/* consumers struct on super master */
typedef struct csm{
    int id;
    struct csm *next;
}CSM;


/**
 * lookup table, used to represent suffix of 
 * every sequences
 */
typedef struct suff{
    int sid;           /* string id */
    int pid;           /* position id */
    struct suff *next; /* ptr to next suff */
}SUFFIX;

/**
 * suffix tree node
 */
typedef struct stnode{
    int depth;                   /* depth since the root, not including the initial size k */
    int rLeaf;                  /* right most leaf index */
    struct suff *lset[SIGMA];    /* subtree's nodes branched according to left characters */
}STNODE;

typedef struct forest{
    int stSize;
    int maxDepth;         /* maxium depth of stree used for couting sort */
    struct stnode *stree;
    struct suff *sf; 
}FOREST;

/**
 * seq info for fasta file
 */
typedef struct seq{
    char *str;  /* actual string of fasta file */
    int strLen; /* string length */
}SEQ;

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
    int AOL;             /* AlignOverLongerSeq */
    int SIM;             /* MatchSimilarity */
    int OS;              /* OptimalScoreOverSelfScore */
}PARAM;

/**
 * statistics packed in struct 
 */
typedef struct work{
    int nAlign;
    int nPairs;
}WORK;

#endif /* end of type.h */
