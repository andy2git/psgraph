#ifndef S_POOL_H_
#define S_POOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "type.h"
#include "elib.h"

/* seq. str. stat */
#define STR_FREE 0
#define STR_TOGO 1
#define STR_SENT 2

/**
 * pool of str resource for sending strings
 */
typedef struct str{
    char stat;
    char *str;
    MPI_Request req;  /* associated with each *str */
}STR;

typedef struct spool{
    int space;
    struct str *pool;
}SPOOL;

void initSPool(SPOOL *sPool, int size, int maxStrSize);
void freeSPool(SPOOL *sPool, int size);
int deSPool(SPOOL *sPool, int size);

#endif /* end of spool.h */
