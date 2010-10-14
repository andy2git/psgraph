#ifndef PRODUCER_H_
#define PRODUCER_H_

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include "type.h"
#include "elib.h"
#include "lib.h"
#include "cfg.h"
#include "buffer.h"
#include "forest.h"


int producer(int master, int spMaster, int groupID, char *frPath, char *cfgFile, int nSeqs, 
                MPI_Datatype msgMdt, MPI_Comm *comm);

#endif /* end of producer.h */
