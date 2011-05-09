#ifndef PRODUCER_H_
#define PRODUCER_H_

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <semaphore.h>

#include "omp.h"
#include "mpi.h"
#include "type.h"
#include "dtype.h"
#include "elib.h"
#include "lib.h"
#include "loadseq.h"
#include "bucket.h"
#include "buffer.h"


int producer(int key, int master, int procs, int ntseqs, int k, MPI_Comm *comm);

#endif /* end of producer.h */
