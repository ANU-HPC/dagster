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


#ifndef TABLE_MASTER_H_
#define TABLE_MASTER_H_

#include <iostream>
#include <iterator>
#include <vector>
#include <deque>
#include <unordered_set>

#include "Dag.h"
#include "SolutionsInterface.h"

/** A hash function for a std::vector<int> */
struct VectorHash {
  size_t operator()(const std::vector<int> &v) const {
    std::hash<int> hasher;
    size_t seed = 0;
    for (int i : v) {
      seed ^= hasher(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }
};



// TableSolutions:
// a simple organiser of messages to/from/between nodes of a dag, stored as 'tables' of messages
// wraps the SolutionsInterface to inherit some superfiscail pre-post processing
class TableSolutions : public WrappedSolutionsInterface {
 private:
  Cnf** additional_clauses; // stores the set of additional clauses for each node of the dag
  bool dumb_mode;
  Dag *dag;                                                // a reference to the appropriate DAG
  std::vector<std::vector<std::vector<Message>>> messages; // a collection of messages indexed by from,to
  std::vector<std::unordered_set<std::vector<int>, VectorHash>> completed_combinations; // a collection of matched sequences of messages allready done
                                                                     // for each node, a set of messages (lists of literal assignments)
  bool _get_combination(int node, std::vector<int> &message_indexes, int reverse_connection_index);

 public:
  TableSolutions(Dag *dag, bool dumb_mode); // creates a new master instance, passing the appropriate dag structure.
  virtual ~TableSolutions();
  virtual void add_message(Message* m); // add a message to the Master's memory, stored by message's to and from fields
  virtual Message *get_new_message_combination(int depth); // for a specified depth, get a new message combination if there is one.
  virtual int** get_additional_clauses(int node);
  virtual void register_message_completion(Message* m);
  
  virtual void dump_checkpoint(FILE* fp);
  virtual void load_checkpoint(FILE* fp);
};

#endif // TABLE_MASTER_H_
