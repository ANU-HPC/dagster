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


MinisatSolver::MinisatSolver(Cnf* cnf) {
DB(printf("adding CNF to minisatsolver\n");
cnf->print();)
	this->cnf = new Cnf(cnf);
	this->mark2 = (bool*)calloc(sizeof(bool),cnf->vc+1);
	this->solver_unit_contradiction = false;
	this->unit_assignments.clear(true);
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
      if (!addClause_(lits)) {
        solver_unit_contradiction = true;
      }
    }
    //eliminate(true);
  }

MinisatSolver::~MinisatSolver() {
  free(this->mark2);
  delete this->cnf;
}


bool MinisatSolver::append_cnf(Cnf* cnf) {
  while (cnf->vc > nVars()) newVar();
  vec<Lit> lits;
  for (int i=0; i<cnf->cc; i++) {
    lits.clear(true);
    for (int j=0; j<cnf->cl[i]; j++) {
      int var = abs(cnf->clauses[i][j])-1;
      while (var >= nVars()) newVar(); // just to be sure
      lits.push((cnf->clauses[i][j] > 0) ? mkLit(var) : ~mkLit(var));
    }
    if (!addClause(lits)) {
      solver_unit_contradiction = true;
    }
    if (lits.size()==1) {
      this->unit_assignments.push(lits[0]);
    }
  }
  return true;
}

// scan through the solution generated and set mark2[var] to be true for all the variables nessisary to satisfy the CNF
// NOTE: this is a ...relatively... simple reason-finding 
bool MinisatSolver::prune_solution(Message* reference_message) {
  for (int i=1; i<=cnf->vc; i++)
    mark2[i] = false;
  // mark all thoes variables in the reference_message
  if (reference_message!=NULL)
    for (auto it = reference_message->assignments.begin(); it!= reference_message->assignments.end(); it++)
      if (abs(*it)<=cnf->vc)
        mark2[abs(*it)] = true;
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

int MinisatSolver::run(Message *m) {
  if (solver_unit_contradiction == true) // if unit clause conflict detected return immediate UNSAT
    return false;
  vec<Lit> lits;
  lits.clear();
  for (int j=0; j<m->assignments.size(); j++) {
    int var = abs(m->assignments[j])-1;
    lits.push((m->assignments[j] > 0) ? mkLit(var) : ~mkLit(var));
  }
  for (int i=0; i<this->unit_assignments.size(); i++) {
    lits.push(this->unit_assignments[i]);
  }
  bool ret = solve(lits, false, false);
  DB(printf("returning %i\n",ret);)
  return ret;
}
void MinisatSolver::load_into_message(Message* m, RangeSet &r, Message* reference_message) {
  if (!prune_solution(reference_message)) {
    throw ConsistencyException("Minisat returned false solution\n");
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
  // add in reference_message constraints
  for (auto it = reference_message->assignments.begin(); it!=reference_message->assignments.end(); it++) {
    for (auto it2 = m->assignments.begin(); it2!=m->assignments.end(); it2++) {
      if (*it==*it2) {
        break;
      } else if (*it==-*it2) {
        throw ConsistencyException("Minisat returned solution contradicting reference message\n");
      }
    }
    m->assignments.push_back(*it);
  }
}
bool MinisatSolver::is_solver_unit_contradiction() {
  return this->solver_unit_contradiction;
}
bool MinisatSolver::reset_solver() {
  //search(0);
  cancelUntil(0);
  return true;
}
bool MinisatSolver::solver_add_conflict_clause(std::deque<int> d) {
  vec<Lit> lits;
  lits.clear();
  for (auto it = d.begin(); it!=d.end(); it++) {
    int lit = *it;
    int abs_lit = abs(lit)-1;
    while (abs_lit>=nVars()) newVar();
    lits.push((lit > 0) ? mkLit(abs_lit) : ~mkLit(abs_lit));
  }
  if (lits.size()==1) {
    this->unit_assignments.push(lits[0]);
  }
  return addClause(lits);
}


