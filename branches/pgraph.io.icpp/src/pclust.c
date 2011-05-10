#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "mpi.h"
#include "type.h"
#include "timer.h"
#include "cfg.h"
#include "producer.h"
#include "master.h"
#include "consumer.h"
#include "spmaster.h"


static char gCfgFile[MAX_FILENAME_LEN];
static char gForestPath[MAX_FILENAME_LEN];
static char gOutPath[MAX_FILENAME_LEN];
static char gSeqFile[MAX_FILENAME_LEN];
static int gN;   /* #(fasta seqs) */
static int gF;   /* #(tree files) */
static int groupSize;  /* #producers */
static int pdSize;

int optCK(int rank, int argc, char **argv);

int main(int argc, char **argv){
    int gRank;
    int procs;
    int groupID;        /* control of subset assignment */
    int key;            /* control of rank assignment */
    MPI_Comm intraComm; /* subgroup communicator */
    int lm; /* local master rank */
    int sp; /* supermaster rank */

    MPI_Datatype msgMdt;
    MSG msgType;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &gRank);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);


    /* no buffer for stdout */
    //setbuf(stdout, (char *)0);
    //setbuf(stderr, (char *)0);

    if(optCK(gRank, argc, argv) != 7){
        if(gRank == 0){
            printf("Usage : %s -f {forest path} -s {fasta} -n {#seqs} -c {cfg file} -g {group size} -p {pdSize} -o {output path}\n", argv[0]);
        }
	    exit(-1);
    }

    /* NOTE: init blosum matrix for alignment */
    initMap(SIGMA);

    /* TODO param read from cmd line */
    int nGroup;
    
    /* TODO: need to check a group has at least 3 procs */
    nGroup = procs/groupSize;

    if(gRank == 0){
        printf("---------------------------------------------------\n");
        printf("%-15s: %.35s\n", "forest path", gForestPath);
        printf("%-15s: %.35s\n", "fasta seq", gSeqFile);
        printf("%-15s: %d\n", "#seqs", gN);
        printf("%-15s: %.35s\n", "config file", gCfgFile);
        printf("%-15s: %d\n", "#groupSize", groupSize);
        printf("%-15s: %d\n", "#groups", nGroup);
        printf("---------------------------------------------------\n");

        if(procs%groupSize > 0 && procs%groupSize < 4){
            printf("wrong group Size and procs ratio\n");
        }
    }

    double t1, t2;
    t1 = cTime();

    build_msg_type(&msgType, &msgMdt);


    /* procs-1 is supermaster node, also color 0
     * is reserved for supermaster */
    if(gRank != procs -1){
        groupID = gRank/groupSize + 1;
        key = gRank%groupSize; 
    }else{
        groupID = 0;
        key = 0;
    }
    MPI_Comm_split(MPI_COMM_WORLD, groupID, key, &intraComm);


    if(gRank%groupSize == 0){ /* master node */
        lm = 0;
        master(lm, groupID, groupSize, pdSize, procs, gN, gCfgFile, msgMdt, &intraComm);
        t2 = cTime();
        printf("Master%d: finished in <%.2lf> secs\n", gRank, t2-t1);
    }else if(gRank%groupSize >= 1 && gRank%groupSize <= pdSize){
        sp = procs - 1;
        lm = 0;
        producer(lm, sp, groupID, gForestPath, gCfgFile, gN, msgMdt, &intraComm);
        t2 = cTime();
        printf("PD%d: finished in <%.2lf> secs\n", gRank, t2-t1);
    }else if(gRank == procs-1){
        gF = getCfgVal(gCfgFile, "Forest_Files");
        spMaster(groupSize, pdSize, nGroup, procs, gCfgFile, msgMdt, gF);
        t2 = cTime();
        printf("SM%d: finished in <%.2lf> secs\n", gRank, t2-t1);
    }else{
        lm = 0;
        if(groupID >= nGroup) groupSize -= 1;
        consumer(lm, groupID, groupSize, pdSize, key, gSeqFile, gN, gCfgFile, gOutPath, msgMdt, &intraComm);
        t2 = cTime();
        printf("CS%d: finished in <%.2lf> secs\n", gRank, t2-t1);
    }


    MPI_Type_free(&msgMdt);
    MPI_Finalize();
    exit(EXIT_SUCCESS);
}


int optCK(int rank, int argc, char **argv){
	int option;
	int cnt = 0;
	
	while(-1 != (option = getopt(argc, argv, "f:s:n:c:g:p:o:"))){
		switch (option){
		    case '?':
                if(rank == 0){
                    printf("Usage : %s -f {forest path} -s {fasta} -n {#seqs} -c {cfg file} -g {group size} -p {pdSize} -o {output path}\n", argv[0]);
                }
			    exit(-1);
		    case 'f':
			    strcpy(gForestPath, optarg);
			    cnt++;
			    break;
		    case 's':
			    strcpy(gSeqFile, optarg);
			    cnt++;
			    break;
		    case 'n':
                gN = atoi(optarg);
			    cnt++;
			    break;
            case 'c':
                strcpy(gCfgFile, optarg);
                cnt++;
                break;
            case 'g':
                groupSize = atoi(optarg);
                cnt++;
                break;
            case 'p':
                pdSize = atoi(optarg);
                cnt++;
                break;
            case 'o':
                strcpy(gOutPath, optarg);
                cnt++;
                break;
		}
	}
	return cnt;
}
