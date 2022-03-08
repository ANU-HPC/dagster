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

// True global variables
int world_rank; // absolute MPI global world rank
int world_size; // absolute MPI global world size
Arguments command_line_arguments; // holder for parsed command line arguments
CnfHolder* cnf_holder; // the cnf_holder object for retrieving CNF components for a dag


// for a given message, output it to the results file
void process_solution(Dag* dag, Message* m) {
  FILE *fout;
  TEST_NOT_NULL(fout = fopen(command_line_arguments.output_filename, "a"))
  for (int i = 0; i < m->assignments.size(); i++)
    if (dag->reporting.find( abs(m->assignments[i])) )    // only print the variable if it is in reporting
      fprintf(fout, "%i ", m->assignments[i]);
  fprintf(fout, "\n");
  fclose(fout);
}



// if mode 0, we dont need to worry about any gnovelty or strengthener stuff
// and we can proceed with a tested vanilla TinySAT structure, where there is one master and the rest are tinisats.
vector<Message*> mode_0_execute(WrappedSolutionsInterface *master_implementation) {
  MPI_Comm mastercommunicator; //  = MPI_COMM_WORLD;
  vector<Message*> solutions;
  // We are assuming at least 2 processes for this task
  if (world_size < 2) {
    LOG(ERROR) << "World size must be greater than 1";
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
  MPI_Comm_split(MPI_COMM_WORLD, 0, world_rank, &mastercommunicator);
  MPICommsInterface* comms = new MPICommsInterface(&mastercommunicator);
  if (world_rank == 0) { // enter the master loop if rank zero
    auto master = Master(comms,master_implementation,command_line_arguments.ENUMERATE_SOLUTIONS,command_line_arguments.BREADTH_FIRST_NODE_ALLOCATIONS);
    solutions = master.loop();
  } else { // enter the worker loop otherwise
    Worker* worker = new Worker(cnf_holder->dag, comms, NULL, NULL);
    worker->loop();
    delete worker;
  }
  delete comms;
  return solutions;
}

// if mode 1, make partitions and subcommunicators to glue together gnovelties with HybridSAT solvers
// need to do some index juggling to figure out which processes should be gnovelties and linked with tinisats
// mastercommunicator holds the master and the CDCLs
vector<Message*> mode_1_execute(WrappedSolutionsInterface *master_implementation) {
  MPI_Comm subcommunicator;
  MPI_Comm mastercommunicator;
  MPI_Comm subcommunicator_strengthener;
  vector<Message*> solutions;
  // check that we can even boot a master, and one worker, with its allocated novelties
  if (world_size < 2 + command_line_arguments.novelty_number) {
    LOG(ERROR) << "World size must be at least enough to support a master, worker and associated gnovelties";
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
  // enter the master loop if rank zero
  if (world_rank == 0) {
    // master process does not have any gNovelty helpers
    MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, 0, &subcommunicator);
    MPI_Comm_split(MPI_COMM_WORLD, 0, 0, &mastercommunicator);
    //enter master loop
    MPICommsInterface* comms = new MPICommsInterface(&mastercommunicator);
    auto master = Master(comms,master_implementation,command_line_arguments.ENUMERATE_SOLUTIONS,command_line_arguments.BREADTH_FIRST_NODE_ALLOCATIONS);
    solutions = master.loop();
    delete comms;
  } else { // else we are a worker of some kind.
    int num_procs_per_hybrid_group = 1 + command_line_arguments.novelty_number;
    int hybrid_group_num = (world_rank - 1) / num_procs_per_hybrid_group;
    int index_in_hybrid_group = (world_rank - 1) % num_procs_per_hybrid_group;
    if ((hybrid_group_num + 1) * num_procs_per_hybrid_group >= world_size) { // if we have an underfull final group then merge it into the previous one, to create an overfull group
      hybrid_group_num--;
      index_in_hybrid_group += 1 + (command_line_arguments.novelty_number);
    }
    MPI_Comm_split(MPI_COMM_WORLD, hybrid_group_num, index_in_hybrid_group, &subcommunicator);
    if (index_in_hybrid_group == 0) {
      // we are the controlling process of a HybridSatSolver group
      MPI_Comm_split(MPI_COMM_WORLD, 0, hybrid_group_num + 1, &mastercommunicator);
      MPICommsInterface* comms = new MPICommsInterface(&mastercommunicator);
      Worker* worker = new Worker(cnf_holder->dag, comms, &subcommunicator, NULL);
      worker->loop(); // enter the worker loop
      delete comms;
      delete worker;
    } else {
      // we are a gNovelty helper process
      MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, 0, &mastercommunicator);
      // dump the process into gnovelty_main with the appropriate subcommunicator, and hope everything works >_<
      gnovelty_main(&subcommunicator, command_line_arguments.suggestion_size, command_line_arguments.advise_scheme, command_line_arguments.dynamic_local_search);
    }
  }
  return solutions;
}


