/*************************
Copyright 2020 Charles Gretton

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

#include "constraints.hh"
#include "problem.hh"

extern Problem problem;

int print__at_most_one(const set<int>& vars){
  for(auto _v = vars.begin()
	; _v != vars.end()
	; _v++){
    auto _v_= _v;
    _v_++;
    while(_v_ != vars.end()){
      assert(*_v != *_v_);
      problem.cnf<<-*_v<<" "<<-*_v_<<" 0\n";
      problem.num_clauses++;
      _v_++;
    }
  }

  return -1; // For type compatibility with tree-variant of this constraint.
}

void print__exactly_one(const set<int>& vars){
  // No more than one element in $vars$ 
  print__at_most_one(vars);

  // One element in $vars$ should be true
  ostringstream oss;
  for(auto _v =vars.begin()
	; _v != vars.end()
	; _v++){
    oss<<*_v<<" ";
  }
  oss<<"0\n";
  problem.cnf<<oss.str();
  problem.num_clauses++;
}

void print__tree_exactly_one(const set<int>& vars){
  if ( vars.size() < 4 ) {
    print__exactly_one(vars);
    return;
  }
  
  int last_aux = print__tree_at_most_one(vars);
  assert( last_aux > 0 );
  problem.cnf<<last_aux<<" 0\n";
  problem.num_clauses++;
}

int print__tree_at_most_one(const set<int>& vars){
  if ( vars.size() < 4 ) {
    print__at_most_one(vars);
    return -1;
  }

  // I find that trees are best made, and debugged, with triangles -- lhs, rhs, top -- lhs -> top; rhs -> top; -lhs | -rhs; top -> lhs|rhs
  typedef tuple<int,int,int> Item;
  set<Item> items;
  for(auto _v =vars.begin()
	; _v != vars.end()
	; _v++){
    Item item = make_tuple(*_v /*lhs*/, *_v /*rhs*/, *_v /*top*/);
    items.insert(item);
  }

  assert( 1 < items.size() );

  // NOTE: C. Gretton -- Redundancy below in case I make a mistake and need to debug.
  int last_aux = -1;  
  while( 1 < items.size() ){
    set<Item> new_items; // i.e. shall be $items$ next iterations
    for(auto _v = items.begin()
	  ;  _v != items.end()
	  ;  _v++){
      auto _v_= _v;
      _v_++;
      Item item;
      if(_v_ != items.end()){ // Two consecutive triples must be mutex
	auto aux = problem.cnfauxvar();
	last_aux=aux;
	
	problem.cnf<<"-"<<get<2>(*_v)<<" -"<<get<2>(*_v_)<<" 0\n";
	problem.num_clauses++;
	
	problem.cnf<<"-"<<get<2>(*_v)<<" "<<aux<<" 0\n";
	problem.num_clauses++;
	
	problem.cnf<<"-"<<get<2>(*_v_)<<" "<<aux<<" 0\n";
	problem.num_clauses++;
	
	problem.cnf<<get<2>(*_v)<<" "<<get<2>(*_v_)<<" -"<<aux<<" 0\n";
	problem.num_clauses++;
	
        item = Item(get<2>(*_v)/*lhs*/, get<2>(*_v_)/*rhs*/, aux/*top*/);
      } else { // There is a lone triple without a paired element. Pass this along...
	item = make_tuple(get<2>(*_v),get<2>(*_v),get<2>(*_v)); // Odd number of $items$...
	assert(items.end() == _v_);
	new_items.insert(item);
	break;
      }
      new_items.insert(item);
      assert(items.end() != _v);
      _v = _v_;
    } // for(auto _v = items.begin()

    assert(new_items.size () < items.size() );
    items=new_items;
  }
  
  assert(last_aux != -1);
  return last_aux;
}



void post__clique_edges_have_one_colour_each(int colours, const E& e){
  auto edge_colour_vars = set<int>();
  for(int c = 0; c < colours; c++){
    auto var = problem.cnfvar(EC(e,c));
    edge_colour_vars.insert(var);
  }
  print__tree_exactly_one(edge_colour_vars);
  //print__exactly_one(edge_colour_vars); // TESTING PENDING
}

