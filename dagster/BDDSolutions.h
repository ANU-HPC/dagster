/*************************
Copyright 2020 Eamon Barett

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


#ifndef BDD_MASTER_H_
#define BDD_MASTER_H_

#include "Dag.h"
#include "cudd.h"
#if __has_include("dddmp.h")
#include "dddmp.h"
#endif

#include <iostream>
#include <iterator>
#include <vector>
#include <unordered_map>
#include <fstream>
#include "SolutionsInterface.h"


class BDDSolutions : public WrappedSolutionsInterface {
 public:
  enum  BDD_compilation_scheme {cubes,minisat,paths};
  void set__BDD_compilation_scheme(const std::string& scheme);
  BDD_compilation_scheme get__BDD_compilation_scheme() const;
  
  int* max_depth; // points to dag max depth
  int vc;
  BDDSolutions::BDD_compilation_scheme bdd_compilation_scheme;
  
  Dag* dag;
  DdManager* ddmgr;                 // the CUDD manager for decision diagrams
  std::vector<std::vector<DdNode*>> bdds;  // a table of BBDs for incoming assignments indexed by to, from
  std::vector<DdNode*> communicated; // each node has a bdd for all previous sent assignments
  std::vector<Message*> initial_messages; // start with an empty message to each base node
  std::vector<DdNode*> completed; // each node has a bdd for all previous sent assignments

 public:
  BDDSolutions(Dag *dag, int vc); // creates a new master instance, passing the appropriate dag structure, and the variable count of the master problem (for aux var index start).
  ~BDDSolutions();
  virtual void add_message(Message* m); // add a message to the Master's memory, stored by message's to and from fields
  void readd_message(Message* M);       // add a message back to master's memory
  virtual Message *get_new_message_combination(int depth); // for a specified depth, get a new message combination if there is one.
  int** compile_negated_cnf(int node); // compile the node's bdd as cnf, used when a new worker starts at the node
  int** encode_negated_cnf(int node); // compile the node's completed bdd as cnf as described by the MiniSAT encoding in Abio, 2016.
                        // the first int* includes the number of variables and number of clauses
  int** path_based_encode_negated_cnf(int node); // compile cnf of negated completed[node] bdd using CompletePath encoding from Abio
  void print_cnf(int** additional_clauses, std::string fname);
  void print_three_negated_cnfs(int node, int num_models);  // print the cnf of the negated completed[node] bdd using each of the 3 methods 
  virtual void print_stats(bool print_all_bdds, bool full) override;
  void print_bdds(FILE* fp, int to);
  void print_completed(FILE* fp, int node);
  void print_completed_bdd(FILE*, int node);
  void print_communicated(FILE* fp, int node);
  void reorder_bdds();
  virtual int** get_additional_clauses(int node);
  virtual void register_message_completion(Message* m);
  
  virtual void dump_checkpoint(FILE* fp);
  virtual void load_checkpoint(FILE* fp);
};

#endif // BDD_MASTER_H_
