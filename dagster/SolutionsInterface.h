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


#ifndef MASTER_INTERFACE_H_
#define MASTER_INTERFACE_H_

#include "exceptions.h"
#include "Message.h"
#include "Dag.h"
#include <algorithm>
#include <vector>

//SolutionsInterface:
// a minimal virtual class of functions that will be the outward presenting face of any class stores and handles solutions and messages to/from SAT solving workers in a dag structure
class SolutionsInterface {
  public:
  // INPUT_MESSAGE(): message 'm' is a solution from a SAT solver working on a node of the dag, input it into this organiser, do processing and setup new messages bassed apon it, returning the number of new messages generated from it.
  virtual int input_message(Message* m)=0;
  // OUTPUT_MESSAGE(): pull a new message from this organiser that a SAT solver will then compute, flag whether the message should be pulled depth/breadth first in the dag
  virtual Message *output_message(bool BREADTH_FIRST_NODE_ALLOCATIONS)=0;
  // GET_ADDITIONAL_CLAUSES(): get int** array of additional clauses that is relevent to computations on <node>
  virtual int** get_additional_clauses(int node)=0;
  
  virtual void dump_checkpoint(FILE* fp)=0;
  virtual void load_checkpoint(FILE* fp)=0;
};



// WrappedSolutionsInterface:
//  inherits SolutionsInterface and provides some superficial pre-post-processing around splitting messages up to further connections in the dag and message counting
//  is an abstract class that other classes will inherit and override message storing/resolving mechanisms
class WrappedSolutionsInterface : public SolutionsInterface {
  private:
  Dag* dag;
  std::vector<int> incoming_message_count; // counts of input/output messages from each node
  std::vector<int> outgoing_message_count;
  protected:
  WrappedSolutionsInterface(Dag *dag) {
    incoming_message_count.resize(dag->no_nodes);
    outgoing_message_count.resize(dag->no_nodes);
    this->dag = dag;
  };
  
  public:
  virtual ~WrappedSolutionsInterface() {}
  virtual void register_message_completion(Message* m)=0;
  virtual void add_message(Message* m)=0; // add a message to the memory, stored by message's to and from fields
  virtual void print_stats(bool file, bool full) {};
  virtual Message *get_new_message_combination(int depth)=0; // for a specified depth of the dag, get a new message combination if there is one.
  virtual int** get_additional_clauses(int node)=0; //pass on virtual method from SolutionsInterface
  virtual void dump_checkpoint(FILE* fp)=0;
  virtual void load_checkpoint(FILE* fp)=0;

  void register_completion(Message* m) {
    // registers for when a message is a solution from a particular nodes
    // need to make sure only the necessary variables are being added
    
    // only add the variable if it is on the arc in the dag
    // look for i in amalgamated_forward_connection_literals
    // if i is not there, continue

    // if the node has no forward connections, it is the final
    // node and should record the assignments to the important variables
    // - the reporting variables in the dag
    
    if (dag->forward_connections[m->from].size() == 0) {
      for (int i=m->assignments.size()-1; i>= 0; i--) {
        if (!dag->reporting.find(abs(m->assignments[i]))) {
          m->assignments.erase(m->assignments.begin()+i);
        }
      }
    } else {
      for (int i=m->assignments.size()-1; i>= 0; i--) {
        if (!dag->amalgamated_forward_connection_literals[m->from].find( abs(m->assignments[i]) ) ) {
          m->assignments.erase(m->assignments.begin()+i);
        }
      }
    }
    register_message_completion(m);
  }
  
  int input_message(Message* m) {
    // given a message m that is a solution from a SAT solver at a node in the dag, 
    // register it as complete, and split it into new seeding messages for further nodes 
    if (m==NULL)
      throw BadParameterException("WrappedSolutionsInterface input_message called with NULL message");
    register_completion(m); // register it as solution
    Message temp_m;
    for (int j = 0; j < dag->forward_connections[m->from].size(); j++) { // otherwise send to all of the forward connections, purging unnessisary variables along the arcs
      temp_m.set(m);
      temp_m.to = dag->forward_connections[m->from][j];
      temp_m.purge_variables(dag->forward_connection_literals[temp_m.from][temp_m.to]);
      outgoing_message_count[temp_m.from]++; 
      add_message(&temp_m);
    }
    return dag->forward_connections[m->from].size();
  }
  
  Message *output_message(bool BREADTH_FIRST_NODE_ALLOCATIONS) {
    // get a message from the pile of uncomputed messages, 
    // flag indicates whether message should be given depth/breadth first from among dag nodes
    int depth = (BREADTH_FIRST_NODE_ALLOCATIONS ? 0 : dag->max_depth);
    while (BREADTH_FIRST_NODE_ALLOCATIONS ? depth <= dag->max_depth : depth >= 0) { // progressively increase the depth looking for maximum work for our workers
      Message *mess = get_new_message_combination(depth);                // generate an assignment
      if (mess == NULL) {
        depth += (BREADTH_FIRST_NODE_ALLOCATIONS ? 1 : -1);
      } else {  // for each new assignment found add it to the buffer, and create vectors for workers to be assigned to work on the message
        incoming_message_count[mess->to]++;
        return mess;
      }
    }
    return NULL;
  }
  
  int get_incoming_message_count(int node) {
    return incoming_message_count[node];
  }
  int get_outgoing_message_count(int node) {
    return outgoing_message_count[node];
  }
  
};

#endif // MASTER_INTERFACE_H_
