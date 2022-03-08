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


#include "Dag.h"

#include <glog/logging.h>
#include <ctype.h>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <sstream>

#include "exceptions.h"
#include "utilities.h"
#include "mpi_global.h"

using namespace std;

Dag::Dag() {
  clear();
}
Dag::~Dag() {
  clear();
}

// load the dag from the supplied filename
Dag::Dag(const char *fname) {
  clear();
  size_t len = 100000;
  char line[len];
  char c;
  FILE *f;

  // load up the file
  if ((f = fopen(fname, "r")) == NULL)
    throw FileFailedException(fname);
  // check valid header
  fgets(line, len, f);
  if (strcmp(line, "DAG-FILE\n") != 0) 
    throw ParsingException(" Invalid Dag-file HEADER\n");
  // load the node number, and resize dag vectors appropriately
  if (fscanf(f, "NODES:%i\n", &no_nodes) != 1)
    throw ParsingException(" Invalid Dag-file NODE_COUNTER\n");
  if (no_nodes <= 0)
    throw ParsingException(" Invalid Dag-file NODE_COUNTER\n");
  forward_connections.resize(no_nodes);
  clause_indices_for_each_node.resize(no_nodes);
  depth_for_each_node.resize(no_nodes);
  amalgamated_forward_connection_literals.resize(no_nodes);
  forward_connection_literals.resize(no_nodes);
  for (int i = 0; i < no_nodes; i++)
    forward_connection_literals[i].resize(no_nodes);

  // checks Graph subheader
  fgets(line, len, f);
  if (strcmp(line, "GRAPH:\n") != 0)
    throw ParsingException(" Invalid Dag-file GRAPH_SUB_HEADER\n");
  // load all the connections (forward and reverse) in the directed graph
  unsigned int from, to;
  int begin = 0;
  int end = 0;
  while ((c = getc(f)) != EOF) {
    if (isdigit(c)) {
      ungetc(c, f);
      if ((fscanf(f, "%u->%u", &from, &to) != 2) || (from >= no_nodes) || (to >= no_nodes) || (getc(f) != ':'))
        throw ParsingException(" Invalid Dag-file connection\n");
      if ((from < 0) || (from >=no_nodes) || (to < 0) || (to >=no_nodes))
        throw ParsingException(" Invalid Dag-file connection, refers to non-existent node\n");
      forward_connections[from].push_back(to);
      c = getc(f);
      if (isdigit(c))
        ungetc(c, f); //TODO: empty parsing
      while (c != '\n') {
        int read_terms = fscanf(f, "%u-%u", &begin, &end);
        if ((read_terms == 1) && (begin>=0)) {
          forward_connection_literals[from][to].insert(begin);
          amalgamated_forward_connection_literals[from].insert(begin);
        } else if ((read_terms == 2) && (begin>=0) && (end>=begin)) {
          for (int i = begin; i < end + 1; i++) {
            forward_connection_literals[from][to].insert(i);
            amalgamated_forward_connection_literals[from].insert(i);
          }
        } else
          throw ParsingException(" Invalid Dag-file interface litterals badly formatted\n");
        c = getc(f);
      }
    } else {
      ungetc(c, f);
      break;
    }
  }

  // checks the Clause subheader
  fgets(line, len, f);
  if (strcmp(line, "CLAUSES:\n") != 0)
    throw ParsingException(" Invalid Dag-file CLAUSES_SUB_HEADER\n");
  // iteratively read what clauses are attached to each node
  int node = 0;
  while ((c = getc(f)) != EOF) {
    if (isdigit(c)) {
      ungetc(c, f);
      if (fscanf(f, "%u", &node) != 1)
        throw ParsingException(" Invalid Dag-file CLAUSES badly formatted\n");
      c = getc(f);
      if ((c != ':') || (node >= no_nodes) || (node <0))
        throw ParsingException(" Invalid Dag-file CLAUSES badly formatted\n");
      while (c != '\n') {
        int read_terms = fscanf(f, "%u-%u", &begin, &end);
        if ((read_terms == 1) && (begin>=0)) {
          clause_indices_for_each_node[node].insert(begin);
        } else if ((read_terms == 2) && (begin>=0) && (end>begin)) {
          for (int i = begin; i < end + 1; i++)
            clause_indices_for_each_node[node].insert(i);
        } else
          throw ParsingException(" Invalid Dag-file CLAUSES badly formatted\n");
        c = getc(f);
      }
    } else {
      ungetc(c, f);
      break;
    }
  }
  // check the Reporting subheader
  fgets(line, len, f);
  if (strcmp(line, "REPORTING:\n") != 0)
    throw ParsingException("Invalid Dag-file REPORTING_SUBHEADER\n");
  while ((c = getc(f)) != EOF) {
    if (!isdigit(c))
      throw ParsingException("Invalid Dag-file REPORTING badly formatted\n");
    ungetc(c, f);
    while (c != '\n') {
      int read_terms = fscanf(f, "%u-%u", &begin, &end);
      if ((read_terms == 1) && (begin>=1)) {
        reporting.insert(begin);
      } else if ((read_terms == 2) && (begin>=1) && (end>begin)) {
        for (int i = begin; i < end + 1; i++) {
          reporting.insert(i);
        }
      } else
        throw ParsingException(" Invalid Dag-file REPORTING body badly formatted\n");
      c = getc(f);
    }
  }
  fclose(f);
  generate_from_minimal();
}