// if mode 2, make partitions and subcommunicators to glue together gnovelties with HybridSAT solvers and a strengthener each
// need to do some index juggling to figure out which processes should be gnovelties and linked with tinisats and strengthener
vector<Message*> mode_2_execute(WrappedSolutionsInterface *master_implementation) {
  MPI_Comm subcommunicator_sls;
  MPI_Comm mastercommunicator;
  MPI_Comm subcommunicator_strengthener;
  vector<Message*> solutions;
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
    //enter master loop
    MPICommsInterface* comms = new MPICommsInterface(&mastercommunicator);
    auto master = Master(comms,master_implementation,command_line_arguments.ENUMERATE_SOLUTIONS,command_line_arguments.BREADTH_FIRST_NODE_ALLOCATIONS);
    solutions = master.loop();
    delete comms;
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
      MPICommsInterface* comms = new MPICommsInterface(&mastercommunicator);
      Worker* worker = new Worker(cnf_holder->dag, comms, &subcommunicator_sls, &subcommunicator_strengthener);
      worker->loop(); // enter the worker loop
      delete comms;
      delete worker;
    } else if (bin_modulo == 1) { // we are a strengthener instance
      strengthener_surrogate_main(&subcommunicator_strengthener, cnf_holder);
    } else if (command_line_arguments.novelty_number > 0) { // we are a gnovelty instance
      int subcommunicator_sls_world_rank;
      MPI_Comm_rank(subcommunicator_sls, &subcommunicator_sls_world_rank);
      // dump the process into gnovelty_main with the appropriate subcommunicator_sls, and hope everything works >_<
      gnovelty_main(&subcommunicator_sls, command_line_arguments.suggestion_size, command_line_arguments.advise_scheme, command_line_arguments.dynamic_local_search);
    } else {
      VLOG(2) << "process " << world_rank << " is left over and will not contribute to SAT solving" << std::endl;
    }
  }
  return solutions;
}


// if mode 3, make partitions and subcommunicators to glue together each SAT solvers and a strengthener each
vector<Message*> mode_3_execute(WrappedSolutionsInterface *master_implementation) {
  MPI_Comm mastercommunicator;
  MPI_Comm subcommunicator_strengthener;
  vector<Message*> solutions;
  // check that we can even boot a master, and one worker, with its allocated novelties
  if ((world_size >=3) && ((world_size-1)%2 ==0)) {} else {
    LOG(ERROR) << "World size must be at least enough to support a master, and worker + strengthener pairs";
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
  // enter the master loop if rank zero
  if (world_rank == 0) {
    MPI_Comm_split(MPI_COMM_WORLD, 0, 0, &mastercommunicator);
    MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, 0, &subcommunicator_strengthener);
    //enter master loop
    MPICommsInterface* comms = new MPICommsInterface(&mastercommunicator);
    auto master = Master(comms,master_implementation,command_line_arguments.ENUMERATE_SOLUTIONS,command_line_arguments.BREADTH_FIRST_NODE_ALLOCATIONS);
    solutions = master.loop();
    delete comms;
  } else {
    // we are a worker of some kind
    int bin_index = (world_rank - 1) / 2;    // groups the processes into sizes of 1+novelty_number
    int bin_modulo = (world_rank - 1) % 2;   // the rank of the processes in each of the groups
    // Splitting communicators
    if (bin_modulo == 0) { // Tinisat
      MPI_Comm_split(MPI_COMM_WORLD, 0, bin_index + 1, &mastercommunicator);
      MPI_Comm_split(MPI_COMM_WORLD, bin_index, bin_modulo, &subcommunicator_strengthener);
      MPICommsInterface* comms = new MPICommsInterface(&mastercommunicator);
      Worker* worker = new Worker(cnf_holder->dag, comms, NULL, &subcommunicator_strengthener);
      worker->loop(); // enter the worker loop
      delete comms;
      delete worker;
    } else { // Strengthener
      MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, 0, &mastercommunicator);
      MPI_Comm_split(MPI_COMM_WORLD, bin_index, bin_modulo, &subcommunicator_strengthener);
      strengthener_surrogate_main(&subcommunicator_strengthener, cnf_holder);
    }
  }
  return solutions;
}



