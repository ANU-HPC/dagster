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



//*************************************
//
//   INTERACTIVE FUNCTIONS: 
//       -experimental- interface for external programs to compile 
//       with dagster, and call these functions to initiate dagster calls
//
//*************************************


#include <algorithm>
#include <glog/logging.h>
#include <iostream>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

using namespace std;

#include "Cnf.h"
#include "Dag.h"
#include "arguments.h"
#include "gnovelty/gnovelty_main.hh"
#include "Master.h"
#include "Worker.h"
#include "MPICommsInterface.h"
#include "strengthener/StrengthenerInterface.h"
#include "CnfHolder.h"
#include <zlib.h>

#include "SolutionsInterface.h"
#include "TableSolutions.h"
#include "BDDSolutions.h"

#include "mpi_global.h"



MPI_Comm mastercommunicator;
MPI_Comm subcommunicator_sls;
MPI_Comm subcommunicator_strengthener;

int world_rank;
int world_size;

Cnf* cnf;
Dag* dag;
CnfHolder* cnf_holder;
Master* master = NULL;

Arguments command_line_arguments;
WrappedSolutionsInterface *master_implementation;
MPICommsInterface* comms;

vector<Message*> extra_clauses;
bool workers_in_loop = false;


void clearBDD() {
  if (master_implementation != NULL)
    delete master_implementation;
  if (command_line_arguments.master_sub_mode==0) {
    master_implementation = new TableSolutions(dag,false);
  } else {
    master_implementation = new BDDSolutions(dag, cnf_holder->max_vc);
    if ("" != command_line_arguments.BDD_compilation_scheme)
    {
      VLOG(4) <<__LINE__<<std::endl<<__PRETTY_FUNCTION__<<std::endl
  	    << "MASTER: setting compilation scheme to: "
  	    <<command_line_arguments.BDD_compilation_scheme<<".\n";
      ((BDDSolutions*)master_implementation)->set__BDD_compilation_scheme(command_line_arguments.BDD_compilation_scheme);
    } else {
      VLOG(4) <<__LINE__<<std::endl<<__PRETTY_FUNCTION__<<std::endl
  	    << "MASTER: no BDD compilation set on command line.\n";
    }
  }
  master->master = master_implementation;
  for (auto it = extra_clauses.begin(); it != extra_clauses.end(); it++) {
    master_implementation->register_message_completion(*it);
  }
}


int recieve_CNF_DAG() {
  // load up the DAG and CNF, and connect them together
  if (cnf!=NULL)
    delete cnf;
  cnf = comms->receive_Cnf(0, MPI_TAG_CNF_SEND);
  if (cnf == NULL) // if NULL CNF sent and recieved then send zero, thus terminate
    return 0;
  if (cnf_holder != NULL)
    delete cnf_holder;
  if (dag!=NULL)
    delete dag;
  dag = comms->receive_Dag(0, MPI_TAG_DAG_SEND);
  //dag->reference_clauses(cnf);
  cnf_holder = new CnfHolder(dag, NULL, NULL, 2);
  cnf_holder->master_cnf = cnf; //cnf_holder->generate_decomposition();
  cnf_holder->max_vc=cnf->vc;
  return 1; // return of one indicates successfull recieve of a dag/cnf
}

void load_CNF_DAG(Cnf* new_cnf, Dag* new_dag) {
  if (workers_in_loop)
    master->send_exit();
  cnf = new_cnf;
  dag = new_dag;
  if (cnf==NULL) {
    for (int i=1; i<world_size; i++) {
      comms->send_Cnf(i,MPI_TAG_CNF_SEND,cnf);
    }
  } else {
    for (int i=1; i<world_size; i++) {
      comms->send_Cnf(i,MPI_TAG_CNF_SEND,cnf);
      comms->send_Dag(i,MPI_TAG_DAG_SEND,dag);
    }
    // load up the DAG and CNF, and connect them together
    //dag->reference_clauses(cnf);
    if (cnf_holder != NULL)
      delete cnf_holder;
    cnf_holder = new CnfHolder(dag, NULL, NULL, 2);
    cnf_holder->master_cnf = cnf; //cnf_holder->generate_decomposition();
    cnf_holder->max_vc=cnf->vc;
    for (auto it = extra_clauses.begin(); it != extra_clauses.end(); it++)
      delete *it;
    extra_clauses.clear();
    clearBDD();
    workers_in_loop = true;
  }
}


vector<Message*> solve() {
  vector<Message*> solutions;
  solutions = master->loop();
  return solutions;
}