void Dag::clear() {
  no_nodes = 0;
  forward_connections.clear();
  reverse_connections.clear();
  clause_indices_for_each_node.clear();
  depth_for_each_node.clear();
  forward_connection_literals.clear();
  amalgamated_forward_connection_literals.clear();
  node_status.clear();
  subgraph_index.clear();
  reporting.clear();
  max_depth = 0;
}


// if all of (no_nodes, reporting, clause_indices_for_each_node, forward_connections, forward_connection_literals) are specified, fill out the rest of the details
void Dag::generate_from_minimal() {
  reverse_connections.clear();
  depth_for_each_node.clear();
  amalgamated_forward_connection_literals.clear();
  node_status.clear();
  subgraph_index.clear();

  reverse_connections.resize(no_nodes);
  depth_for_each_node.resize(no_nodes);
  amalgamated_forward_connection_literals.resize(no_nodes);
  node_status.resize(no_nodes);
  subgraph_index.resize(no_nodes);
  
  for (int i=0; i<no_nodes; i++) {
    for (auto it = forward_connections[i].begin(); it != forward_connections[i].end(); it++) {
      reverse_connections[*it].push_back(i);
      for (auto it3 = forward_connection_literals[i][*it].buffer.begin(); it3!=forward_connection_literals[i][*it].buffer.end(); it3++) {
        for (int c = (*it3).first; c<=(*it3).second; c++) {
          amalgamated_forward_connection_literals[i].insert(c);
        }
      }
    }
  }
  // conduct all additional consistency checks.
  check_consistency();
  assign_depths_and_status();
}



// create the DAG class form a series of ints, generated by dehydrate process
Dag::Dag(int* dehydrated_data) {
  clear();
  int size = dehydrated_data[0];
  int upto = 1;
  int stage = 0;
  int counter = 0;
  int to,from;
  while (upto<size) {
    if (stage==0) {
      no_nodes = dehydrated_data[upto++];
      forward_connections.resize(no_nodes);
      clause_indices_for_each_node.resize(no_nodes);
      forward_connection_literals.resize(no_nodes);
      for (int i = 0; i < no_nodes; i++)
        forward_connection_literals[i].resize(no_nodes);
      stage = 1;
    } else if (stage == 1) {
      int d = dehydrated_data[upto++];
      if (d!=-1)
        reporting.insert(d);
      else
        stage = 2;
    } else if (stage == 2) {
      int d = dehydrated_data[upto++];
      if (d!=-1)
        clause_indices_for_each_node[counter].insert(d);
      else
        counter++;
        if (counter>=no_nodes) {
          stage = 3;
          counter = 0;
        }
    } else {
      if (counter == 0) {
        from = dehydrated_data[upto++];
        counter++;
      } else if (counter == 1) {
        to = dehydrated_data[upto++];
        forward_connections[from].push_back(to);
        counter++;
      } else {
        int d = dehydrated_data[upto++];
        if (d!=-1)
          forward_connection_literals[from][to].insert(d);
        else {
          counter = 0;
        }
      }
    }
  }
  generate_from_minimal();
}


