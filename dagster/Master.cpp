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


#include "Master.h"
#include "Cnf.h"
#include "message.h"
#include "utilities.h"
#include <algorithm>
#include <argp.h>
#include <cstdio>
#include <cstdlib>
#include <glog/logging.h>
#include <iostream>
#include <mpi.h>
#include <stack>
#include <set>
#include "arguments.h"
#include "mpi_global.h"
#include "DisorderedArray.h"
#include <cassert>
#include <ctime>

using namespace std;


// Master main loop:
// infinite loop process of:
// - checking if there is as much work available as there are workers (pulling from the <master> solutions object)
//   -- if there is no work, and all workers are free then set the terminate-trigger to exit all processes
// - sending new work to thoes workers who are requesting it
// - recieving all messages from workers and processing what they send:
//   -- if worker sends MPI_TAG_REQUEST_FOR_ASSIGNMENT, then it is completely done processing that packet of work, and is requesting a new packet of work, new work is sent with the MPI_TAG_NEW_ASSIGNMENT tag
//   -- if worker sends MPI_TAG_SOLUTION, then it is trying to deposit a solution to the work it is currently chomping away on
//   -- if worker sends MPI_TAG_POLL_FOR_REASSIGNMENT, then it is wondering if it should continue processing the work it has. MPI_TAG_NEW_ASSIGNMENT is sent if it is to switch to a new message, otherwise the same tag MPI_TAG_POLL_FOR_REASSIGNMENT is returned, indicating that the worker should keep going
// - if all workers are asking for new work, and there is no more work for them to do, then send the MPI_TAG_KILL to shut them down
//
// the Master main loop works in conjunction with the MasterOrganiser object, which keeps track of what messages are allocated to what workers
//
// Master has different operating mode depending on flag ENUMERATE_SOLUTIONS:
// - ENUMERATE_SOLUTIONS=0, means that all work will terminate as soon as any worker finds a solution to a terminal node in the dag, this is essentially racing to the first solution
// - ENUMERATE_SOLUTIONS=1, means that all work will continue until every satisfying assignment to the dag is complete, this if full model counting exersize and can be expensive
// - ENUMERATE_SOLUTIONS=2, means that master will try to find one solution on each disjoint subgraph in the dag, if dag is connected then equivalent to ENUMERATE_SOLUTIONS=0
// Additionally, master has different priorities when it collects messages, particularly by flag BREADTH_FIRST_NODE_ALLOCATIONS:
// - BREADTH_FIRST_NODE_ALLOCATIONS=0, means that the master will attempt to generate work that is DEPTH first in the dag
// - BREADTH_FIRST_NODE_ALLOCATIONS=1, means that the master will attempt to generate work that is BREADTH first in the dag
//

