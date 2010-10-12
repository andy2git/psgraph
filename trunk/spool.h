#ifndef S_POOL_H_
#define S_POOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "type.h"
#include "elib.h"

void initSPool(SPOOL *sPool, int size, int maxStrSize);
void freeSPool(SPOOL *sPool, int size);
int deSPool(SPOOL *sPool, int size);

#endif /* end of spool.h */
