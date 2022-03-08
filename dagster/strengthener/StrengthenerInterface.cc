/*************************
Copyright 2021 Marshall Cliffton

This file is part of Dagster.

Dagster is free software; you can redistribute it 
and/or modify it under the terms of the GNU General 
Public License as published by the Free Software 
Foundation; either version 2 of the License, or
(at your option) any later version.

Dagster is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR 
A PARTICULAR PURPOSE. See the GNU General Public 
License for more details.

You should have received a copy of the GNU General 
Public License along with Dagster.
If not, see <http://www.gnu.org/licenses/>.
*************************/


#include "StrengthenerInterface.h"
#include "SolRed.h"
#include "mpi_global.h"
#include "../Cnf.h"
#include "../CnfHolder.h"
#include "../mpi_global.h"

#if defined(__linux__)
#include <fpu_control.h>
#endif

#ifdef MINIRED
using namespace MiniRed;
#elif defined GLUCORED
using namespace GlucoRed;
#endif

int strengthener_surrogate_main(MPI_Comm *communicator, CnfHolder* cnf_holder) {
  // get filename, additional clauses from master
  // start up reducer with this information, rank and tags
  // TODO manage communicating a shutdown signal (look at how it is done in
  // the SLS process

  int phase = 0;
  // Get a new CNF file from SatHandler
  int fname_length;
  MPI_Status mpi_status;
  MPI_Recv(&fname_length, 1 /*Number of ints*/, MPI_INT, 0 /*CDCL process ID*/, CNF_FILENAME_LENGTH_TAG, *communicator, &mpi_status);
  Message* m = NULL;
  Cnf* cnf = NULL;
  int* buffer = NULL;
  while (fname_length > 0) {
    // start waiting for the next CNF filename length to be sent
    // (a length of zero is the signal to kill this process)
    if (buffer!=NULL)
      free(buffer);
    TEST_NOT_NULL(buffer = (int*)calloc(sizeof(int), fname_length))
    if (m!=NULL)
      delete m;
    if (cnf!=NULL)
      delete cnf;
    MPI_Recv(buffer, fname_length, MPI_INT, 0, CNF_FILENAME_TAG, *communicator, &mpi_status);
    m = new Message(buffer);
    cnf = cnf_holder->compile_Cnf_from_Message(m);
    MPI_Request next_cnf_req;
    MPI_Recv_init(&fname_length, 1, MPI_INT, 0, CNF_FILENAME_LENGTH_TAG, *communicator, &next_cnf_req);
    MPI_Start(&next_cnf_req);
    run_strengthener(communicator, phase++, &next_cnf_req, cnf);
  }

  if (buffer!=NULL)
    free(buffer);
  if (m!=NULL)
    delete m;
  if (cnf!=NULL)
    delete cnf;
  return 0;
}



void load_in_Cnf(SolRed* S, Cnf* cnf) {
  while (cnf->vc > S->nVars()) S->newVar();
  vec<Lit> lits;
  int var;
  for (int i=0; i<cnf->cc; i++) {
    lits.clear();
    for (int j=0; j<cnf->cl[i]; j++) {
      var = abs(cnf->clauses[i][j])-1;
      while (var >= S->nVars()) S->newVar(); // just to be sure
      lits.push((cnf->clauses[i][j] > 0) ? mkLit(var) : ~mkLit(var));
    }
    S->addClause_(lits);
  }
}



int run_strengthener(MPI_Comm *communicator, int phase, MPI_Request *next_cnf_req, Cnf* cnf) {
  //printf("\nc This is %s\n\n", VERSION_STRING);
  // SW130410 I'm not sure if changing the FPU precision makes a lot of sense
  // here given that MiniRed's operation will not be repeatable due to thread
  // scheduling. Let's leave it here for the sake of making minimal
  // modifications
#if defined(__linux__)
  fpu_control_t oldcw, newcw;
  _FPU_GETCW(oldcw);
  newcw = (oldcw & ~_FPU_EXTENDED) | _FPU_DOUBLE;
  _FPU_SETCW(newcw);
  // printf("WARNING: for repeatability, setting FPU to use double
  // precision\n");
#endif


  SolRed* S = new SolRed();

  S->verbosity = 0;
  load_in_Cnf(S, cnf);
  S->simplify();
  S->runReducingLoop(*communicator, phase, next_cnf_req); // Enter main loop for reducing
  delete S;
  return 0;
}