vector<Message*> Master::loop() {
  return loop(NULL);
}
vector<Message*> Master::loop(const char* checkpoint_file) {
  if (checkpoint_file == NULL) {
    clear();
  } else { //TODO: test this
    FILE* fp = fopen(checkpoint_file,"r");
    load_checkpoint(fp);
    fclose(fp);
  }
  bool terminate_trigger = false;
  
  clock_t t = clock();
  clock_t t2 = clock();
  int checkpointing_index = 0;
  while (true) {
    // checkpointing timing logic
    if (checkpointing) { //TODO: test this
      if (clock()-t2 > 30*60*CLOCKS_PER_SEC) {
        t2 = clock();
        std::stringstream ss;
        ss << "checkpoint_"<<checkpointing_index<<".txt";
        FILE* fp = fopen(ss.str().c_str(),"w");
        dump_checkpoint(fp);
        fclose(fp);
        checkpointing_index = (checkpointing_index+1)%6;
      }
    }
    // timing and assignment message every 4 seconds
    if (clock()-t > 4*CLOCKS_PER_SEC) {
      t = clock();
      stats->print_stats();
      std::stringstream ss;
      ss << std::fixed << std::setprecision(2) << "MASTER: message buffer -- ";
      for (int i=0; i<organiser->num_workers; i++) {
        ss << i << ":";
        if (organiser->workers[i].assigned==NULL) {
          ss << "###" << "," << "###" << " ";
        } else {
          ss << organiser->workers[i].assigned->to << "," << (clock() - organiser->workers[i].assigned->time_start)*1.0/CLOCKS_PER_SEC << " ";
        }
      }
      VLOG(3) << ss.str();
    }

    VLOG(3) << "MASTER: generating new messages"; // attempt to find as much work as there are workers to consider
    while ((!terminate_trigger) && (organiser->message_buffer.length < comms->world_size-1)) { // look for maximum work for our workers
      Message *mess = master->output_message(BREADTH_FIRST_NODE_ALLOCATIONS);                // generate an assignment
      if (mess != NULL) {  // for each new assignment found add it to the buffer
        if (subgraph_finished.find(cnf_holder->dag->subgraph_index[mess->to]) != subgraph_finished.end()) { // if the respective subgraph is finished then dump the message 
          delete mess;
          continue;
        }
        mess->time_start = clock();
        organiser->add_message(mess);
      } else
        break;
    }
    if (organiser->message_buffer.length == 0) {// all workers are idle, and there are no more messages to allocate set the terminate_trigger
      terminate_trigger = true;
      VLOG(3) << "MASTER: terminate trigger true";
    }
    if (terminate_trigger) {// if the terminate_trigger is set, make sure all workers are polled before exit.
      int i;
      VLOG(3) << "MASTER: poll exit loop";
      for (i=0; i<organiser->num_workers; i++)
        if (!(organiser->workers[i].polled)) break;
      if (i==organiser->num_workers) break;
    }

    organiser->allocate_assignments(); // call the organisers function to arrange messages to workers
    VLOG(3) << "MASTER: sending new reassignments";
    // send out all the new assignments for polled workers, or ping for polled workers without new assignments
    for (int i=0; i<organiser->num_workers; i++) {
      if ((!terminate_trigger) && (organiser->workers[i].polled)) { // dont send out any new messages if terminate_trigger
        if (organiser->workers[i].to_be_assigned != NULL) { // if master is not in terminate mode, and there is work to be assigned then send the work out
          int** additional_clauses = master->get_additional_clauses(organiser->workers[i].to_be_assigned->to);
          organiser->workers[i].to_be_assigned->set_additional_clauses(additional_clauses); // generate negated BDD representation (or otherwise NULL) if no additional clauses allready seen before
          stats->start_message(i,organiser->workers[i].to_be_assigned->to);
          comms->send_message(i+1, MPI_TAG_NEW_ASSIGNMENT, organiser->workers[i].to_be_assigned);
          VLOG(2) << "sending message to worker "<< (i+1) << " " << *(organiser->workers[i].to_be_assigned);

          const int dag_node = organiser->workers[i].to_be_assigned->to;
          dag_nodes_given_assignments.insert(dag_node);

          if (additional_clauses != NULL) {
            for (int **clause = additional_clauses; *clause != NULL; clause++)
              free(*clause);
            free(additional_clauses);
          }
          organiser->workers[i].assigned = organiser->workers[i].to_be_assigned;
          organiser->workers[i].to_be_assigned = NULL;
          organiser->workers[i].needs_refresh = false;
        } else {
          VLOG(3) << "MASTER: polling worker " << i+1;
          comms->send_tag(i+1, MPI_TAG_POLL_FOR_REASSIGNMENT);
        }
        organiser->workers[i].polled = false;
      }
    }

    // do an open blocking recieve from any of the workers
    VLOG(3) << "MASTER: receiving from workers";
    auto [source_worker,message_tag, m] = comms->receive_message();
    if (message_tag == MPI_TAG_REQUEST_FOR_ASSIGNMENT) { // worker is telling master they are completely done with their work message,
      VLOG(2) << "MASTER: received MPI_TAG_REQUEST_FOR_ASSIGNMENT from worker " << source_worker;
      // everybody is done with the message
      if (organiser->workers[source_worker-1].assigned != NULL) {
        stats->register_finish(source_worker-1);
        VLOG(1) << "MASTER: timing message for node " << organiser->workers[source_worker-1].assigned->to << " took " << (clock() - organiser->workers[source_worker-1].assigned->time_start)*1.0/CLOCKS_PER_SEC;
        delete organiser->workers[source_worker-1].assigned;
        organiser->remove_message(organiser->workers[source_worker-1].assigned);
      }
      organiser->workers[source_worker-1].polled = true;
      
    } else if (message_tag == MPI_TAG_SOLUTION) { // worker is attempting to deposit a solution
      VLOG(3) << "MASTER: received MPI_TAG_SOLUTION from worker " << source_worker;
      stats->register_solution(source_worker-1);
      if (organiser->workers[source_worker-1].assigned!=NULL)
        organiser->refresh_except(organiser->workers[source_worker-1].assigned, source_worker-1);
      dag_nodes_generated_solutions.insert(m->from);
      if ((!terminate_trigger) && (master->input_message(m) == 0)) { // if terminate_trigger not set then input message, and if it is for terminal node
        solutions.push_back(new Message(m));// found a solution for the terminal node of the DAG, push it to solutions buffer
        if (ENUMERATE_SOLUTIONS==2) { // if ENUMERATE_SOLUTIONS mode 2, then destroy all messages on the same subgraph
          if (organiser->workers[source_worker-1].assigned != NULL) {
            subgraph_finished.insert(cnf_holder->dag->subgraph_index[m->to]);
            int i=0;
            delete organiser->workers[source_worker-1].assigned;
            organiser->remove_message(organiser->workers[source_worker-1].assigned);
            while (i<organiser->message_buffer.length) {
              if (cnf_holder->dag->subgraph_index[organiser->message_buffer[i]->to] == cnf_holder->dag->subgraph_index[m->to]) {
                delete organiser->message_buffer[i];
                organiser->remove_message(organiser->message_buffer[i]);
              } else {
                i++;
              }
            }
          }
        } else if ((ENUMERATE_SOLUTIONS==0) || ((ENUMERATE_SOLUTIONS==3))) { // if ENUMERATE_SOLUTIONS mode 0 or 3, then first solution found, initiate a full terminate
          VLOG(3) << "MASTER: terminate trigger true";
          terminate_trigger = true;
        } else {} // continue running till all solutions found
      }
      
    } else if (message_tag == MPI_TAG_POLL_FOR_REASSIGNMENT) { // worker is asking if they should be reassigned to a different message
      VLOG(3) << "MASTER: received MPI_TAG_POLL_FOR_REASSIGNMENT from worker " << source_worker;
      // if the worker needs an update on additional clauses, and is not the only one left on that message, then free it from the message entirely
      if (organiser->workers[source_worker-1].assigned!=NULL)
        if (organiser->workers[source_worker-1].needs_refresh)
          organiser->workers[source_worker-1].assigned=NULL;
      organiser->workers[source_worker-1].polled = true;
      
    } else
      LOG(ERROR) << "MASTER: master received a message with unknown tag: " << message_tag;
    if (m!=NULL)
      delete m;
  }
  if (exiting)
    send_exit();
  last_call_unsat = solutions.size() == 0;
  stats->print_stats();
  return solutions;
}

