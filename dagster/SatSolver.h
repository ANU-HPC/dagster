/*************************
Copyright 2020 Mark Burgess

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
//
//Originally extending from Tinisat
// undder GPL, authored 2007 Jinbo Huang
//

#ifndef _SAT_SOLVER
#define _SAT_SOLVER

#include "CnfManager.h"
#include "strengthener/MpiBuffer.h"
#include <deque>
#include <functional>
#include <ostream>
#include <vector>

#include <mpi.h>
#include <deque>
#include <ostream>
#include <set>
#include <string>
#include <vector>
#include "SatSolver.h"
#include "strengthener/Work.h"
#include <stdlib.h>
#include <stdio.h>
#include <deque>
#include "Dag.h"
#include "Cnf.h"
#include "message.h"


struct Luby {           // restart scheduler as proposed in
  vector<unsigned> seq; // Optimal Speedup of Las Vegas Algorithms
  unsigned index;       // Michael Luby et al, 1993
  unsigned k;
  Luby() : index(0), k(1) {}
  unsigned next() {
    if (++index == (unsigned)((1 << k) - 1))
      seq.push_back(1 << (k++ - 1));
    else
      seq.push_back(seq[index - (1 << (k - 1))]);
    return seq.back();
  }
};

enum { suggestion_first,
       cdcl_first } heuristic_rotation;

class SatSolver : public CnfManager {
private:

  unsigned nVars;       // num of variables in varOrder
  Luby luby;            // restart scheduler
  unsigned lubyUnit;    // unit run length for Luby's
  unsigned nextDecay;   // next score decay point
  unsigned nextRestart; // next restart point
  bool short_stopping;

  int selectLiteral();
  // select next literal by resolving conflicts in learnt clauses
  int selectLiteral__conflict();
  // select next literal using Variable State Independent Decaying Sum heuristic
  int selectLiteral__vsids();

  bool verifySolution();
  bool verify_and_trim_Solution();

  /** buffer for communication with reducer instance */
  MpiBuffer *mpi_buffer;
  Work work;
  /** buffer for receiving solutions from SLS instances */
  int* sls_solution_buffer;
  size_t sls_solution_buffer_size;
  /** Counter for (non-novel) solutions found by SLS for this node */
  int sls_solution_count;
  /** Counter for novel solutions found by SLS for this node */
  int sls_novel_solution_count;

  vector<int> unit_conflicts; // TinySAT specific. Recall of unit conflict clauses from last run? Basically, TinySAT deletes unit clauses. 
  vector<int> solution_conflict_indices; // special vector that stores indices of untrimmable conflict clauses
  /** The phase of communications for the hybrid SAT solver. */
  int phase;
  MPI_Comm *communicator_sls;
  MPI_Comm *communicator_strengthener;
  
  MPI_Request sls_solution_request;

public:

  int run();

  void add_arbitrary_clause(int *inClauseArray, int inClauseLength, int inClauseLitPool, int inClauseLitPoolPos);

  int numSLSProcesses;  // total number of SLS processes
  int *processDLevel;
  int currentSLS;        // next SLS process to assign
  int **suggestions;     // list of suggestions received from SLS
  int currentSuggestion; // index of current suggestion to try

  // The CDCL search is a backtracking search that explores a
  // binary tree according to some heuristic. At any given point
  // of the search, the "prefix" is the set of decisions that
  // lead to the bit of tree being explored at that point.
  int *prefix;

  // index of the suggestion we are on
  int currentSuggestionBuffer;
  // the boolean flag indicating whether an ongoing window suggestion buffer get is in progress (actually functions as a flag to get both buffers worth of suggestions)
  bool onGoingGet;
  // the rank of the SLS process that we are currently getting the window for suggestion data from
  int lockedSLS;

  // Number of assignments to variables---i.e. suggestions---that are
  // transmitted as advice from a gnovelty+ search instance
  int suggestion_size;

  // How many decisions to make before asking for advice from a gnovelty+ search
  int decision_interval;

  // suggest the next literal to be set by the CDCL instance from SLS
  int get_suggestion();

  // function to check gnovelties for solutions
  int sls__get_solutions();

  // backtrack solver and reset hybrid solver levels after backtracking
  void backtrack_func(int);

  /** the function to call to push clauses to the reducer */
  void push_to_reducer(deque<int>* toPushClause, int inClauseLitPool, int inClauseLitPoolPos);
  /** the function to call to receive clauses from the reducer */
  void get_from_reducer();

  SatSolver(Cnf* cnf, int decision_interval, int suggestion_size, MPI_Comm *communicator_sls, MPI_Comm *communicator_strengthener, bool pure_literal_assertion, bool short_stopping, string &heuristic_rotation_scheme, int phase);
  ~SatSolver();

  MPI_Win window;
  MPI_Win solution_window;

  void solver_add_conflict_clause(std::deque<int>);
  bool reset_solver();
  void printSolution(FILE *);


};
#endif
