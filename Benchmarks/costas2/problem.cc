/*************************
Copyright 2020 Charles Gretton, Josh Milthorpe

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

#include "global.hh"
#include "problem.hh"
#include <algorithm>
#include <vector>
#include <fstream>
#include <ostream>

int Problem::cnfauxvar(){
  aux.insert(nextcnfvar++);
  cerr<<"MAPPING: "<<nextcnfvar - 1<<" -> auxilliary variable"<<endl;
  return nextcnfvar - 1;
}

int Problem::cnfvar__vertical(int column_index, int height)
{
  tuple<int,int> query(column_index, height);
  auto _cnfvar = rowassignment__to__cnfvar.find(query);
  if(rowassignment__to__cnfvar.end() != _cnfvar){
    return _cnfvar->second;
  }
  
  rowassignment__to__cnfvar[query] = nextcnfvar++;
  cerr<<"MAPPING: "<<nextcnfvar - 1<<" -> c"<<column_index<<" h"<<height<<endl;
  return nextcnfvar - 1;
}

int Problem::cnfvar__horizontal(int row_index, int height){
  tuple<int,int> query(row_index, height);
  auto _cnfvar = colassignment__to__cnfvar.find(query);
  if(colassignment__to__cnfvar.end() != _cnfvar){
    return _cnfvar->second;
  }
  
  colassignment__to__cnfvar[query] = nextcnfvar++;
  cerr<<"MAPPING: "<<nextcnfvar - 1<<" -> r"<<row_index<<" h"<<height<<endl;
  return nextcnfvar - 1;

}

int Problem::cnfvar(int index, int height, string orientation){
  if ( 0 == orientation.compare("VERTICAL") ) {
    return Problem::cnfvar__vertical(index, height);
  } else if ( 0 == orientation.compare("HORIZONTAL") ) {
    return Problem::cnfvar__horizontal(index, height);
  } else {
    cerr<<__LINE__<<" :: FAILURE MODE";
    assert(0);
    exit(-1);
  }
  
  return 0;
}


int Problem::cnfvar__horizontal(int index1, int index2, int distance){
  tuple<int,int,int> query(index1, index2, distance);
  auto _cnfvar = horizontal_distance__to__cnfvar.find(query);
  if( horizontal_distance__to__cnfvar.end() != _cnfvar ){
    return _cnfvar->second;
  }
  
  horizontal_distance__to__cnfvar[query] = nextcnfvar++;
  cerr<<"MAPPING: "<<nextcnfvar - 1<<" -> r"<<index1<<" r"<<index2<<" d"<<distance<<endl;
  return nextcnfvar - 1;
}


int Problem::cnfvar__vertical(int index1, int index2, int distance){
  tuple<int,int,int> query(index1, index2, distance);
  auto _cnfvar = distance__to__cnfvar.find(query);
  if(distance__to__cnfvar.end() != _cnfvar){
    return _cnfvar->second;
  }
  
  distance__to__cnfvar[query] = nextcnfvar++;
  cerr<<"MAPPING: "<<nextcnfvar - 1<<" -> c"<<index1<<" c"<<index2<<" d"<<distance<<endl;
  return nextcnfvar - 1;
}

int Problem::cnfvar(int index1, int index2, int distance, string orientation){
  if ( 0 == orientation.compare("VERTICAL") ) {
    return Problem::cnfvar__vertical(index1, index2, distance);
  } else if ( 0 == orientation.compare("HORIZONTAL") ) {
    return Problem::cnfvar__horizontal(index1, index2, distance);
  } else {
    cerr<<__LINE__<<" :: FAILURE MODE";
    assert(0);
    exit(-1);
  }

  return 0;
}

int Problem::dist_exists(int hdist, int vdist) {
  tuple<int, int> query(hdist, vdist);
  auto _cnfvar = dist__exists__cnfvar.find(query);
  if (dist__exists__cnfvar.end() != _cnfvar) {
    return _cnfvar->second;
  }

  dist__exists__cnfvar[query] = nextcnfvar++;
  cerr << "MAPPING: " << nextcnfvar - 1 << " -> for some x,y such that y = x + " << hdist << ", dist(x,y) = " << vdist << endl;
  return nextcnfvar - 1;
}

/** 
 * Print a DAG for this Costas problem to the named file.
 * If bisect==true, divide the problem into two nodes:
 * the first node solves for alldifferent vertical distances for horizontal
 * distances of 2..(N-1)/2; the second node solves the complete problem given
 * the solutions from the first node as a constraint.
 * If bisect==false, divide the problem into (N-1)/2 - 1 nodes:
 * node M solves for alldifferent vertical distances for horizontal distances
 * of (N-1)/2-M..(N-1)/2 given the solution from node (M-1) as a constraint;
 * as before the final node solves the complete Costas problem.
 */