void Master::send_exit() {
  // kill all remaining workers
  for (int i = 1; i < comms->world_size; i++) {
    VLOG(2) << "MASTER: sending kill signal to worker " << i;
    comms->send_tag(i, MPI_TAG_KILL);
  }
}


void Master::clear() {
  dag_nodes_generated_solutions.clear();
  dag_nodes_given_assignments.clear();
  subgraph_finished.clear();
  solutions.clear();
}

//dump_checkpoint: (DRAFT) //TODO: check working
// dump all information to open writable file pointer
// suitable for files subsequently loaded by load_checkpoint() method
void Master::dump_checkpoint(FILE* fp) {
  master->dump_checkpoint(fp);
  organiser->dump_checkpoint(fp);
  fprintf(fp,"%i ",solutions.size());
  for (Message* m : solutions) {
    m->dump_to_file(fp);
  }
  //dag_nodes_generated_solutions
  fprintf(fp,"%i ",dag_nodes_generated_solutions.size());
  for (int i : dag_nodes_generated_solutions) {
    fprintf(fp, "%i ",i);
  }
  //dag_nodes_given_assignments
  fprintf(fp,"%i ",dag_nodes_given_assignments.size());
  for (int i : dag_nodes_given_assignments) {
    fprintf(fp, "%i ",i);
  }
  //subgraph_finished
  fprintf(fp,"%i ",subgraph_finished.size());
  for (int i : subgraph_finished) {
    fprintf(fp, "%i ",i);
  }
}

//load_checkpoint: (DRAFT) //TODO: check working
// load all information from a readable file pointer
// suitable for files written by dump_checkpoint() method
void Master::load_checkpoint(FILE* fp) {
  master->load_checkpoint(fp);
  organiser->load_checkpoint(fp);
  int i;
  
  fscanf(fp,"%i ", &i);
  for (int k=0; k<i; k++) {
    int ll = 0;
    fscanf(fp,"%i ",&ll);
    dag_nodes_generated_solutions.insert(ll);
  }
  fscanf(fp,"%i ", &i);
  for (int k=0; k<i; k++) {
    int ll = 0;
    fscanf(fp,"%i ",&ll);
    dag_nodes_given_assignments.insert(ll);
  }
  fscanf(fp,"%i ", &i);
  for (int k=0; k<i; k++) {
    int ll = 0;
    fscanf(fp,"%i ",&ll);
    subgraph_finished.insert(ll);
  }
}




