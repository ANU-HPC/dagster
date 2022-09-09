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


#ifndef _WORKER_H
#define _WORKER_H

#include "SatSolver.h"
#include "mpi_global.h"
#include "MPICommsInterface.h"

#include "SatSolverInterface.h"

// Worker class:
// holds data relevent to worker loop, and the worker loop itself.
class Worker {
public:
  MPICommsInterface* comms; // the communicator the worker interfaces with the master on
  Dag* dag; // local reference to the dag structure
  int solver_index; // pointer to SAT solver
  SatSolverInterface** solvers;
  Cnf* generated_cnf; // pointer to CNF that the SAT solver is working on
  int phase; // phase counter, for each message the worker sends to the gnovelties/strengthener send a new 'phase' and disregard any messages that are from old phases
  bool minisat_mode; // flag to set minisat instead of tinisat CDCL
  
  MPI_Comm *communicator_sls; // communicator for talking to the gnovelties
  MPI_Comm *communicator_strengthener; // communicator for talking to the strengthener
  
  int message_index; // the index of the message that the worker has/is working on

  Worker(Dag* dag, MPICommsInterface* comms, MPI_Comm* communicator_sls, MPI_Comm* communicator_strengthener, bool minisat_mode=false) {
    this->dag = dag;
    this->comms = comms;
    this->communicator_sls = communicator_sls;
    this->communicator_strengthener = communicator_strengthener;
    this->minisat_mode = minisat_mode;
    this->solver_index = 0;
    this->phase = 0;
    this->generated_cnf = NULL;
    this->message_index = 0;
    this->solvers = (SatSolverInterface**)calloc(sizeof(SatSolverInterface*),dag->no_nodes);
  }
  ~Worker();

  void loop();  // the Worker master loop
  void initialise_solver_from_message(Message* m); // given an initial message for a node of dag d, create all the infastructure to compute resulting messages
  int solve_and_generate_message(Message* m, Message* reference_message); // for an object message m load in the values of a solution (if there is one) and return true, if no solution return false
  bool reset_solver_for_next_solution(Message* m); // after a solution is loaded into a message, call this method to reset the solver for the processing of an additional message
};

#endif

