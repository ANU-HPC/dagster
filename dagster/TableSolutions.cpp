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


#include "Message.h"
#include "TableSolutions.h"
#include "utilities.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <glog/logging.h>
#include "utilities.h"
#include "exceptions.h"
#include "mpi_global.h"
#include <set>
#include <vector>
#include <iterator>
using namespace std;

// Constructor of TableSolutions:
//   for a given DAG structure, initialise all tables according to sizes
TableSolutions::TableSolutions(Dag *new_dag, bool dumb_mode) : WrappedSolutionsInterface(new_dag) {
  if (new_dag == NULL)
    throw BadParameterException("cannot initialise master with NULL dag");
  dag = new_dag;
  this->dumb_mode = dumb_mode;
  // size all data structures
  messages.resize(dag->no_nodes);
  completed_combinations.resize(dag->no_nodes);
  for (int i = 0; i < dag->no_nodes; i++)
    messages[i].resize(dag->no_nodes);
  // create all additional clause holders
  TEST_NOT_NULL(additional_clauses = (Cnf**)calloc(sizeof(Cnf*),dag->no_nodes))
  for (int i = 0; i < dag->no_nodes; i++)
    additional_clauses[i] = new Cnf();
}

// Destructor of TableSolutions: free data structures
TableSolutions::~TableSolutions() {
  for (int i = 0; i < dag->no_nodes; i++)
    delete additional_clauses[i];
  free(additional_clauses);
}

// get_additional_clauses(node):
//   return a copy of int** arrays of clauses additional to computation on a specified node
int** TableSolutions::get_additional_clauses(int node) {
  if (additional_clauses[node]->cc==0)
    return NULL;
  int** new_additional_clauses;
  TEST_NOT_NULL(new_additional_clauses = (int**)calloc(sizeof(int*),additional_clauses[node]->cc+1))
  for (int i=0; i< additional_clauses[node]->cc; i++)
    TEST_NOT_NULL(new_additional_clauses[i] = (int*)calloc(sizeof(int),additional_clauses[node]->cl[i]+1))
  for (int i=0; additional_clauses[node]->clauses[i]; i++)
    for (int j=0; additional_clauses[node]->clauses[i][j]; j++)
      new_additional_clauses[i][j] = additional_clauses[node]->clauses[i][j];
  return new_additional_clauses;
}

// add_message(Message m):
//   add a message to the messages pile to proccess in terms of to-and-from nodes, checking to make sure everything is realistic
void TableSolutions::add_message(Message* m) {
  if ((m->from >=0) && (m->to >= 0) && (m->from <dag->no_nodes) && (m->to <dag->no_nodes)) {
    // sort the message and do a duplication check
    sort(m->assignments.begin(), m->assignments.end(), compareAbs);
    for (auto it = messages[m->from][m->to].begin(); it != messages[m->from][m->to].end(); it++)
      if (it->assignments == m->assignments)
        return;
    messages[m->from][m->to].push_back(*m);
  } else {
    throw BadParameterException("TableSolutions adding message that doesn't make sense");
  }
}

// register_message_completion(Message m):
//   for a solution message comming from m->from, add the negation of its litterals to the appropriate additional_clause holders
//   so that subsequent evaluations of the node avoids thoes solutions already registered
//   note: needs to perform a duplicate check.
void TableSolutions::register_message_completion(Message* m) {
  if ((m->from < 0) || (m->from >= dag->no_nodes))
    throw BadParameterException("register completion with badly formed message");
  sort(m->assignments.begin(), m->assignments.end(), compareAbs);
  // negate the message into array 'arr'
  int n = m->assignments.size();
  int arr[n+1];
  int j = 0;
  for (int i = 0; i < n; i++)
    arr[j++] = -m->assignments[i];
  arr[j] = 0;
  if (j>0) {
    // directly check uniqueness
    for (int ci=0; ci<additional_clauses[m->from]->cc; ci++) {
      bool identical = true;
      for (int cj=0; additional_clauses[m->from]->clauses[ci][cj]!=0; cj++) {
        if (additional_clauses[m->from]->clauses[ci][cj]!=arr[cj]) {
          identical = false;
          break;
        }
      }
      if (identical == true) {
        return;
      }
    }
    // if unique then add
    additional_clauses[m->from]->add_clause(arr);
  }
}