set<int> get_unsat_nodes() {
  assert (master->last_call_unsat);
  set<int> unsat_nodes = set<int>();

  // for all nodes which were given an assignment
  for (auto ita=master->dag_nodes_given_assignments.begin(); ita!=master->dag_nodes_given_assignments.end(); ita++) {
    // dag_node given assignment
    const int dag_node = *ita; 
    if (master->dag_nodes_generated_solutions.find(dag_node) == master->dag_nodes_generated_solutions.end()) {
      // given assignment and didn't produce a solution
      unsat_nodes.insert(dag_node);
    } else {
      // if this node DID produce a solution, check if its descendants didn't
      // also see if any descendant can't create a solution
      for (auto itb=dag->forward_connections[dag_node].begin(); itb!=dag->forward_connections[dag_node].end(); itb++) {
        const int descendant = *itb;
        //check if descendant produced a solution
        if (master->dag_nodes_generated_solutions.find(descendant) == master->dag_nodes_generated_solutions.end()) {
          // if all THEIR parents produced solutions AND it didn't produce a solution then either:
          // 1. then the BDD couldn't combine solutions
          // 2. it could, but all those were unsat
          // Either way, the node is unsat
        
          bool all_coparents_produced_solutions = true;
          for (auto itc=dag->reverse_connections[descendant].begin(); itc!=dag->reverse_connections[descendant].end(); itc++) {
            const int coparent = *itc; 
            if (master->dag_nodes_generated_solutions.find(coparent) == master->dag_nodes_generated_solutions.end()){
              all_coparents_produced_solutions = false;
              break;
            }
          }
          if (all_coparents_produced_solutions) unsat_nodes.insert(descendant);
        }
      }
    }
  }

  assert(unsat_nodes.size());
  return unsat_nodes;
}

void addClause(int node, int* clause) {
  Message* m = new Message(node,node);
  for (int i=0; clause[i]!=0; i++)
    m->assignments.push_back(-clause[i]);
  extra_clauses.push_back(m);
  master_implementation->register_message_completion(m);
}

void initial_message(Message* message) {
  master_implementation->add_message(message);
}

void shutdown() {
  load_CNF_DAG(NULL,NULL);
  VLOG(2) << "process " << world_rank << " exiting";
  delete comms;
  if (master_implementation != NULL)
    delete master_implementation;
  if (master != NULL)
    delete master;
  if (cnf!=NULL)
    delete cnf;
  if (dag!=NULL)
    delete dag;
  for (auto it = extra_clauses.begin(); it != extra_clauses.end(); it++)
    delete *it;
  MPI_Finalize();
}

void worker_shutdown() {
  VLOG(2) << "process " << world_rank << " exiting";
  if (comms != NULL)
    delete comms;
  if (cnf!=NULL)
    delete cnf;
  if (dag!=NULL)
    delete dag;
  MPI_Finalize();
}

void master_init() {
  comms = new MPICommsInterface(&mastercommunicator);
  master = new Master(comms,master_implementation,command_line_arguments.ENUMERATE_SOLUTIONS,command_line_arguments.BREADTH_FIRST_NODE_ALLOCATIONS,false);
}