// returns the size of the dehydrated string (in ints)
int Dag::get_dehydrated_size() {
  int upto = 1;
  upto++;
  for (auto it = reporting.buffer.begin(); it!=reporting.buffer.end(); it++) {
    for (int c = (*it).first; c<=(*it).second; c++) {
      upto++;
    }
  }
  upto++;
  for (int i=0; i<no_nodes; i++) {
    for (auto it = clause_indices_for_each_node[i].buffer.begin(); it != clause_indices_for_each_node[i].buffer.end(); it++) {
      for (int c = (*it).first; c<=(*it).second; c++) {
        upto++;
      }
    }
    upto++;
  }
  for (int i=0; i<no_nodes; i++)
    for (auto it2 = forward_connections[i].begin(); it2 != forward_connections[i].end(); it2++) {
      upto++;
      upto++;
      for (auto it3 = forward_connection_literals[i][*it2].buffer.begin(); it3 != forward_connection_literals[i][*it2].buffer.end(); it3++) {
        for (int c = (*it3).first; c<=(*it3).second; c++) {
          upto++;
        }
      }
      upto++;
    }
  return upto;
}

// convert the DAG into a linear array of integers
int Dag::dehydrate(int *output_data) {
  int upto = 1;
  output_data[upto++] = no_nodes;
  for (auto it = reporting.buffer.begin(); it!=reporting.buffer.end(); it++) {
    for (int c = (*it).first; c<=(*it).second; c++) {
      output_data[upto++] = c;
    }
  }
  output_data[upto++] = -1;
  for (int i=0; i<no_nodes; i++) {
    for (auto it = clause_indices_for_each_node[i].buffer.begin(); it != clause_indices_for_each_node[i].buffer.end(); it++) {
      for (int c = (*it).first; c<=(*it).second; c++) {
        output_data[upto++] = c;
      }
    }
    output_data[upto++] = -1;
  }
  for (int i=0; i<no_nodes; i++)
    for (auto it2 = forward_connections[i].begin(); it2 != forward_connections[i].end(); it2++) {
      output_data[upto++] = i;
      output_data[upto++] = *it2;
      for (auto it3 = forward_connection_literals[i][*it2].buffer.begin(); it3 != forward_connection_literals[i][*it2].buffer.end(); it3++) {
        for (int c = (*it3).first; c<=(*it3).second; c++) {
          output_data[upto++] = c;
        }
      }
      output_data[upto++] = -1;
    }
  output_data[0] = upto;
  return upto;
}






int Dag::_recursive_forward(int i, int depth, vector<vector<int>> &forward) {
  if (depth > MAX_DAG_DEPTH)
    return -1;
  int counts = 1;
  for (auto it = forward[i].begin(); it != forward[i].end(); it++) {
    int temp;
    temp = _recursive_forward(*it, depth + 1, forward);
    if (temp == -1)
      return -1;
    counts += temp;
  }
  return counts;
}

bool Dag::check_all_unique(const vector<int> &v) {
  std::set<int> all_clauses;
  for (int i = 0; i < v.size(); i++) {
    if (all_clauses.count(v[i]) > 0)
      return false;
    else
      all_clauses.insert(v[i]);
  }
  return true;
}

// do some basic sanity checking.
void Dag::check_consistency() {
  // checks no duplicate forward connections
  for (auto it = forward_connections.begin(); it != forward_connections.end(); it++)
    if (!check_all_unique(*it))
      throw ConsistencyException(" Invalid Dag-file has repeated forward connections!\n");
  // checks no duplicate reverse connections
  for (auto it = reverse_connections.begin(); it != reverse_connections.end(); it++)
    if (!check_all_unique(*it))
      throw ConsistencyException(" Invalid Dag-file has repeated reverse connections!\n");
  // checks that the graph is acyclic
  // by selecting the nodes and traversing all the ways forward
  // and then checking if infinite loop is encountered (is rough coding)
  for (int i = 0; i < no_nodes; i++) {
    if (_recursive_forward(i, 0, forward_connections) == -1)
      throw ConsistencyException(" Invalid Dag-file is cyclic! or too big!\n");
  }
}

