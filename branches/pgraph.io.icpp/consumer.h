#ifndef CONSUMER_H_
#define CONSUMER_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "mpi.h"
#include "type.h"
#include "timer.h"
#include "dtype.h"
#include "cfg.h"
#include "buffer.h"
#include "loadseq.h"
#include "dynamic.h"

#define MAX_NAME_LEN 100

void consumer(int master, int groupID, int gSize, int pdSize, int rank, char *seqFile, int nSeqs, char *cfgFile, char *outPath,
                 MPI_Datatype msgMdt, MPI_Comm *comm);

#endif /* end of consumer.h */
