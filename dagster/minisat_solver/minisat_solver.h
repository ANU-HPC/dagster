/*************************
Copyright 2021 Mark Burgess

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


#ifndef MINISATSOLVER_WORKER_H
#define MINISATSOLVER_WORKER_H


#include <mpi.h>
#include <errno.h>

#include <signal.h>
#include <zlib.h>
#include "../Cnf.h"
#include "../CnfHolder.h"

#include "../SatSolverInterface.h"
#include "SimpSolver.h"


using namespace Minisat;


class MinisatSolver : public SatSolverInterface, public SimpSolver {
  public:
  Cnf* cnf;
  bool* mark2; // array used to mark variables relevent to the solution being processed, decided by function
  bool solver_unit_contradiction;
  vec<Lit> unit_assignments;
  
  bool prune_solution(Message* reference_message);
  
  MinisatSolver(Cnf* cnf);
  bool append_cnf(Cnf* cnf);
  int run(Message* m);
  void load_into_message(Message* m, RangeSet &r, Message* reference_message);
  bool is_solver_unit_contradiction();
  bool reset_solver(); // dont need to do anything, since minisat is incremental anyways??
  bool solver_add_conflict_clause(std::deque<int> d);
  ~MinisatSolver();
};


#endif