// assign a depth to each node such that a forwardly connected node always has a greater depth.
// also detect disjoint graphs and allocate them different subgraph indices
void Dag::assign_depths_and_status() {
  //initially unassign the depth of all nodes but node zero - with an assumed initial depth of zero.
  for (auto it = depth_for_each_node.begin(); it != depth_for_each_node.end(); it++)
    *it = INT_MIN;
  for (auto it = subgraph_index.begin(); it!=subgraph_index.end(); it++)
    *it = INT_MIN;
  int sum_allocated = 0;
  int subgraph_index_upto = 0;
  max_depth = 0;
  while (sum_allocated < no_nodes) {
    int min_allocated_depth = 0;
    int max_allocated_depth = 0;
    // choose an unassigned node and allocated it with depth of zero
    for (int i=0; i<no_nodes; i++) {
      if (depth_for_each_node[i] == INT_MIN) {
        depth_for_each_node[i] = 0;
        subgraph_index[i] = subgraph_index_upto;
        sum_allocated += 1;
        break;
      }
    }
    int temp;
    int allocated = 1;
    // scan through the nodes adding +1 and -1 depth node relations until no more allocation happens.
    // making sure that nodes are allocated the lowest possible depth with +1 -1 relations.
    while (allocated > 0) {
      allocated = 0;
      for (int i = 0; i < no_nodes; i++) {
        if ((depth_for_each_node[i] != INT_MIN) && (subgraph_index[i] == subgraph_index_upto)) {
          for (auto it = forward_connections[i].begin(); it != forward_connections[i].end(); it++) {
            if (depth_for_each_node[*it] == INT_MIN) {
              temp = depth_for_each_node[i] + 1;
              depth_for_each_node[*it] = temp;
              subgraph_index[*it] = subgraph_index[i];
              allocated++;
              sum_allocated++;
              if (temp > max_allocated_depth)
                max_allocated_depth = temp;
            }
          }
          for (auto it = reverse_connections[i].begin(); it != reverse_connections[i].end(); it++) {
            if (depth_for_each_node[*it] == INT_MIN) {
              sum_allocated++;
            }
            temp = depth_for_each_node[i] - 1;
            if ((depth_for_each_node[*it] > temp) || (depth_for_each_node[*it] == INT_MIN)) {
              depth_for_each_node[*it] = temp;
              subgraph_index[*it] = subgraph_index[i];
              allocated++;
              if (temp < min_allocated_depth)
                min_allocated_depth = temp;
            }
          }
        }
      }
    }
    //normalise so that the minimum node depth is one
    for (auto it = depth_for_each_node.begin(); it != depth_for_each_node.end(); it++)
      if (subgraph_index[it-depth_for_each_node.begin()] == subgraph_index_upto)
        *it -= min_allocated_depth;
    //set max_depth
    if (max_allocated_depth - min_allocated_depth > max_depth) {
      max_depth = max_allocated_depth - min_allocated_depth;
    }
    subgraph_index_upto++;
  }

  //set status
  for (int i = 0; i < no_nodes; i++) {
    if (reverse_connections[i].size() == 0) {
      node_status[i] = 1;
      reverse_connections[i].push_back(i);
    } else {
      node_status[i] = 0;
    }
  }
}


// debug print loaded DAG information
void Dag::print() {
  printf("DAG:\n");
  printf("no_nodes: %i\n",no_nodes);
  printf("max_depth: %i\n",max_depth);
  printf("node_status: ");
  for (auto it = node_status.begin(); it!=node_status.end(); it++) {
    printf("%i ",*it);
  }
  printf("\n");
  printf("forward connections:\n");
  for (auto it = forward_connections.begin();it!=forward_connections.end(); it++) {
    printf("%i: ",it-forward_connections.begin());
    for (auto it2 = (*it).begin(); it2 != (*it).end(); it2++) {
      printf("%i ",*(it2));
    }
    printf("\n");
  }
  printf("reverse connections:\n");
  for (auto it = reverse_connections.begin();it!=reverse_connections.end(); it++) {
    printf("%i: ",it-reverse_connections.begin());
    for (auto it2 = (*it).begin(); it2 != (*it).end(); it2++) {
      printf("%i ",*(it2));
    }
    printf("\n");
  }
  printf("forward connection literals:\n");
  for (auto it = forward_connection_literals.begin();it!=forward_connection_literals.end(); it++) {
    for (auto it2 = (*it).begin(); it2 != (*it).end(); it2++) {
      printf("%i->%i: ",it-forward_connection_literals.begin(),it2-(*it).begin());
      (*it2).print();
    }
  }
  printf("amalgamated forward connection literals:\n");
  for (auto it = amalgamated_forward_connection_literals.begin();it!=amalgamated_forward_connection_literals.end(); it++) {
    printf("%i: ",it-amalgamated_forward_connection_literals.begin());
    (*it).print();
  }
  printf("clause_indices_for_each_node:\n");
  for (auto it = clause_indices_for_each_node.begin();it!=clause_indices_for_each_node.end(); it++) {
    printf("%i: ",it-clause_indices_for_each_node.begin());
    (*it).print();
  }
  printf("depth for each node:\n");
  for (auto it = depth_for_each_node.begin(); it!=depth_for_each_node.end(); it++) {
    printf("%i:%i\n",it-depth_for_each_node.begin(), *it);
  }
  printf("Reporting:\n");
  reporting.print();
}