void post__clique_edges_have_one_colour_each(int colours, int N){
  /*for(int i =0 ; i < N; i++){
    for(int j =i+1 ; j < N; j++){
      post__clique_edges_have_one_colour_each(colours, E(i,j));
    }
  }*/
  // modified for backwards
  for(int i = N-1; i >= 0; i--){
    for(int j =i-1 ; j >= 0; j--){
      if ((i==N-1) && (j==problem.Z-1))
        problem.levNa_begin = problem.num_clauses+1;
      post__clique_edges_have_one_colour_each(colours, E(i,j));
      if ((i==N-1) && (j==0))
        problem.levNa_end = problem.num_clauses+1;
    }
  }
}


int get_unitary_index(vector<int>& unitary_variables, int index) {
  if (index<0) {
    cerr <<"ErROR";
    return 0;
  } else if (index==0) {
    return -unitary_variables[0];
  } else if (index<=unitary_variables.size()) {
    return unitary_variables[index-1];
  } else {
    cerr <<"ErROR";
    return 0;
  }
}

int get_unitary_index_negative(vector<int>& unitary_variables, int index) {
  if (index<0) {
    cerr <<"ErROR";
    return 0;
  } else if (index==0) {
    return unitary_variables[0];
  } else if (index<=unitary_variables.size()) {
    return -unitary_variables[index-1];
  } else {
    cerr <<"ErROR";
    return 0;
  }
}

// basic unitary cardinality constraint
// see: "Efficient CNF encoding of Boolean cardinality constraints" by Olivier Bailleux and and Yacine Boufkhad
vector<int> add_cardinality_constraint(vector<int> variables) {
  if (variables.size()>1) {
    // split the variables into two groups
    float midpoint = variables.size()*1.0/2;
    vector<int> part_a;
    vector<int> part_b;
    for (int i=0; i<variables.size(); i++) {
      if (i<midpoint) {
        part_a.push_back(variables[i]);
      } else {
        part_b.push_back(variables[i]);
      }
    }
    // get the unitary counts for each of the groups
    vector<int> variables_a = add_cardinality_constraint(part_a);
    vector<int> variables_b = add_cardinality_constraint(part_b);
/*problem.cnf<<"from: {";
for (int i=0; i<variables_a.size(); i++)
problem.cnf<<variables_a[i]<<",";
problem.cnf<<"} and {";
for (int i=0; i<variables_b.size(); i++)
problem.cnf<<variables_b[i]<<",";
problem.cnf<<"}:\n";*/
    // combine the unitary counts for the two groups
    vector<int> unitary_variables;
    unitary_variables.clear();
//cerr<<"MAPPING: beginning block"<<endl;
    for (int i = 0; i<variables_a.size()+variables_b.size(); i++) {
//cerr<<"MAPPING: sum: "<<(i+1)<<endl;
      unitary_variables.push_back(problem.cnfauxvar());
    }
    // setup implications
    for (int i=0; i<=variables_a.size(); i++) {
      for (int j=0; j<=variables_b.size(); j++) {
//problem.cnf<<"  "<<i<<", and "<<j<<" -> "<<(i+j)<<"\n";
        problem.cnf<<get_unitary_index_negative(variables_a, i)<<" "<<get_unitary_index_negative(variables_b, j)<<" "<<get_unitary_index(unitary_variables,i+j)<< " 0\n"; //a+b->a+b
	    problem.num_clauses++;
      }
    }
    for (int i=1; i<=variables_a.size(); i++) {
      for (int j=1; j<=variables_b.size(); j++) {
//problem.cnf<<"  NOT "<<i<<", and NOT "<<j<<" -> NOT "<<(i+j-1)<<"\n";
        problem.cnf<<get_unitary_index(variables_a, i)<<" "<<get_unitary_index(variables_b, j)<<" "<<get_unitary_index_negative(unitary_variables,i+j-1)<< " 0\n"; //a+b->a+b
	    problem.num_clauses++;
      }
    }
    for (int i=1; i<=variables_a.size(); i++) {
//problem.cnf<<"  NOT "<<i<<", -> NOT "<<(i+variables_b.size())<<"\n";
      problem.cnf<<get_unitary_index(variables_a, i)<<" "<<get_unitary_index_negative(unitary_variables,i+variables_b.size())<< " 0\n"; //a+0->a
	  problem.num_clauses++;
    }
    for (int j=1; j<=variables_b.size(); j++) {
//problem.cnf<<"  , and NOT "<<j<<" -> NOT "<<(variables_a.size()+j)<<"\n";
      problem.cnf<<get_unitary_index(variables_b, j)<<" "<<get_unitary_index_negative(unitary_variables,j+variables_a.size())<< " 0\n"; //b+0->b
	  problem.num_clauses++;
    }
    
    for (int j=1; j<variables_a.size()+variables_b.size(); j++) {
//problem.cnf<<j+1<<" -> "<<j<<"   aka   NOT "<<j<<" -> NOT "<<(j+1)<<"\n";"\n";
      problem.cnf<<get_unitary_index(unitary_variables,j)<<" "<<get_unitary_index_negative(unitary_variables,j+1)<< " 0\n";
	  problem.num_clauses++;
    }
    return unitary_variables;
  } else {
    return variables;
  }
}




