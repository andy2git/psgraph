#ifndef MASTER_H_
#define MASTER_H_

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "mpi.h"
#include "type.h"
#include "timer.h"
#include "buffer.h"
#include "queue.h"
#include "cfg.h"

void master(int master, int groupID, int gSize, int pdSize, int procs, int nSeqs, char *cfgFile, MPI_Datatype msgMdt, MPI_Comm *comm);

#endif /* end of master.h */