// Message* get_new_message_combination(int depth):
//   returns a new combination message for a node at a given depth in the dag
//   does this by setting up a search for comatable message combinations from incomming arcs in the dag
//   recursive inner function is _get_combination(...)
//   return either a new message, OR NULL for no new messages at this depth
Message *TableSolutions::get_new_message_combination(int depth) {
  if ((depth <0) || (depth>dag->max_depth))
    throw BadParameterException("master called get_new_message combination with bad depth");
  // setup some temporary space for the search recursion
  vector<int> message_indexes;
  vector<vector<int>> literal_assignments;
  literal_assignments.clear();
  
  // for each node, if it is at the depth we are working at, we will consider it
  for (int i = 0; i < dag->no_nodes; i++) {
    if (dag->depth_for_each_node[i] == depth) {
      VLOG(5) << "inspecting node " << i;

      // reset temporary space for searching of compatable messages to the node under consideration
      message_indexes.clear();
      message_indexes.resize(dag->reverse_connections[i].size());

      // trigger the recursive search for compatable message
      VLOG(5) << "beginning recursive";
      bool ret = _get_combination(i, message_indexes, 0);
      VLOG(5) << "end recursive";

      // if compatable message combination found, dump message assignments into an vector, and format the new message to the node
      if (ret == true) {
        for (int reverse_connection_index=0; reverse_connection_index<dag->reverse_connections[i].size(); reverse_connection_index++) {
          int src = dag->reverse_connections[i][reverse_connection_index];
          literal_assignments.push_back(messages[src][i][message_indexes[reverse_connection_index]].assignments);
        }
        Message *m = new Message(i, dag->reverse_connections[i][0]); // create a new message to return
        resolve_sorted_vectors(m->assignments, literal_assignments);
        VLOG(5) << "returning new message";
        return m;
      }
      // otherwise proceed ot next compatable node.
    }
  }
  VLOG(5) << "no new messages, returning NULL";
  return NULL; // return NULL indicating that all messages are taken care-of at this depth.
}


// _get_combinatition(dst, message_indexes, reverse_connection_index):
//   inner recursive function to scan through all reverse connections from node <dst> to find compatable messages non-conflicting literal assignments.
//
//   dst is destination node of the messages, and reverse_connection_index is the index of the reverse_connection being scanned
//   message_indexes holds the indices of the messages to be combined.
//   returning true if has found consistent combination, false otherwise
//
bool TableSolutions::_get_combination(int dst, vector<int> &message_indexes, int reverse_connection_index) {
  // for the current reverse_connction_index, get the node index
  int src = dag->reverse_connections[dst][reverse_connection_index];
  VLOG(5) << "begining recursive from node " << src;

  // scan through all messages addressed to dst from this src reverse_connection node
  for (int message_index = 0; message_index < messages[src][dst].size(); message_index++) {
    Message &mess = messages[src][dst][message_index];
    VLOG(5) << "loading " << mess;
    
    if (!dumb_mode) { // check if the message has conflicting literals with thoes messages already indexed.
      for (auto &mess_assignment : mess.assignments)  {// for each literal assignment in the message under consideration
        for (int reverse_conn_index = 0; reverse_conn_index < reverse_connection_index; reverse_conn_index++) {
          int reverse_src = dag->reverse_connections[dst][reverse_conn_index];
          Message* reverse_connection_message = &(messages[reverse_src][dst][message_indexes[reverse_conn_index]]);
          for (auto &reverse_assignment : reverse_connection_message->assignments) { // for each of the literal assignments belonging to each reverse_connection
            if (mess_assignment == -reverse_assignment) { // if conflict then skip to next message
              goto next_message;
            }
          }
        }
      }
      VLOG(5) << "no conflict literals";
    }
    message_indexes[reverse_connection_index] = message_index;

    VLOG(5) << "loaded literals";
    // check that there is another reverse_connection to check the messages of, and pass execution to inspecting its messages if so
    if (reverse_connection_index + 1 < dag->reverse_connections[dst].size()) {
      VLOG(5) << "more reverse_connections to handle";
      bool ret = _get_combination(dst, message_indexes, reverse_connection_index + 1);
      if (ret == true) {
        return ret;
      } else { // reached a conflict somewhere down the line, backing up to try a new message
        continue;
      }
    } else { // otherwise we have reached a coherent message amalgamation.
      VLOG(5) << "no more reverse_connections to handle";
      if (completed_combinations[dst].count(message_indexes) > 0) { // combination allready tried, backing up to try a new message
      } else { // for a new amalgamation, add it to completed amalgamations
        completed_combinations[dst].emplace(message_indexes); // now, given that we have a novel combination amalgam of messages, simply return true to flag this fact.
        return true;
      }
    }
next_message:;
  }
  // if we have run out of messages without conflicts, then we havnt found coherent amalgam, return false.
  return false;
}