// adding constraints where number of colour zero <= number of colour one <= number of colour two <= ... <= number of colour M-1
void add_colour_quantifier_symmetry_breaking(int colours, int N, vector<vector<int>>& unitary_variables ) {
  vector<int> colour_buffer;
  //vector<vector<int>> unitary_variables;
  // considering all colours
  for(int c=0; c<colours; c++) {
    // load all the variables of that colour into colour buffer
    colour_buffer.clear();
    for(int i =0 ; i < N; i++){
      for(int j =i+1 ; j < N; j++){
        colour_buffer.push_back(problem.cnfvar(EC(E(i,j),c)));
      }
    }
    // create and append unitary_variables corresponding to the counts of thoes colours
    unitary_variables.push_back(add_cardinality_constraint(colour_buffer));
  }
  // setup constraints of one colour dominant
  for (int i=0; i<unitary_variables.size()-1; i++) {
    for (int j=0; j<unitary_variables[i].size(); j++) {
      problem.cnf<<unitary_variables[i][j]<<" -"<<unitary_variables[i+1][j]<<" 0\n"; //number of colour i must be more than colour i+1
	  problem.num_clauses++;
    }
  }
}


void add_colour_at_least_for_each_vertex(int colours, int N, vector<vector<int>>& unitary_variables, int atleast) {
  for (int i=0; i<unitary_variables.size()-1; i++) {
    assert( (atleast >= 0) && (atleast<unitary_variables[i].size()));
    problem.cnf<<unitary_variables[i][atleast]<<" 0\n"; //number of colour i must be more than colour i+1
	problem.num_clauses++;
  }
}

void add_colour_at_most_for_each_vertex(int colours, int N, vector<vector<int>>& unitary_variables, int atmost) {
  for (int i=0; i<unitary_variables.size()-1; i++) {
    assert( (atmost >= 0) && (atmost<unitary_variables[i].size()));
    problem.cnf<<"-"<<unitary_variables[i][atmost]<<" 0\n"; //number of colour i must be more than colour i+1
	problem.num_clauses++;
  }
}


// Added by Mark Burgess 23rd April 2021
// simplified partial symmetry breaking by assuming the first edge is the first colour, the second edge is one of the first two colours, the third edge is one of three colours, etc.
// NOTE: potentially incompatable with post__symmetry_breaking_vertex_enumeration();
void post__simplified__symmetry_breaking_on_colours(int colours, int N){
  int colour_upto = 1;
  for(int i =0 ; i < N; i++){
    for(int j =i+1 ; j < N; j++){
      for(int c = colour_upto; c<colours; c++) {
        int var = problem.cnfvar(EC(E(i,j),c));
	    problem.cnf<<"-"<<var<<" 0\n";
	    problem.num_clauses++;
      }
      colour_upto++;
    }
  }
}



void populate_less_than_greater_than_aux_vars(int N) { // checks that the less_than_greater_than_aux_vars are initialised to appropriate sizing
  // initialise array if not yet done.
  if (problem.less_than_greater_than_aux_vars.size() != N) {
	problem.less_than_greater_than_aux_vars.resize(N);
    for (int i=0; i<N; i++) {
      problem.less_than_greater_than_aux_vars[i].resize(N);
      for (int j=0; j<N; j++) {
        problem.less_than_greater_than_aux_vars[i][j].resize(N);
        for (int k=0; k<N; k++) {
          problem.less_than_greater_than_aux_vars[i][j][k].resize(N);
        }
      }
    }
  }
}

