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

#include "../SatSolverInterface.h"
#include "SimpSolver.h"
//#include "SimpSolver.cc"

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



MinisatSolver::MinisatSolver(Cnf* cnf) {
    verbosity=0;
    while (cnf->vc > nVars()) newVar();
    vec<Lit> lits;
    int var;
    for (int i=0; i<cnf->cc; i++) {
      lits.clear();
      for (int j=0; j<cnf->cl[i]; j++) {
        var = abs(cnf->clauses[i][j])-1;
        while (var >= nVars()) newVar(); // just to be sure
        lits.push((cnf->clauses[i][j] > 0) ? mkLit(var) : ~mkLit(var));
      }
      addClause_(lits);
    }
    //eliminate(true);
  }

MinisatSolver::~MinisatSolver() {}

int MinisatSolver::run() {return solve(false);}
void MinisatSolver::load_into_message(Message* m, RangeSet &r) {
    m->assignments.clear();
    for (auto var = r.buffer.begin(); var != r.buffer.end(); var++) {
      for (int variable = (*var).first; variable <= (*var).second; variable++) {
        if (variable <= nVars()) {
          if (model[variable-1]!=l_Undef) {
            if (model[variable-1]==l_True) {
              m->assignments.push_back(variable);
            } else {
              m->assignments.push_back(-variable);
            }
          }
        }
      }
    }
  };
bool MinisatSolver::is_solver_unit_contradiction() {return false;};
bool MinisatSolver::reset_solver() {return true;}
void MinisatSolver::solver_add_conflict_clause(std::deque<int> d) {
    vec<Lit> lits;
    lits.clear();
    for (auto it = d.begin(); it!=d.end(); it++) {
      int lit = *it;
      int abs_lit = abs(lit);
      while (abs_lit>=nVars()) newVar();
      lits.push((lit > 0) ? mkLit(abs_lit) : ~mkLit(abs_lit));
    }
    addClause_(lits);
  }
void MinisatSolver::load_into_deque(deque<int> &d, RangeSet &r) {
    d.clear();
    for (auto var = r.buffer.begin(); var != r.buffer.end(); var++) {
      for (int variable = (*var).first; variable <= (*var).second; variable++) {
        if (variable <= nVars()) {
          if (model[variable-1]!=l_Undef) {
            if (model[variable-1]==l_True) {
              d.push_back(variable);
            } else {
              d.push_back(-variable);
            }
          }
        }
      }
    }
  };


