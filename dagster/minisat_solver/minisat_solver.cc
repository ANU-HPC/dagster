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


#include "minisat_solver.h"
#include "mpi_global.h"
#include "../Cnf.h"
#include "../CnfHolder.h"
#include "../mpi_global.h"

#include"core/Solver.h"

#include "../SatSolverInterface.h"
#include "SimpSolver.h"

using namespace Minisat;


MinisatSolver::MinisatSolver(Cnf* cnf, int node) {
DB(printf("adding CNF to minisatsolver\n");
cnf->print();)
	this->use_elim = false;
	this->use_rcheck = false;
	this->cnf = cnf;
	this->node = node;
    verbosity=0;
    this->mark2 = NULL;
    add_cnf(cnf);
    //eliminate(true);
  }

MinisatSolver::~MinisatSolver() {
  free(this->mark2);
}


void MinisatSolver::add_cnf(Cnf* cnf_appendage) {
	int max_vc = cnf_appendage->vc;
	if (this->cnf != NULL)
	  if (max_vc < this->cnf->vc)
	    max_vc = this->cnf->vc;
	this->mark2 = (bool*)realloc(this->mark2,sizeof(bool)*(max_vc+1));
    while (max_vc > nVars()) newVar();
    vec<Lit> lits;
    int var;
    for (int i=0; i<cnf_appendage->cc; i++) {
      lits.clear();
      for (int j=0; j<cnf_appendage->cl[i]; j++) {
        var = abs(cnf_appendage->clauses[i][j])-1;
        while (var >= nVars()) newVar(); // just to be sure
        lits.push((cnf_appendage->clauses[i][j] > 0) ? mkLit(var) : ~mkLit(var));
      }
      addClause_(lits);
    }
}


// scan through the solution generated and set mark2[var] to be true for all the variables nessisary to satisfy the CNF
// NOTE: this is a ...relatively... simple reason-finding 
bool MinisatSolver::prune_solution() {
  for (int i=1; i<=cnf->vc; i++)
    mark2[i] = false;
  // scan through original CNF clauses, marking variables that satisfy clauses true
  for (int i=0; i<cnf->cc; i++) {
    bool satisfied = false;
    int min_satisfying_var = -1;
    int min_satisfying_lit = -1;
    for (int j=0; j<cnf->cl[i]; j++) {
      int lit = cnf->clauses[i][j];
      int var = abs(lit);
      if ( ((lit>0) && (model[var-1]==l_True)) || ((lit<0) && (model[var-1]==l_False)) ) { // if literal is set
        satisfied = true;
        if (mark2[var] == true) {
          min_satisfying_var = var;
          min_satisfying_lit = lit;
          break;
        }
        if ((min_satisfying_var==-1)||(
        	((lit>0) && (min_satisfying_lit<0)) || // priority towards positive literals
        	(var<min_satisfying_var)
        	))
          min_satisfying_var = var;
      }
    }
    if (!satisfied) // if a clause is unsatisfied return false
      return false;
    mark2[min_satisfying_var] = true;
  }
  return true;
}

int MinisatSolver::run() {
  bool ret = solve(false);
  DB(printf("returning %i\n",ret);)
  return ret;
}
void MinisatSolver::load_into_message(Message* m, RangeSet &r) {
  if (VLOG_IS_ON(4)) {
    std::stringstream ss;
    for (int v = 0; v<nVars(); v++) {
      if (model[v]==l_True)
       ss << (v+1) << " ";
      if (model[v]==l_False)
       ss << -(v+1) << " ";
    }
    VLOG(4) << "MINISAT_SOLVER -- solution prior to trimming " << ss.str();
  }
  if (!prune_solution()) {
    throw ConsistencyException("Minisat returned false solution\n");
  }
  if (VLOG_IS_ON(4)) {
    std::stringstream ss;
    for (int v = 0; v<nVars(); v++) {
      if (mark2[v+1]) {
        ss << (v+1) << " ";
      } else {
        ss << -(v+1) << " ";
      }
    }
    VLOG(4) << "MINISAT_SOLVER -- marked variables " << ss.str();
  }
  m->assignments.clear();
  for (auto var = r.buffer.begin(); var != r.buffer.end(); var++) {
    for (int variable = (*var).first; variable <= (*var).second; variable++) {
      if ((variable > cnf->vc) || (!mark2[variable]))
        continue;
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
  VLOG(4) << "MINISAT_SOLVER -- solution post trimming " << *m;
}
bool MinisatSolver::is_solver_unit_contradiction() {
  return false;
}
bool MinisatSolver::reset_solver() {
  //search(0);
  cancelUntil(0);
  return true;
}
void MinisatSolver::solver_add_conflict_clause(std::deque<int> d) {
  cnf->add_clause(d);
  vec<Lit> lits;
  lits.clear();
  for (auto it = d.begin(); it!=d.end(); it++) {
    int lit = *it;
    int abs_lit = abs(lit)-1;
    while (abs_lit>=nVars()) newVar();
    lits.push((lit > 0) ? mkLit(abs_lit) : ~mkLit(abs_lit));
  }
  addClause(lits);
}
void MinisatSolver::load_into_deque(deque<int> &d, RangeSet &r) {
  if (!prune_solution()) {
    throw ConsistencyException("Minisat returned false solution\n");
  }
  d.clear();
  for (auto var = r.buffer.begin(); var != r.buffer.end(); var++) {
    for (int variable = (*var).first; variable <= (*var).second; variable++) {
      if ((variable > cnf->vc) || (!mark2[variable]))
        continue;
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
}