tuple<int,int> get_edge_comparrison(int v1, int v2, int v3, int v4, int colours, int N) {
  populate_less_than_greater_than_aux_vars(N);
  if (v2<v1) FLIP(v1,v2) // vertex ordering in an edge is irrelevent, only consider one way.
  if (v4<v3) FLIP(v3,v4)
  if (problem.less_than_greater_than_aux_vars[v1][v2][v3][v4].size()!=0) // return if already calcualted before
    return problem.less_than_greater_than_aux_vars[v1][v2][v3][v4][0];

  int less_than = problem.cnfauxvar_lex();
  int greater_than = problem.cnfauxvar_lex();
  cerr<<"MAPPING: "<<less_than<<" and "<<greater_than<<" -> is less_than and greater_than between edges: "<<v1<<"-"<<v2<<" and "<<v3<<"-"<<v4<<endl;
  for (int c1=0; c1<colours; c1++) {
    for (int c2=0; c2<colours; c2++) {
      int var1 = problem.cnfvar(EC(E(v1,v2),c1));
      int var2 = problem.cnfvar(EC(E(v3,v4),c2));
      if (c1<c2) {
        problem.cnf<<" -"<<var1<<" -"<<var2<<" "<<less_than<<" 0\n"; // for any two colours if one is less than the other then less_than aux_var is true
        problem.num_clauses++;
      } else if (c1>c2) {
        problem.cnf<<" -"<<var1<<" -"<<var2<<" "<<greater_than<<" 0\n"; // " " " " greater than the other, then greater_than aux_var is true
        problem.num_clauses++;
      } else { // if neither is true then tacitly they are equal, and less_than, and greater_than are both false
        problem.cnf<<" -"<<var1<<" -"<<var2<<" -"<<less_than<<" 0\n"; // " " " " greater than the other, then greater_than aux_var is true
        problem.num_clauses++;
        problem.cnf<<" -"<<var1<<" -"<<var2<<" -"<<greater_than<<" 0\n"; // " " " " greater than the other, then greater_than aux_var is true
        problem.num_clauses++;
      }
    }
  }
  problem.cnf<<" -"<<less_than<<" -"<<greater_than<<" 0\n"; // if less than is true then greater than is not (and vice versa)
  problem.num_clauses++;

  auto tup1 = make_tuple(less_than, greater_than); // push forward and reverse ordering of less_than,greater_than vars to the arrays
  auto tup2 = make_tuple(greater_than, less_than);
  problem.less_than_greater_than_aux_vars[v1][v2][v3][v4].push_back(tup1); 
  problem.less_than_greater_than_aux_vars[v3][v4][v1][v2].push_back(tup2);
  return tup1;
}

void equality_to_some_depth(vector<tuple<int,int>>& series1, vector<tuple<int,int>>& series2, int colours, int N, int depth) {
  for (int j=0; j<depth; j++) {
    auto [v1_c,v2_c] = series1[j];
    auto [v3_c,v4_c] = series2[j];
    auto [less_than_comp2, greater_than_comp2] = get_edge_comparrison(v1_c, v2_c, v3_c, v4_c, colours, N);
    problem.cnf<<" "<< less_than_comp2 <<" "<<greater_than_comp2; // prepend equalities
  }
}