//dump_checkpoint: (DRAFT)
// dump all Table information to open writable file pointer
// suitable for files subsequently loaded by load_checkpoint() method
void TableSolutions::dump_checkpoint(FILE* fp) {
	fprintf(fp,"92529 "); // small randomish code to identify files created by TableSolutions
	fprintf(fp,"%i ", this->dag->no_nodes);
	fprintf(fp,"%i ", this->dumb_mode);
	
	for (int i=0; i<this->dag->no_nodes; i++) {
		for (int j=0; j<this->dag->no_nodes; j++) {
			fprintf(fp,"%lu ", this->messages[i][j].size());
			for (int k=0; k<this->messages[i][j].size(); k++) {
				this->messages[i][j][k].dump_to_file(fp);
			}
			fprintf(fp,"\n");
		}
	}
	for (int i=0; i<this->dag->no_nodes; i++) {
		additional_clauses[i]->output_dimacs(fp);
	}
	for (int i=0; i<this->dag->no_nodes; i++) {
		fprintf(fp,"%lu ",this->completed_combinations[i].size());
		for (auto it = this->completed_combinations[i].begin(); it != this->completed_combinations[i].end(); it++) {
			for (auto a = (*it).begin(); a!=(*it).end(); a++) {
				fprintf(fp,"%i",*a);
				if (a!=(*it).end()-1) {
					fprintf(fp," ");
				}
			}
			fprintf(fp,",");
		}
		fprintf(fp,"\n");
	}
}

//load_checkpoint: (DRAFT)
// load all Table information from a readable file pointer
// suitable for files written by dump_checkpoint() method
void TableSolutions::load_checkpoint(FILE* fp) {
	// loading checkpoint
	int identifier;
	int reads;
	reads = fscanf(fp, "%i ", &identifier);
	CHECK_EQ(reads,1);
	if (identifier != 92529) {
		throw BadParameterException("Cannot load checkpoint not created using TableSolutions interface into TableSolutions interface class");
	}
	int dag_no_nodes;
	reads = fscanf(fp, "%i ", &dag_no_nodes);
	CHECK_EQ(reads,1);
	CHECK_EQ(dag_no_nodes, this->dag->no_nodes);
	int input_dumb_mode;
	reads = fscanf(fp, "%i ", &input_dumb_mode);
	this->dumb_mode = (input_dumb_mode!=0);
	CHECK_EQ(reads,1);
	
	for (int i=0; i<this->dag->no_nodes; i++) {
		for (int j=0; j<this->dag->no_nodes; j++) {
			this->messages[i][j].clear();
			int no_messages;
			if (fscanf(fp,"%i ", &no_messages) != 1)
		      throw BadParameterException(" TableSolutions file corruption");
			for (int k=0; k<no_messages; k++) {
				Message m;
				m.read_from_file(fp);
				this->messages[i][j].push_back(m);
			}
		}
	}
	for (int i=0; i<this->dag->no_nodes; i++) {
		delete additional_clauses[i];
		additional_clauses[i] = new Cnf();
		additional_clauses[i]->load_DIMACS_Cnf(fp);
	}
	for (int i=0; i<this->dag->no_nodes; i++) {
		this->completed_combinations[i].clear();
		int no_combinations;
		reads = fscanf(fp,"%i ",&no_combinations);
		CHECK_EQ(reads,1);
		for (int j=0; j<no_combinations; j++) {
			vector<int> combination;
			combination.clear();
			while (true) {
				int read;
				reads = fscanf(fp,"%i ",&read);
				if (reads!=1) {
					break;
				}
				combination.push_back(read);
			}
			this->completed_combinations[i].emplace(combination);
			fgetc(fp);
		}
	}
}



