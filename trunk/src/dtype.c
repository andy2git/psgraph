/*
 * $Rev$ 
 * $Date$ 
 * $Author$
 *
 * Copyright 2010 Washington State University. All rights reserved.
 * ----------------------------------------------------------------
 * This file will build derived datatype for mpi environment!
 *
 */

#include "dtype.h"

/* ------------------------------------------*
 *  Construct MPI_Datatype
 *
 *  typedef struct msg{
 *      char tag;
 *      int id1;    -- rank & s1
 *      int id2;    -- status & s2 
 *  }MSG;
 * 
 * ------------------------------------------*/

void build_msg_type(MSG *msg, MPI_Datatype *mdt){
    int size = 3;
    int i;

    MPI_Datatype type[] = {MPI_CHAR, MPI_INT, MPI_INT};
    int blockLen[] = {1, 1, 1};
    MPI_Aint disp[size];


    MPI_Address(&msg->tag, &disp[0]);
    MPI_Address(&msg->id1, &disp[1]);
    MPI_Address(&msg->id2, &disp[2]);
    
    for(i = 2; i >= 0; i--)
        disp[i] -= disp[0];

    MPI_Type_struct(size, blockLen, disp, type, mdt);
    MPI_Type_commit(mdt);

}