void Problem::print__dag_file(string filename, bool bisect) {
  int num_levels = (N - 1) / 2;

  // all-different placement clauses (N*(N-1)/2 + 1) per column and row (2*N)
  int num_placement_clauses = (N * (N - 1) / 2 + 1) * 2 * N;

  ofstream dag_file;
  dag_file.open(filename);
  dag_file << "DAG-FILE" << endl;
  dag_file << "NODES:" << num_levels << endl;
  dag_file << "GRAPH:" << endl;

  // print variables for levels in ascending order
  vector<pair<pair<int, int>, set<pair<int, int>>>> sorted_level_vars(dag_edge_variable_ranges.size());
  partial_sort_copy(dag_edge_variable_ranges.begin()
		    , dag_edge_variable_ranges.end()

		    , sorted_level_vars.begin()
		    , sorted_level_vars.end()
		    
		    ,[](const pair<pair<int, int>, set<pair<int, int>>> &left
			,const pair<pair<int, int>, set<pair<int, int>>> &right
			)
		    {return (left.first.first < right.first.first);}
		    );

  int max_level = sorted_level_vars.size() - 1;
  int level = 0;
  int print_level = 0;
  bool first = true;
  for (auto &edge_range : sorted_level_vars) {
    ostringstream variables;
    set<pair<int, int>> variable_ranges = edge_range.second;

    auto &range = *(--(variable_ranges.end()));
    variables << range.first << "-" << range.second;
    
    // for (auto &range : variable_ranges) {
    //   if (first)
    //     first = false;
    //   else
    //     variables << ",";
    //   variables << range.first << "-" << range.second;
    // }

      
    if (!bisect || level == max_level) {
      dag_file << print_level << "->" << (print_level+1) << ":";
      dag_file << variables.str() << endl;
      print_level++;
    }
    level++;
  }

  dag_file << "CLAUSES:" << endl;
  // print clauses for levels in ascending order
  vector<pair<int, set<pair<int, int>>>> sorted_levels(dag_node_clause_ranges.size());
  partial_sort_copy(dag_node_clause_ranges.begin()
		    , dag_node_clause_ranges.end()
		    , sorted_levels.begin()
		    , sorted_levels.end()
		    , [](const pair<int, set<pair<int, int>>> &left
			 , const pair<int, set<pair<int, int>>> &right)
		    {return (left.first < right.first);}
		    );

  // every level gets all clauses of the previous level
  level = 0;
  print_level = 0;
  ostringstream clauses;
  clauses << "0-" << (num_placement_clauses - 1);
  for (auto &node_range : dag_node_clause_ranges) {
    set<pair<int, int>> clause_ranges = node_range.second;
    for (auto &range : clause_ranges) {
      clauses << "," << range.first << "-" << range.second;
    }
    if (!bisect || level == max_level) {
      dag_file << print_level++ << ":";
      dag_file << clauses.str() << endl;
    }
    level++;
  }
  
  // final level of dag includes all clauses
  dag_file << print_level << ":"
           << "0-" << (num_clauses - 1) << endl;
  // add reporting
  dag_file << "REPORTING:" << endl;
  dag_file << "1-" << (N*N) << endl;
}