tuple<int,int> less_than_greater_than_edge_series(vector<tuple<int,int>>& series1, vector<tuple<int,int>>& series2, int colours, int N) {
// returs an auxiliary variable less_than,greater_than pair for a series of edges less than another in lex order 
  if (series1.size()!=series2.size()) {
    printf("ERROR: comparrison between series unequal\n");
    exit(1);
  }
  int size = series1.size();
  int less_than = problem.cnfauxvar_lex();
  int greater_than = problem.cnfauxvar_lex();
  cerr<<"MAPPING: "<<less_than<<" and "<<greater_than<<" -> is less_than and greater_than between series: ";
  for (int i=0; i<series1.size(); i++)
    cerr<<get<0>(series1[i])<<"-"<<get<1>(series1[i])<<",";
  cerr<<" and ";
  for (int i=0; i<series1.size(); i++)
    cerr<<get<0>(series2[i])<<"-"<<get<1>(series2[i])<<",";
  cerr<<endl;
  for (int depth=0; depth<size; depth++) {
    auto [v1,v2] = series1[depth];
    auto [v3,v4] = series2[depth];
    auto [less_than_comp, greater_than_comp] = get_edge_comparrison(v1, v2, v3, v4, colours, N);
    // for any two colours if one is less than the other then less_than aux_var is true
    problem.cnf<<less_than<<" -"<< less_than_comp;
    equality_to_some_depth(series1, series2, colours, N, depth);
    problem.cnf<<" 0\n";
    problem.num_clauses++;
    // for any two colours if one is greater than the other then greater_than aux_var is true
    problem.cnf<<greater_than<<" -"<< greater_than_comp;
    equality_to_some_depth(series1, series2, colours, N, depth);
    problem.cnf<<" 0\n";
    problem.num_clauses++;
  }
  // if less than is true then greater than is not (and vice versa)
  problem.cnf<<" -"<<less_than<<" -"<<greater_than<<" 0\n";
  problem.num_clauses++;

  // if all equal then not greater than
  problem.cnf<<"-"<<greater_than;
  equality_to_some_depth(series1, series2, colours, N, size);
  problem.cnf<<" 0\n";
  problem.num_clauses++;

  // if all equal then not less than
  problem.cnf<<"-"<<less_than;
  equality_to_some_depth(series1, series2, colours, N, size);
  problem.cnf<<" 0\n";
  problem.num_clauses++;
  
  return make_tuple(less_than, greater_than);
}



// post symmetry breaking constraints on vertex enumeration
// see "Breaking Symmetries in Graph Representation" by Michael Codish, Alice Miller, Patrick Prosser, Peter J. Stuckey
//
void post__symmetry_breaking_vertex_enumeration(int colours, int N) {
  for (int vertex1=0; vertex1<N-1; vertex1++) { // comparing pairs of vertices vertex2>vertex1
    for (int vertex2=vertex1+1; vertex2<N; vertex2++) {
      vector<tuple<int,int>> series1;
      vector<tuple<int,int>> series2;
      series1.clear();
      series2.clear();
      for (int connecting_vertex=N-1; connecting_vertex>=0; connecting_vertex--) // reverse order
        if ((connecting_vertex!=vertex1) && (connecting_vertex!=vertex2)) { // there are N-2 other vertices other than vertex1 and vertex2
          series1.push_back(make_tuple(vertex1,connecting_vertex));
          series2.push_back(make_tuple(vertex2,connecting_vertex));
        }
      auto new_less_than_greater_than = less_than_greater_than_edge_series(series1, series2, colours, N);
      // if all of the ks are true then new_less_than is also true
      problem.cnf<<"-"<<get<1>(new_less_than_greater_than)<<" ";
      problem.cnf<<"0\n";
      problem.num_clauses++;
    }
  }
}