int main(int argc, char **argv) {
  // initialise google logging and load command line arguments
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  command_line_arguments.load(argc, argv);

  //initialise all the MPI stuff, find out MPI world rank and size
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // load the dag, and do all required CNF splitting
  Dag* dag = new Dag(command_line_arguments.dag_filename);
  cnf_holder = new CnfHolder(dag, command_line_arguments.cnf_directory, command_line_arguments.cnf_filename, 2);
  if ((command_line_arguments.cnf_directory == NULL) || (world_rank == 0))
    cnf_holder->generate_decomposition();
  else
    cnf_holder->generate_pseudo_decomposition();
  
  // initialise the respective master solution object, and populate with seed messages
  WrappedSolutionsInterface *master_implementation = NULL;
  if (world_rank == 0) {
    if (command_line_arguments.master_sub_mode==0) {
      master_implementation = new TableSolutions(dag,false);
    } else if (command_line_arguments.master_sub_mode==1) {
      master_implementation = new BDDSolutions(dag,cnf_holder->max_vc);
      if ("" != command_line_arguments.BDD_compilation_scheme) {
        VLOG(4) <<__LINE__<<__PRETTY_FUNCTION__<< "MASTER: setting compilation scheme to: "<<command_line_arguments.BDD_compilation_scheme<<".\n";
        ((BDDSolutions*)master_implementation)->set__BDD_compilation_scheme(command_line_arguments.BDD_compilation_scheme);
      } else {
        VLOG(4) <<__LINE__<<__PRETTY_FUNCTION__<<"MASTER: no BDD compilation set on command line.\n";
      }
    } else {
      VLOG(2) << "WARNING: using TableMaster in dumb mode... I hope you know what you are doing.";
      master_implementation = new TableSolutions(dag,true);
    }
    // seed dag with ininital empty messages for each node.
    for (int i = 0; i < dag->no_nodes; i++) {
      if (dag->node_status[i] == 1) {
        Message *m = new Message(i, i);
        master_implementation->add_message(m);
        delete m;
      }
    }
    //clear dag_out file
    FILE *fout;
    TEST_NOT_NULL(fout = fopen(command_line_arguments.output_filename, "w"))
    fclose(fout);

  }

  // enter into respective mode
  vector<Message*> solutions;
  if (command_line_arguments.mode == 0) {
    solutions = mode_0_execute(master_implementation);
  } else if (command_line_arguments.mode == 1) {
    solutions = mode_1_execute(master_implementation);
  } else if (command_line_arguments.mode == 2) {
    solutions = mode_2_execute(master_implementation);
  } else if (command_line_arguments.mode == 3) {
    solutions = mode_3_execute(master_implementation);
  } else {
    throw BadParameterException("Dagster called with non existant mode");
  }
  
  // dump found solutions to output file
  for (auto it = solutions.begin(); it != solutions.end(); it++) {
    VLOG(1) << "SOLUTION: " << **it;
    process_solution(dag, *it);
  }
  // print message counts
  if (world_rank == 0) {
    if (VLOG_IS_ON(1)) {
      VLOG(0) << "MASTER: number of solutions at each node:";
      for (int i = 0; i < dag->no_nodes; i++)
        VLOG(0) << "node " << i << ": incoming " << master_implementation->get_incoming_message_count(i) << " outgoing " << master_implementation->get_outgoing_message_count(i);
      VLOG(0) << "MASTER: number of nodes in each bdd:";
      master_implementation->print_stats(false, true);
    }
    delete master_implementation;
  }
  // print exit message and cleanup
  VLOG(2) << "process " << world_rank << " exiting";
  
  delete dag;
  MPI_Finalize();
}

