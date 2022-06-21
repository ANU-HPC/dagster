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
#include "mpi_global.h"
#include "../Cnf.h"
#include "../CnfHolder.h"
#include "../mpi_global.h"

#if defined(__linux__)
#include <fpu_control.h>
#endif
#include"core/Solver.h"

using namespace Minisat;

int minisat_surrogate_main(MPI_Comm *communicator, CnfHolder* cnf_holder) {

  return 0;
}



void load_in_Cnf(Solver* S, Cnf* cnf) {
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



int minisat_run_strengthener(MPI_Comm *communicator, int phase, MPI_Request *next_cnf_req, Cnf* cnf) {


  Solver* S = new Solver();

  S->verbosity = 0;
  load_in_Cnf(S, cnf);
  //S->simplify();
  //S->runReducingLoop(*communicator, phase, next_cnf_req); // Enter main loop for reducing
  delete S;
  return 0;
}