void global_init(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  //command_line_arguments.load(argc, argv);
  //initialise all the MPI stuff
  MPI_Init(NULL, NULL);
  // Find out rank, size
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  master_implementation = NULL;
  comms = NULL;
  cnf_holder = NULL;
  extra_clauses.clear();
  
  // if mode 0, we dont need to worry about any gnovelty stuff
  // and we can proceed with a tested vanilla TinySAT structure
  if (command_line_arguments.mode == 0) {
    // We are assuming at least 2 processes for this task
    if (world_size < 2) {
      LOG(ERROR) << "World size must be greater than 1";
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
    MPI_Comm_split(MPI_COMM_WORLD, 0, world_rank, &mastercommunicator);
    if (world_rank == 0) {
    } else {
      comms = new MPICommsInterface(&mastercommunicator);
      while (recieve_CNF_DAG()) {
        Worker* worker = new Worker(dag, comms, NULL, NULL);
        worker->loop(); // enter the worker loop otherwise
        delete worker;
      }
    }
  }
  // if mode 1, make partitions and subcommunicators to glue together gnovelties with a HybridSAT solvers
  // mastercommunicator holds the master and the CDCLs
  else if (command_line_arguments.mode == 1) {
    // check that we can even boot a master, and one worker, with its allocated novelties
    if (world_size < 2 + command_line_arguments.novelty_number) {
      LOG(ERROR) << "World size must be at least enough to support a master, worker and associated gnovelties";
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
    // enter the master loop if rank zero
    if (world_rank == 0) {
      // master process does not have any gNovelty helpers
      MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, 0, &subcommunicator_sls);
      MPI_Comm_split(MPI_COMM_WORLD, 0, 0, &mastercommunicator);
    } else { // else we are a worker of some kind.
      int num_procs_per_hybrid_group = 1 + command_line_arguments.novelty_number;
      int hybrid_group_num = (world_rank - 1) / num_procs_per_hybrid_group;
      int index_in_hybrid_group = (world_rank - 1) % num_procs_per_hybrid_group;
      if ((hybrid_group_num + 1) * num_procs_per_hybrid_group >= world_size) { // if we have an underfull final group then merge it into the previous one, to create an overfull group
        hybrid_group_num--;
        index_in_hybrid_group += 1 + (command_line_arguments.novelty_number);
      }
      MPI_Comm_split(MPI_COMM_WORLD, hybrid_group_num, index_in_hybrid_group, &subcommunicator_sls);
      if (index_in_hybrid_group == 0) {
        // we are the controlling process of a HybridSatSolver group
        MPI_Comm_split(MPI_COMM_WORLD, 0, hybrid_group_num + 1, &mastercommunicator);
        comms = new MPICommsInterface(&mastercommunicator);
        while (recieve_CNF_DAG()) {
          Worker* worker = new Worker(dag, comms, &subcommunicator_sls, NULL);
          worker->loop(); // enter the worker loop
          delete worker;
        }
      } else {
        // we are a gNovelty helper process
        MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, 0, &mastercommunicator);
        // dump the process into gnovelty_main with the appropriate subcommunicator
        while (recieve_CNF_DAG())
          gnovelty_main(&subcommunicator_sls, command_line_arguments.suggestion_size, command_line_arguments.advise_scheme, command_line_arguments.dynamic_local_search);
      }
    }
  } else if (command_line_arguments.mode == 2) {
    // check that we can even boot a master, and one worker, with its allocated novelties
    if (world_size < 3 + command_line_arguments.novelty_number) {
      LOG(ERROR) << "World size must be at least enough to support a master, worker, strengthener and associated gnovelties";
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
    // enter the master loop if rank zero
    if (world_rank == 0) {
      MPI_Comm_split(MPI_COMM_WORLD, 0, 0, &mastercommunicator);
      MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, 0, &subcommunicator_sls);
      MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, 0, &subcommunicator_strengthener);
    } else {
      // we are a worker of some kind
      int bin_index = (world_rank - 1) / (2 + command_line_arguments.novelty_number);    // groups the processes into sizes of 1+novelty_number
      int bin_modulo = (world_rank - 1) % (2 + command_line_arguments.novelty_number);   // the rank of the processes in each of the groups
      if ((bin_index + 1) * (2 + command_line_arguments.novelty_number) >= world_size) { // if we have an underfull final group then merge it into the previous one, to create an overfull group
        bin_index--;
        bin_modulo += 2 + command_line_arguments.novelty_number;
      }
      // Splitting communicators
      if (bin_modulo == 0) { // Tinisat
        MPI_Comm_split(MPI_COMM_WORLD, 0, bin_index + 1, &mastercommunicator);
        MPI_Comm_split(MPI_COMM_WORLD, bin_index, bin_modulo, &subcommunicator_sls);
        MPI_Comm_split(MPI_COMM_WORLD, bin_index, bin_modulo, &subcommunicator_strengthener);
      } else if (bin_modulo == 1) { // Strengthener
        MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, 0, &mastercommunicator);
        MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, 0, &subcommunicator_sls);
        MPI_Comm_split(MPI_COMM_WORLD, bin_index, bin_modulo, &subcommunicator_strengthener);
      } else { // SLS
        MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, 0, &mastercommunicator);
        MPI_Comm_split(MPI_COMM_WORLD, bin_index, bin_modulo, &subcommunicator_sls);
        MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, 0, &subcommunicator_strengthener);
      }
      // Loading up exectuables
      if (bin_modulo == 0) { // we are a specific HybridSatSolver
        comms = new MPICommsInterface(&mastercommunicator);
        while (recieve_CNF_DAG()) {
          Worker* worker = new Worker(dag, comms, &subcommunicator_sls, &subcommunicator_strengthener);
          worker->loop(); // enter the worker loop
          delete worker;
        }
      } else if (bin_modulo == 1) { // we are a strengthener instance
        while (recieve_CNF_DAG())
          strengthener_surrogate_main(&subcommunicator_strengthener, cnf_holder);
      } else if (command_line_arguments.novelty_number > 0) { // we are a gnovelty instance
        int subcommunicator_sls_world_rank;
        MPI_Comm_rank(subcommunicator_sls, &subcommunicator_sls_world_rank);
        // dump the process into gnovelty_main with the appropriate subcommunicator_sls
        while (recieve_CNF_DAG())
          gnovelty_main(&subcommunicator_sls, command_line_arguments.suggestion_size, command_line_arguments.advise_scheme, command_line_arguments.dynamic_local_search);
      } else {
        VLOG(2) << "process " << world_rank << " is left over and will not contribute to SAT solving" << std::endl;
      }
    }
  }
  if (world_rank != 0) { // all worker processes terminate once they exit their loops, but the master processes dosnt enter a loop and proceeds
    worker_shutdown();
	exit(0);
  }
  master_init();
}