// constrains there to be atleast one colour connected to the last vertex, thoes colours will be in order and also
// constrains there to be more of (or equal to) a greater colour on the last vertex than a lesser colour
// note: compatable with post__symmetry_breaking_vertex_enumeration
void post__symmetry_breaking_on_colours_last_vertex(int colours, int N) {
  //problem.levNb_begin = problem.num_clauses;
  for (int i=0; i<colours; i++) { // there must exist atleast one of each colour on final vertex
    for (int vertex1=0; vertex1<N-1; vertex1++)
      problem.cnf<<problem.cnfvar(EC(E(vertex1,N-1),i))<<" ";
    problem.cnf<<"0\n";
    problem.num_clauses++;
  }
  for (int vertex1=0; vertex1<N-2; vertex1++) { // colours must be sequential on final vertex
    for (int c=0; c<colours-1; c++) {
      problem.cnf<<"-"<<problem.cnfvar(EC(E(vertex1,N-1),c))<<" "<<problem.cnfvar(EC(E(vertex1+1,N-1),c))<<" "<<problem.cnfvar(EC(E(vertex1+1,N-1),c+1))<<" 0\n";
      problem.num_clauses++;
    }
    problem.cnf<<"-"<<problem.cnfvar(EC(E(vertex1,N-1),colours-1))<<" "<<problem.cnfvar(EC(E(vertex1+1,N-1),colours-1))<<" 0\n";
    problem.num_clauses++;
  }
  // there must be more of (or equal to) the greater colours than any lesser colour
  // ___W___XZ___Y___  if W and X are the same colour c1, and if Z is c1+1, then Y is c1+1
  // if Y is outside of feasible, then false
  for (int c1=0; c1<colours-1; c1++) {
    int c2 = c1+1;
    for (int v_sub_critical=0; v_sub_critical<N-2; v_sub_critical++) {
      int v_critical=v_sub_critical+1;
      for (int v_sub_critical_check = 0; v_sub_critical_check < v_sub_critical; v_sub_critical_check++) {
        int v_over_critical = v_sub_critical - v_sub_critical_check + v_critical; 
        if (v_over_critical < N-1) {
          problem.cnf<<"-"<<problem.cnfvar(EC(E(v_sub_critical_check,N-1),c1))<<" -"<<problem.cnfvar(EC(E(v_sub_critical,N-1),c1));
          problem.cnf<<" -"<<problem.cnfvar(EC(E(v_critical,N-1),c2))<<" "<<problem.cnfvar(EC(E(v_over_critical,N-1),c2))<<" 0\n";
          problem.num_clauses++;
        } else { // Y is outside of feasible
          problem.cnf<<"-"<<problem.cnfvar(EC(E(v_sub_critical_check,N-1),c1))<<" -"<<problem.cnfvar(EC(E(v_sub_critical,N-1),c1));
          problem.cnf<<" -"<<problem.cnfvar(EC(E(v_critical,N-1),c2))<<" 0\n";
          problem.num_clauses++;
        }
      }
    }
  }
  //problem.levNb_end = problem.num_clauses;
}



void post__triad_at_vertex_implies_corresponding_triangle_at_vertex(int N, const Triangles& triangles){
  for(int v = 0; v < N; v++){ // clique vertices
    for(auto triangle : triangles ){ // all canonical triangles that can be made with M colours
      auto tid = triangle.get__id();
      auto triad_at_v = problem.cnfvar(tid,v);
      vector<int> perm_at_v_props;
      assert(triangle.get__permutations().size());

      // A satisfied triad implies a witnessed triangle
      for(auto perm : triangle.get__permutations()){
	auto perm_at_v = problem.cnfvar(perm,v);
	problem.cnf<<"-"<<perm_at_v<<" "<<triad_at_v<<" 0\n";
	problem.num_clauses++;
	perm_at_v_props.push_back(perm_at_v);
      }

      // a witnessed triangle implies one or more satisfied triads
      problem.cnf<<"-"<<triad_at_v<<" ";
      for(auto perm_at_v : perm_at_v_props){
	problem.cnf<<perm_at_v<<" ";
      }
      problem.cnf<<" 0\n";
      problem.num_clauses++;
    }
  }
}