void Problem::print__dag_file_decomposed_by_column_placements(const string& filename, const vector<int>& column_indexes, const vector<int>& column_indexes2)
{
  ofstream dag_file;
  dag_file.open(filename);
  dag_file << "DAG-FILE" << endl;
  dag_file << "NODES:" << 3 << endl;
  dag_file << "GRAPH:" << endl;
  
  dag_file << "0->1:";
  for (auto i : column_indexes) {
    for (int j = 0 ; j < N; j++) {
      dag_file << cnfvar(i,j);
      if ( ( i < column_indexes.back() )
	  || (j<N-1) )
        dag_file << ",";
    }
  }
  dag_file << endl;
  dag_file << "1->2:";
  {
    auto _column_indexes = column_indexes2;//column_indexes;
    //for (auto i :column_indexes2) _column_indexes.push_back(i);
    sort(_column_indexes.begin(), _column_indexes.end());
    for (auto i : _column_indexes) {
      for (int j = 0 ; j < N; j++) {
	dag_file << cnfvar(i,j);
	if ( ( i < _column_indexes.back() )
	     || (j<N-1) )
	  dag_file << ",";
      }
    }
  }
  dag_file << endl;
  
  dag_file << "CLAUSES:" << endl;
  
  {ostringstream oss;
    auto current = *set_of_clause_indexes.begin();
    auto first_index = current;
    for(auto i : set_of_clause_indexes){
      if(i == current ) continue;
      if( current + 1 == i ) {
	current++; continue;
      }
      if(first_index != current){
	oss<<first_index
	   <<"-"<<current
	   <<",";
      }else {
	oss<<first_index<<",";
      }
      first_index = i;
      current = i;			      
    }
    if(first_index != current){
      oss<<first_index
	 <<"-"<<current;
    }else {
      oss<<first_index;
    }
    dag_file << "0:" << oss.str() << endl;
  }

  {ostringstream oss;
    auto current = *set_of_clause_indexes2.begin();
    auto first_index = current;
    for(auto i : set_of_clause_indexes2){
      if(i == current ) continue;
      if( current + 1 == i ) {
	current++; continue;
      }
      if(first_index != current){
	oss<<first_index
	   <<"-"<<current
	   <<",";
      }else {
	oss<<first_index<<",";
      }
      first_index = i;
      current = i;			      
    }
    if(first_index != current){
      oss<<first_index
	 <<"-"<<current;
    }else {
      oss<<first_index;
    }
    dag_file << "1:" << oss.str() << endl;
  }
  
  
  dag_file << "2:" << 0 << "-" << num_clauses-1 << endl;  
  dag_file << "REPORTING:" << endl;
  dag_file << "1-" << (N*N) << endl;
}

/** 
 * Print a DAG for this Costas problem to the named file.
 */
void Problem::print__dag_file_simple(string filename) {
  ofstream dag_file;
  dag_file.open(filename);
  dag_file << "DAG-FILE" << endl;
  dag_file << "NODES:" << 2 << endl;
  dag_file << "GRAPH:" << endl;
  dag_file << "0->1:";
  for (int i = 0; i<3; i++) {
    for (int j = 0 ; j < N; j++) {
      dag_file << cnfvar(i,j);
      if ((i<2) || (j<N-1))
        dag_file << ",";
    }
  }
  dag_file << endl;
  dag_file << "CLAUSES:" << endl;
  dag_file << "0:" << clause_first_row_begin << "-" << clause_first_row_end << endl;
  dag_file << "1:" << 0 << "-" << num_clauses-1 << endl;  
  dag_file << "REPORTING:" << endl;
  dag_file << "1-" << (N*N) << endl;
}

/** 
 * Print a DAG for this Costas problem to the named file.
 */
void Problem::print__dag_file_super_simple(string filename) {
  ofstream dag_file;
  dag_file.open(filename);
  dag_file << "DAG-FILE" << endl;
  dag_file << "NODES:" << 1 << endl;
  dag_file << "GRAPH:" << endl;
  dag_file << "CLAUSES:" << endl;
  dag_file << "0:" << 0 << "-" << num_clauses-1 << endl;  
  dag_file << "REPORTING:" << endl;
  dag_file << "1-" << (N*N) << endl;
}

