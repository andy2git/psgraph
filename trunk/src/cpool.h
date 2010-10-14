#ifndef CPOOL_H_
#define CPOOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "type.h"
#include "elib.h"


void initCPool(CPOOL *cPool, int size);
void freeCPool(CPOOL *cPool);
int deCPool(CPOOL *cPool, int size);

#endif /* end of cpool.h */