void post__coloured_3_tour_from_vertex_implies_corresponding_triad(int i, int j, int k, const Triangle& triangle){

  // Undirected graph
  assert ( i < j );
  assert ( j < k );
  
  for (auto permutation : triangle.get__permutations() ){ // For every unique permutation of triangle's edge colours
    assert( 3 == permutation.size() ); // Assert we are treating sequence of 3 colours
    
    int count = 0;
    int prop__condition0 = 0;
    int prop__condition1 = 0;
    int prop__condition2 = 0;
    for ( auto colour : permutation ){ // For every colour in the permutation
      if ( 0 == count ){
	EC condition0 = EC(pair<int,int>(i,j), colour);
	prop__condition0 = problem.cnfvar(condition0);
      } else if ( 1 == count ) {
	EC condition1 = EC(pair<int,int>(j,k), colour);
	prop__condition1 = problem.cnfvar(condition1);
      } else if ( 2 == count ) {
	EC condition2 = EC(pair<int,int>(k,i), colour);
	prop__condition2 = problem.cnfvar(condition2);
      } else {
	assert(0);
      }
      count++;
    }
    assert( 0 < prop__condition0 );
    assert( 0 < prop__condition1 );
    assert( 0 < prop__condition2 );
    
    vector<int> permutation0 = permutation;
    vector<int> permutation1 = permutation;
    vector<int> permutation2 = permutation;
	    
    permutation1[0] = permutation[1];
    permutation1[1] = permutation[2];
    permutation1[2] = permutation[0];
	    
    permutation2[0] = permutation[2];
    permutation2[1] = permutation[0];
    permutation2[2] = permutation[1];
    
    auto prop__triad0 = problem.cnfvar(permutation0,i); // permutation0 is at vertex i
    auto prop__triad1 = problem.cnfvar(permutation1,j); // permutation1 is at vertex j
    auto prop__triad2 = problem.cnfvar(permutation2,k); // permutation2 is at vertex k
    
    auto witness = problem.cnfauxvar();
    
    problem.cnf<<"-"<<prop__condition0<<" "
	       <<"-"<<prop__condition1<<" "
	       <<"-"<<prop__condition2<<" "
	       <<""<<witness<<" 0\n";
    problem.num_clauses++;
    
    problem.cnf<<""<<prop__condition0<<" "
	       <<"-"<<witness<<" 0\n";
    problem.num_clauses++;
    
    problem.cnf<<""<<prop__condition1<<" "
	       <<"-"<<witness<<" 0\n";
    problem.num_clauses++;
    
    problem.cnf<<""<<prop__condition2<<" "
	       <<"-"<<witness<<" 0\n";
    problem.num_clauses++;

    
    problem.cnf<<"-"<<witness<<" "
	       <<prop__triad0<<" 0\n";
    problem.num_clauses++;
    
    problem.cnf<<"-"<<witness<<" "
	       <<prop__triad1<<" 0\n";
    problem.num_clauses++;
    
    problem.cnf<<"-"<<witness<<" "
	       <<prop__triad2<<" 0\n";
    problem.num_clauses++;
    
    problem.report__witnesses(prop__triad0, witness);
    problem.report__witnesses(prop__triad1, witness);
    problem.report__witnesses(prop__triad2, witness);
  }
}

void post__coloured_3_tour_from_vertex_implies_corresponding_triad(int vertex, int N, const Triangles& triangles){
  for ( int j  = vertex+1 ; j < N ; j++ ){ // Second vertex visited on tour
    for ( int k  = j+1 ; k < N ; k++ ){    // Third vertex visited on tour
      for ( auto triangle : triangles ){
	post__coloured_3_tour_from_vertex_implies_corresponding_triad(vertex, j, k, triangle);
      }
    } 
  }
}

void post__coloured_3_tour_from_vertex_implies_corresponding_triad(const vector<int>& vertices, const Triangles& triangles){
  for(auto v : vertices){ // Vertex where the tour starts/ends
    post__coloured_3_tour_from_vertex_implies_corresponding_triad(v, vertices.size(), triangles);
  }

  // Triad implies a witness
  // -- If a triad is satisfied, then it must be witnessed by a compatible tour
  // -- a witness, here, is an auxiliary variable indicating tours of coloured edges that are compatible with the triad
  for ( auto it = problem.triad__to__witnesses.begin()
	  ; it != problem.triad__to__witnesses.end()
	  ; it++ ) {
    assert(it->second.size()); // you cannot have a triad that cannot be witnessed
    auto triad = it->first;
    problem.cnf<<"-"<<triad<<" ";
    for ( auto witness : it->second ) {
      problem.cnf<<""<<witness<<" ";
    }
    problem.cnf<<" 0\n";
    problem.num_clauses++;
  }
}

void post__there_are_no_monochromatic_triangles(int N, const Triangles& monochromatic_triangles){
  for(int v = 0; v < N; v++){
    for(auto triangle : monochromatic_triangles ){
      auto tid = triangle.get__id();
      auto triad_at_v = problem.cnfvar(tid,v);
      problem.cnf<<"-"<<triad_at_v<<" 0\n";
      problem.num_clauses++;
    }
  }
}

void post__every_nonmonochromatic_triangle_appears_everywhere_it_can(int N, const Triangles& nonmonochromatic_triangles)
{
  for(int v = 0; v < N; v++){
    for(auto triangle : nonmonochromatic_triangles ){
      auto tid = triangle.get__id();
      auto triad_at_v = problem.cnfvar(tid,v);
      problem.cnf<<triad_at_v<<" 0\n";
      problem.num_clauses++;
    }
  }
}

