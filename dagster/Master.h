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


#ifndef _MASTER_H
#define _MASTER_H

#include <vector>
#include "Arguments.h"
#include "mpi_global.h"
#include "MPICommsInterface.h"
#include "SolutionsInterface.h"
#include "Message.h"
#include "MasterOrganiser.h"
#include "StatisticsModule.h"
#include "CnfHolder.h"

using namespace std;

extern CnfHolder* cnf_holder;

class Master{
 public:
  MPICommsInterface* comms;
  SolutionsInterface *master;
  int ENUMERATE_SOLUTIONS;
  bool BREADTH_FIRST_NODE_ALLOCATIONS;
  bool exiting;
  int checkpointing;

  // tracking unsat nodes
  set<int> dag_nodes_generated_solutions;
  set<int> dag_nodes_given_assignments;
  bool last_call_unsat = false; // meaningless the first time
  
  MasterOrganiser* organiser;
  StatisticsModule* stats;
  vector<Message*> solutions;
  set<int> subgraph_finished; // keep track of what subgraph indices are finished


  Master(int comms_size, int no_nodes, SolutionsInterface *master, int ENUMERATE_SOLUTIONS, bool BREADTH_FIRST_NODE_ALLOCATIONS, bool exiting=true, int checkpointing=0) {
    this->comms = NULL;
    this->master = master;
    this->ENUMERATE_SOLUTIONS = ENUMERATE_SOLUTIONS;
    this->BREADTH_FIRST_NODE_ALLOCATIONS = BREADTH_FIRST_NODE_ALLOCATIONS;
    this->exiting = exiting;
    this->checkpointing = checkpointing;
    this->organiser = new MasterOrganiser(comms_size-1);
    this->stats = new StatisticsModule(comms_size-1,no_nodes);
    this->clear();
  }
  Master(MPICommsInterface* comms, SolutionsInterface *master, int ENUMERATE_SOLUTIONS, bool BREADTH_FIRST_NODE_ALLOCATIONS, bool exiting=true, int checkpointing=0) {
    this->comms = comms;
    this->master = master;
    this->ENUMERATE_SOLUTIONS = ENUMERATE_SOLUTIONS;
    this->BREADTH_FIRST_NODE_ALLOCATIONS = BREADTH_FIRST_NODE_ALLOCATIONS;
    this->exiting = exiting;
    this->checkpointing = checkpointing;
    this->organiser = new MasterOrganiser(comms->world_size-1);
    this->stats = new StatisticsModule(comms->world_size-1,cnf_holder->dag->no_nodes);
    this->clear();
  }
  ~Master() {
  	delete this->organiser;
  	delete this->stats;
  }
  vector<Message*> loop();
  vector<Message*> loop(const char* checkpoint_file);
  void send_exit();
  void clear();
  
  void dump_checkpoint(FILE* fp);
  void load_checkpoint(FILE* fp);
  
};


#endif

