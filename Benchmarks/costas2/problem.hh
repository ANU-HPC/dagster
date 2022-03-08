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

#ifndef PROBLEM_HH
#define PROBLEM_HH


class Problem{
public:
  const std::vector<int> CANDIDATES{1 , 3  , 5};
  
  void print__dag_file_decomposed_by_column_placements(const string&, const vector<int>& column_indexes);
  
  // PROPOSITION -- (if orientation=="VERTICAL") The token at horizontal position $index1$ is at $orientation$ location $height$
  int cnfvar(int index, int height, string orientation=string("VERTICAL"));
  int cnfvar__vertical(int column_index, int height);
  int cnfvar__horizontal(int row_index, int height);
  
  // PROPOSITION -- (if orientation="VERTICAL") The vertical distance between the token in column at $index1$ and the token in column at $index2$ is $distance$
  int cnfvar(int index1, int index2, int distance, string orientation=string("VERTICAL"));
  int cnfvar__vertical(int index1, int index2, int distance);
  int cnfvar__horizontal(int index1, int index2, int distance);

  // PROPOSITION -- The set of vertical distances between pairs of columns at horizontal distance $hdist$ includes the value $vdist$
  int dist_exists(int hdist, int vdist);

  // PROPOSITION -- An auxiliary proposition
  int cnfauxvar();

  void print__dag_file(std::string filename, bool level);
  void print__dag_file_simple(std::string filename);
  void print__dag_file_super_simple(std::string filename);
  map<pair<int,int>, set<pair<int,int>>> dag_edge_variable_ranges;
  map<int, set<pair<int, int>>> dag_node_clause_ranges;

  /*Dimension of Costas array.*/
  int N;
  
  /*Global counter, incremented each time we need a new proposition.*/
  int nextcnfvar = 1;

  /* Global counter, incremented each time we add a clause. */
  int num_clauses = 0;
  
  map<tuple<int/*(column_)index*/,int/*=height*/>, int> rowassignment__to__cnfvar;
  map<tuple<int/*(row_)index*/,int/*=height*/>, int> colassignment__to__cnfvar;
  map<tuple<int/*(row_)index1*/,int/*(row_)index2*/,int/*=dist[i1,i2]*/>, int> horizontal_distance__to__cnfvar;
  map<tuple<int/*(column_)index1*/,int/*(column_)index2*/,int/*=dist[i1,i2]*/>, int> distance__to__cnfvar;
  map<tuple<int /*horizontal dist*/, int /*vertical dist*/>, int> dist__exists__cnfvar;
  set<int> aux;

  /* CNF representation of problem.*/
  ostringstream cnf;
  
  int clause_first_row_begin;
  int clause_first_row_end;

  /**/
  std::set<int> set_of_clause_indexes;
};

#define ADD_IF_TARGET_COL(i,X,Y) {			\
    const vector<int> candidates = problem.CANDIDATES;	\
  bool answer1 = false;					\
  for (auto x : candidates ) if(X==x)answer1 = true;	\
  bool answer2 = false;					\
  for (auto x : candidates ) if(Y==x)answer2 = true;	\
  if(answer2 && answer1){				\
    problem.set_of_clause_indexes.insert(i);			\
  }							\
  }							\
				    
#define ADD_NO_MATTER_WHAT(i) {problem.set_of_clause_indexes.insert(i);}

#endif
