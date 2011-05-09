#ifndef SP_MASTER_H_
#define SP_MASTER_H_

#include <stdlib.h>
#include <stdio.h>

#include "mpi.h"
#include "type.h"
#include "timer.h"
#include "buffer.h"
#include "queue.h"
#include "cfg.h"
#include "csque.h"

void spMaster(int gSize, int pdSize, int nGroup, int procs, char *cfgFile, MPI_Datatype msgMdt, int nFiles);

#endif /* end of spmaster.h */
