#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/**
 * This program simulates a team of workers running cooperatively to solve
 * a SAT problem. The first worker is a standard DPLL instance (e.g. TiniSAT)
 * which asynchronously receives suggestions for variable assignments from the
 * remaining workers running SLS (with different parameter settings).
 * The DPLL worker occasionally checks for incoming advice and uses it to guide
 * future variable assignments.
 * (The DPLL worker should also send constraints to the SLS instances (e.g. when
 * it discovers UNSAT for some variable assignment), however, this is not
 * currently implemented.)
 */

#define MPI_RANK_MASTER 0
#define ITERS 10
#define NUM_VARS 100
#define TAG_SLS_SUGGEST 1
#define TAG_DPLL_CONSTRAINT 2
#define WORK_TIME 1200

// sleep for a random number of millseconds between 0 and maxTimeMillis
void pretendToWork(int maxTimeMillis) {
  int milliseconds = rand() % maxTimeMillis;
  struct timespec ts;
  ts.tv_sec = milliseconds / 1000;
  ts.tv_nsec = (milliseconds % 1000) * 1000000;
  nanosleep(&ts, NULL);
}

// run backtracking search given a fixed assignment of some variables 
void doDPLL(int rank, int nprocs) { 
  printf("rank %d is DPLL\n", rank);
  int numSLSInstances = nprocs-1;

  // IDs of variables that should be assigned (negative ID == negative assignment)
  int var[NUM_VARS];
  
  MPI_Request receiveReq;
  MPI_Irecv(&var, NUM_VARS, MPI_INT, MPI_ANY_SOURCE, TAG_SLS_SUGGEST, MPI_COMM_WORLD, &receiveReq);
  // run until we've received some number of suggestions from SLS instances
  // (in the real app: keep searching until SAT/UNSAT)
  int i = 0;
  while (i < ITERS * numSLSInstances) {
    // do 'some steps' of DPLL (i.e. some numbers of levels of assignment+propagation)
    pretendToWork(WORK_TIME / numSLSInstances);
   
    // check whether there are any incoming suggestions from SLS instances 
    int incoming;
    MPI_Status status;
    MPI_Test(&receiveReq, &incoming, &status);
    if (incoming != 0) {
      printf("DPLL received suggestion from %d: %d\n", status.MPI_SOURCE, var[0]);
      i += 1;

      // var now contains a new suggested assignment from some SLS instance
      // use it to guide future DPLL assignments

      MPI_Irecv(&var, NUM_VARS, MPI_INT, MPI_ANY_SOURCE, TAG_SLS_SUGGEST, MPI_COMM_WORLD, &receiveReq);
    } 
  }
  printf("Finshed! DPLL received %d suggestions in total\n", i);
}

// run stochastic local search to generate an assignment
void doSLS(int rank, int nprocs) {
  printf("rank %d is SLS\n", rank);

  MPI_Request sendReq;
  int i = 0;
  while (i < ITERS) {
    // run SLS for some number of steps
    pretendToWork(WORK_TIME);

    // make sure previous suggestions have been sent
    if (i > 0) MPI_Wait(&sendReq, MPI_STATUS_IGNORE);

    // now choose some suggested assignments, e.g. variables for which
    // the assignment has not changed for a long time
    int suggested[NUM_VARS];

    suggested[0] = i++;
    printf("SLS rank %d sending suggestion to DPLL: %d\n", rank, suggested[0]);
    MPI_Isend(&suggested, NUM_VARS, MPI_INT, MPI_RANK_MASTER, TAG_SLS_SUGGEST, MPI_COMM_WORLD, &sendReq);
  }
}

int main(int argc, char** argv) {
  MPI_Init(NULL, NULL);

  int nprocs;
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // initialize random number generator for this process
  srand(rank);

  if (rank == MPI_RANK_MASTER) {
    doDPLL(rank, nprocs);
  } else {
    doSLS(rank, nprocs);
  }

  // Finalize the MPI environment. No more MPI calls can be made after this
  MPI_Finalize();
}

