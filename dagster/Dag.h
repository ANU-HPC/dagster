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


#ifndef _DAG
#define _DAG

#include "Cnf.h"

#include "mpi_global.h"
#include "RangeSet.h"

#include <vector>
#include <set>

#define MAX_DAG_DEPTH 10000

class Dag {
private:
  int _recursive_forward(int i, int depth, vector<vector<int>> &forward);
  bool check_all_unique(const vector<int> &v);
public:
  int no_nodes;
  //number of nodes on the dag
  std::vector<int> node_status;
  //number of nodes on the dag
  std::vector<int> subgraph_index;
  // the status of each of the nodes (0= normal node, 1=base node)
  std::vector<std::vector<int>> forward_connections;
  // the forward connections between the nodes
  std::vector<std::vector<RangeSet>> forward_connection_literals;
  // the literals of the interfaces of forward connections between the nodes
  std::vector<RangeSet> amalgamated_forward_connection_literals;
  // the literals of the interfaces of forward connections from the nodes
  std::vector<std::vector<int>> reverse_connections;
  // the reverse connections between the nodes
  std::vector<RangeSet> clause_indices_for_each_node;
  // for each node, the list of lengths of its clauses
  int max_depth;
  // the greatest depth in the DAG
  std::vector<int> depth_for_each_node;
  // for each node, the depth which the node is at
  RangeSet reporting; // vector of the important variables in the problem

  void check_consistency();
  void assign_depths_and_status();
  void clear();

  Dag(const char *fname);
  Dag(int* dehydrated_data);
  Dag();
  ~Dag();

  int dehydrate(int *output_data);
  int get_dehydrated_size();
  void print();
  void generate_from_minimal();

};

#endif
