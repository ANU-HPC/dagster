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
#include "constraints.hh"
#include "problem.hh"

extern Problem problem;

#define N problem.N

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

void rot_sym(int* p, int* n_path){
	for (int y = 0;y < N; y++){
		for (int x = 0; x < N; x++){
		n_path[x+N*y] = p[(x+1)*N-y-1];
		}
	}
}

void flip_sym(int* p, int* n_path){
	for (int y = 0;y < N; y++){
		for (int x = 0; x < N; x++){
			n_path[x+N*y] = p[x+N*(N-y-1)];
		}
	}
}

// void print__tree_exactly_one(const set<int>& vars){
//   if ( vars.size() < 4 ) {
//     print__exactly_one(vars);
//     return;
//   }
  
//   print__tree_at_most_one(vars);
// }

// void print__tree_at_most_one(const set<int>& vars){
//   if ( vars.size() < 4 ) {
//     print__at_most_one(vars);
//     return;
//   }

//   // I find that trees are best made, and debugged, with triangles -- lhs, rhs, top --constraint-is-->> lhs -> top; rhs -> top; -lhs | -rhs
//   typedef tuple<int,int,int> Item;
//   set<Item> items;
//   for(auto _v =vars.begin(); _v != vars.end(); _v++){
//     Item item = make_tuple(*_v /*lhs*/, *_v /*rhs*/, *_v /*top*/);
//     items.insert(item);
//   }
//   assert( 1 < items.size() );

//   // NOTE: C. Gretton -- Redundancy below in case I make a mistake and need to debug.
//   while( 2 < items.size() ){
//     set<Item> new_items; // i.e. shall be $items$ next iterations
//     for(auto _v = items.begin()
// 	  ;  _v != items.end()
// 	  ;  _v++){
//       auto _v_= _v;
//       _v_++;
//       Item item;
//       if(_v_ != items.end()){
//         auto aux = problem.cnfauxvar();
//         problem.cnf<<"-"<<get<2>(*_v)<<" -"<<get<2>(*_v_)<<" 0\n";
// 	problem.num_clauses++;
//         problem.cnf<<"-"<<get<2>(*_v)<<" "<<aux<<" 0\n";
// 	problem.num_clauses++;
//         problem.cnf<<"-"<<get<2>(*_v_)<<" "<<aux<<" 0\n";
// 	problem.num_clauses++;
//         problem.cnf<<get<2>(*_v)<<" "<<get<2>(*_v_)<<" -"<<aux<<" 0\n";
// 	problem.num_clauses++;
//         item = Item(get<2>(*_v)/*lhs*/, get<2>(*_v_)/*rhs*/, aux/*top*/);
//       } else {
//         item = make_tuple(get<2>(*_v),get<2>(*_v),get<2>(*_v)); // Odd number of $items$...
//         assert(items.end() == _v_);
//         new_items.insert(item);
//         break;
//       }
//       new_items.insert(item);
//       assert(items.end() != _v);
//       _v = _v_;
//     }
//     assert(new_items.size () < items.size() );
//     items=new_items;
//   }
  
//   // at the very top of the pyramid we do not need to introduce
//   // another triangle with auxiliary variables, just finish with
//   // statements of the two top existing auxiliary variables
//   assert(items.size()==2);
//   problem.cnf<<"-"<<get<2>(*(items.begin()))<<" -"<<get<2>(*(--items.end()))<<" 0\n";
//   problem.num_clauses++;
//   problem.cnf<<get<2>(*(items.begin()))<<" "<<get<2>(*(--items.end()))<<" 0\n";
//   problem.num_clauses++;
// }

/* 
 * All the paths currently go to maximum length to check ties.
 * Most? All? paths can be made much shorter by considering the additional constraints on
 * the problem.
 * TODO - Figure out and then add in the shorter paths
 */
void print__rotation_translation_constraints() {
	int aux,n_aux;
	int path[8][N*N];
	for (int y = 0; y < N; y++){
		for (int x = 0; x < N; x++){
			path[0][x+N*y] = problem.cnfvar(x,y);
		}
	}
	for (int i = 0; i < 3; i++){
		rot_sym(path[i],path[i+1]);
	}
	flip_sym(path[0],path[4]);
	for (int i = 4; i < 7; i++){
		rot_sym(path[i],path[i+1]);
	}	
	
	int* A = path[0];
	for (int p = 1; p < 8; p++){
		int *B = path[p];
		aux = problem.cnfauxvar();
		if (A[0] != B[0]){
			problem.cnf<<A[0]<<" "<<-B[0]<<" 0\n";
			problem.cnf<<A[0]<<" "<<aux<<" 0\n";
			problem.cnf<<-B[0]<<" "<<aux<<" 0\n";
			problem.num_clauses += 3;
		}
		else{
			problem.cnf<<aux<<" 0\n";
			problem.num_clauses++;
		}
		for (int i = 1; i<N*N-1; i++){
			if (A[i] != B[i]){
				n_aux = problem.cnfauxvar();
				problem.cnf<<B[i]<<" "<<-A[i]<<" "<<aux<<" 0\n";
				problem.cnf<<aux<<" "<<-A[i]<<" "<<-n_aux<<" 0\n";
				problem.cnf<<B[i]<<" "<<-n_aux<<" "<<aux<<" 0\n";
				problem.cnf<<-aux<<" "<<A[i]<<" "<<-B[i]<<" 0\n";
				problem.cnf<<-aux<<" "<<A[i]<<" "<<n_aux<<" 0\n";
				problem.cnf<<n_aux<<" "<<-B[i]<<" "<<-aux<<" 0\n";
				problem.num_clauses += 6;
				aux = n_aux;
			}
		}
		if (A[N*N-1] != B[N*N-1]){
			problem.cnf<<B[N*N-1]<<" "<<-A[N*N-1]<<" "<<aux<<" 0\n";
			problem.cnf<<aux<<" "<<-A[N*N-1]<<" "<<" 0\n";
			problem.cnf<<B[N*N-1]<<" "<<aux<<" 0\n";
			problem.cnf<<-aux<<" "<<A[N*N-1]<<" "<<-B[N*N-1]<<" 0\n";
			problem.num_clauses += 4;
		}
		else{
			problem.cnf<<aux<<" 0\n";
			problem.num_clauses += 1;
		}
	}
}


// Alban Grastien, August 6th 2019
void print__redundant_distances()
{
  int mid = floor( ( N - 1.0) / 2.0 );
  for (int i1 = 0 ; i1 < N; i1++){
    for (int i2 = i1+1; i2 < N ; i2++) {
      if (i2 - i1 > mid) continue;
      for (int i3 = i2+1; i3 < N ; i3++) {
	if (i3 - i1 > mid) continue;
	for (int dist1 = 1-N ; dist1 < N ; dist1++) {
	  if (dist1 == 0) continue;
	  for (int dist2 = 1-N ; dist2 < N ; dist2++) {
	    if (dist2 == 0) continue;
	    int sum = dist1 + dist2;
	    if (sum <= -N || sum == 0 || sum >= N) {
	      int x = problem.cnfvar(i1, i2, dist1);
	      int y = problem.cnfvar(i2, i3, dist2);
	      problem.cnf << -x << " " << -y << " 0\n";
	      problem.num_clauses++;
	      //	    } else {
	      //	      int x = cnfvar(i1, i2, dist1);
	      //	      int y = cnfvar(i2, i3, dist2);
	      //	      int z = cnfvar(i1, i3, sum);
	      //	      cnf << -x << " " << -y << " " << z << " 0\n";
	    }
	  }
	}
      }
    }
  }
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

void print__setup_permutation( void (*_print__exactly_one)(const set<int>&) ){
  // Exactly one token per row
  for (int i = 0 ; i < N; i++){
    set<int> exactly_one;
    for (int j = 0 ; j < N; j++){
      exactly_one.insert(problem.cnfvar(i,j));
    }
    if (i==0)
      problem.clause_first_row_begin = problem.num_clauses;
    _print__exactly_one(exactly_one);
    if (i==2)
      problem.cluase_first_row_end = problem.num_clauses;
  }

  // Exactly one token per column
  for (int i = 0 ; i < N; i++){
    set<int> exactly_one;
    for (int j = 0 ; j <  N; j++){
      exactly_one.insert(problem.cnfvar(j,i));
    }
    _print__exactly_one(exactly_one);
  }
}

void update(map<int/*distance*/,set<int> /*variables*/>& all_differents
	    ,int distance_assignment
	    ,int distance){
  auto _p = all_differents.find(distance);
  if (all_differents.end() == _p){
    set<int> newset;
    newset.insert(distance_assignment);
    all_differents[distance] = newset;
  } else {
    _p->second.insert(distance_assignment);
  }
}

void update(map<tuple<int,int>/*kl*/,set<int> /*variables*/>& all_differents
	    , int distance_assignment
	    , const tuple<int, int>& kl){  
  auto _p = all_differents.find(kl);
  if (all_differents.end() == _p){
    set<int> newset;
    newset.insert(distance_assignment);
    all_differents[kl] = newset;
  } else {
    _p->second.insert(distance_assignment);
  }
}

void print__distance_constraint( int (*_print__at_most_one)(const set<int>&) ){
  int max_dist = (N - 1) / 2;
  int dag_level = max_dist - 1;
  for (int m = 1; m <= max_dist; m++) { // Horizontal distance between tokens is m
    int level_start_clause = problem.num_clauses;
    map<int/*distance*/,set<int> /*variables*/> all_differents__dist;
    map<tuple<int,int>/*kl*/,set<int> /*variables*/> all_differents__kl;
    for(int k = 0 ; k < N; k++){ // column idx
      auto l = k + m; // 'next' column idx
      if ( l >= N ) continue;
      for (int i = 0 ; i < N; i++){ // row idx
	for (int dist = 1 ; dist < N; dist++){ // row idx delta
	  if ( i - dist >= 0 ){ // delta as negative is legal
	    auto distance_assignment = problem.cnfvar(k,l,-dist); // Vertical distance between k and l is -dist
	    update(all_differents__dist,distance_assignment,-dist);
	    update(all_differents__kl,distance_assignment,tuple<int,int>(k,l));
	    auto kth_assignment = problem.cnfvar(k,i);
	    auto lth_assignment = problem.cnfvar(l,i-dist);
	    problem.cnf<<"-"<<kth_assignment<<" -"<<lth_assignment<<" "<<distance_assignment<<" 0\n";
	    problem.num_clauses++;
	  }
	    
	  if( i + dist < N) { // delta as positive is legal
	    auto distance_assignment = problem.cnfvar(k,l,dist);
	    update(all_differents__dist,distance_assignment,dist);
	    update(all_differents__kl,distance_assignment,tuple<int,int>(k,l));
	    auto kth_assignment = problem.cnfvar(k,i);
	    auto lth_assignment = problem.cnfvar(l,i+dist);
	    problem.cnf<<"-"<<kth_assignment<<" -"<<lth_assignment<<" "<<distance_assignment<<" 0\n";
	    problem.num_clauses++;
	  }
	}
      }
    }
    for ( auto p = all_differents__dist.begin() // forall unique vertical distances encountered above, where horizontal distance is m
	    ; p != all_differents__dist.end()
	    ; p++){
      _print__at_most_one(p->second); // Only one kl(m) pair can have vertical distance p->first
    }
    assert(all_differents__kl.size());
    for ( auto p = all_differents__kl.begin() // forall unique kl distances encountered above, where l=k+m
	    ; p != all_differents__kl.end()
	    ; p++){
      _print__at_most_one(p->second); // Only one l=k+m vertical distance can appear
    }

    if (m > 1) {
      // vertical distances at horizontal distance m are assigned to DAG node (m-1)
      pair<int,int> level_distance_constraints(level_start_clause, problem.num_clauses-1);
      problem.dag_node_clause_ranges[dag_level].insert(level_distance_constraints);
    }
    dag_level--;
  } // m is horizontal distance
}

void print__dist__exists() {
  int max_dist = (N - 1) / 2;
  for (int hdist = 1; hdist <= max_dist; hdist++) {                         // Horizontal distance between columns is $hdist$
    int level_start_var = problem.nextcnfvar;
    int level_start_clause = problem.num_clauses;
    for (int vdist = -(N - 1); vdist < N; vdist++) {                        // Vertical distance between columns is $vdist$
      if (vdist == 0)// It cannot be the case the two distinct tokens have 0 vertical distance
        continue;
      auto dist_exists = problem.dist_exists(hdist, vdist);                 // PROPOSITION
      set<int> col_pairs;
      for (int col = 0; col < N - hdist; col++) {
        auto distance_assignment = problem.cnfvar(col, col + hdist, vdist); // PROPOSITION
        col_pairs.insert(distance_assignment);                              // EXPAND :: SET PROPOSITIONS
        problem.cnf << "-" << distance_assignment << " " << dist_exists << " 0\n";
	problem.num_clauses++;
      }
      problem.cnf << "-" << dist_exists;
      for (auto pair_clause = col_pairs.begin();
           pair_clause != col_pairs.end();
           pair_clause++) {
        problem.cnf << " " << *pair_clause;
      }
      problem.cnf << " 0\n";
      problem.num_clauses++;
    }
    
    // If we are working on tokens with a horizontal distance greater than 1 -- i.e. distinct tokens
    if (hdist > 1) {
      int level_end_var = problem.nextcnfvar - 1;
      int dag_level = max_dist - hdist;
      pair<int, int> edge(dag_level, dag_level + 1);
      pair<int, int> variable_range(level_start_var, level_end_var);
      problem.dag_edge_variable_ranges[edge].insert(variable_range);

      pair<int,int> level_distance_set_constraints(level_start_clause, problem.num_clauses-1);
      problem.dag_node_clause_ranges[dag_level].insert(level_distance_set_constraints);
    }
  }
}



void print__setup_row_heights()
{
  for(int i = 0 ; i < N; i++){ // column idx
    for(int height = 0 ; height < N; height++){ // height
      auto col_i_token_at_height = problem.cnfvar(i, height, "VERTICAL");
      auto row_height_token_at_height_i = problem.cnfvar(height,i, "HORIZONTAL");
      problem.cnf<<"-"<<col_i_token_at_height<<" "<<row_height_token_at_height_i<<" 0\n";
      problem.num_clauses++;
      problem.cnf<<"-"<<row_height_token_at_height_i<<" "<<col_i_token_at_height<<" 0\n";
      problem.num_clauses++;
    }
  }
}

void print__half_pi_rotated_distance_constraint(int (*_print__at_most_one)(const set<int>&) )
{
  int max_dist = (N - 1) / 2;
  //int dag_level = max_dist - 1;// Row levels are not interface variables in the current dag
  for (int m = 1; m <= max_dist; m++) { // Vertical distance between tokens is m
    //int level_start_clause = problem.num_clauses;// Row levels are not interface variables in the current dag
    map<int/*distance*/,set<int> /*variables*/> all_differents__dist;
    map<tuple<int,int>/*kl*/,set<int> /*variables*/> all_differents__kl;
    for(int k = 0 ; k < N; k++){ // row idx
      auto l = k + m; // 'next' row idx
      if ( l >= N ) continue;
      for (int i = 0 ; i < N; i++){ // col idx
	for (int dist = 1 ; dist < N; dist++){ // col idx delta
	  if ( i - dist >= 0 ){ // delta as negative is legal
	    auto distance_assignment = problem.cnfvar(k,l,-dist, "HORIZONTAL"); // Horizontal distance between k and l is -dist
	    update(all_differents__dist,distance_assignment,-dist);
	    update(all_differents__kl,distance_assignment,tuple<int,int>(k,l));
	    auto kth_assignment = problem.cnfvar(k,i, "HORIZONTAL");
	    auto lth_assignment = problem.cnfvar(l,i-dist, "HORIZONTAL");
	    problem.cnf<<"-"<<kth_assignment<<" -"<<lth_assignment<<" "<<distance_assignment<<" 0\n";
	    problem.num_clauses++;
	  }
	    
	  if( i + dist < N) { // delta as positive is legal
	    auto distance_assignment = problem.cnfvar(k,l,dist, "HORIZONTAL");
	    update(all_differents__dist,distance_assignment,dist);
	    update(all_differents__kl,distance_assignment,tuple<int,int>(k,l));
	    auto kth_assignment = problem.cnfvar(k,i, "HORIZONTAL");
	    auto lth_assignment = problem.cnfvar(l,i+dist, "HORIZONTAL");
	    problem.cnf<<"-"<<kth_assignment<<" -"<<lth_assignment<<" "<<distance_assignment<<" 0\n";
	    problem.num_clauses++;
	  }
	}
      }
    }

    // All BELOW ARE IMPLIED BY OTHER CONSTRAINTS
    // for ( auto p = all_differents__dist.begin() // forall unique vertical distances encountered above, where vertical distance is m
    // 	    ; p != all_differents__dist.end()
    // 	    ; p++){
    //   _print__at_most_one(p->second); // Only one kl(m) pair can have vertical distance p->first
    // }
    assert(all_differents__kl.size());
    for ( auto p = all_differents__kl.begin() // forall unique kl distances encountered above, where l=k+m
    	    ; p != all_differents__kl.end()
    	    ; p++){
      _print__at_most_one(p->second); // Only one l=k+m vertical distance can appear
    }

    // NOT PARTICIPATING IN DAG DECOMPOSITION AT THIS TIME
    // if (m > 1) {
    //   // vertical distances at vertical distance m are assigned to DAG node (m-1)
    //   pair<int,int> level_distance_constraints(level_start_clause, problem.num_clauses-1);
    //   problem.dag_node_clause_ranges[dag_level].insert(level_distance_constraints);
    // }
    // dag_level--;
  } // m is vertical distance
}

// Prevent sign swapped distance arrays
void  print__symmetry_break__first_distance_at_highest_level_must_be_positive()
{
  int m = (N - 1) / 2;
  int k = 0;
  
  assert( k+m < N-1 );


  for (int dist = -(N-1) ; dist < 0; dist++) {
    auto distance__k_to_k_plus_m = problem.cnfvar(k,k + m, dist);
    problem.cnf<<"-"<<distance__k_to_k_plus_m<<" 0\n";
    problem.num_clauses++;
  }
}

// Prevent rotated distance arrays
void  print__symmetry_break__first_distance_at_highest_level_must_be_greater_than_last()
{
  auto aux = vector<int>();
  int m = (N - 1) / 2;
  int k = 0;
  int l = N-1;

  // Only works for not-tiny arrays.
  assert ( l - m > 0 );
  assert( k+m < N-1 );

  print__symmetry_break__first_distance_at_highest_level_must_be_greater_than_last
    (
     aux,
     k,
     l,
     m
     );
  
}

void print__symmetry_break__first_distance_at_highest_level_must_be_greater_than_last(vector<int> aux ,
										      int k,
										      int l,
										      int m)
{
  if ( k + m > l ) return;
  
  if ( k + m == l ){
    if (aux.size()){
      
      for (int dist = -(N-1) ; dist < 0; dist++) {
	// Make sure this distance is positive if aux is triggered
	for ( auto a : aux ){ // If the trigger is met then...
	  problem.cnf<<"-"<<a<<" ";
	}

	auto distance__k_to_k_plus_m = problem.cnfvar(k,k + m, dist);
	
	problem.cnf<<"-"<<distance__k_to_k_plus_m<<" 0\n";
	problem.num_clauses++;
      }
    }
    return;
  }
  
  for (int dist = -(N-1) ; dist < N; dist++) {
    if ( dist == 0 ) continue;
    
    for (int alt_dist = -(N-1) ; alt_dist < N; alt_dist++) {
      if ( alt_dist == 0 ) continue;
      if ( dist == alt_dist ) continue; // redundant constraint in this case
      
      auto distance__k_to_k_plus_m = problem.cnfvar(k,k + m, dist);
      auto altdistance__l_minus_m_to_l = problem.cnfvar(l-m,l, alt_dist);
      
      if ( dist <= alt_dist ){ // reject -- btw do not need equality
	for ( auto a : aux ){ // If the trigger is met then...
	  problem.cnf<<"-"<<a<<" ";
	}
	
	problem.cnf<<"-"<<distance__k_to_k_plus_m<<" -"<<altdistance__l_minus_m_to_l<<" 0\n";
	problem.num_clauses++;
      } else if ( dist == - alt_dist ){
	auto new_aux = problem.cnfauxvar();
	auto deeper_aux = aux;
	deeper_aux.push_back(new_aux);
	problem.cnf<<"-"<<distance__k_to_k_plus_m<<" -"<<altdistance__l_minus_m_to_l<<" "<<new_aux<<" 0\n";
	problem.num_clauses++;
	problem.cnf<<""<<distance__k_to_k_plus_m<<" -"<<new_aux<<" 0\n";
	problem.num_clauses++;
	problem.cnf<<""<<altdistance__l_minus_m_to_l<<" -"<<new_aux<<" 0\n";
	problem.num_clauses++;
	print__symmetry_break__first_distance_at_highest_level_must_be_greater_than_last(deeper_aux, k+1, l-1, m);
      }	
    }     
  }
}

// Prevent arrays rotated at 90 deg from orbit representative
void  print__symmetry_break__first_distance_at_highest_level_must_be_greater_than_corresponding_rotated_distance()
{
  int m = (N - 1) / 2;
  int k = 0;
  int l = N-1;

  // Only works for not-tiny arrays.
  assert ( l - m > 0 );
  assert( k+m < N-1 );

  
  for (int dist = -(N-1) ; dist < N; dist++) {
    if ( dist == 0 ) continue;
    
    for (int alt_dist = -(N-1) ; alt_dist < N; alt_dist++) {
      if ( alt_dist == 0 ) continue;
      if ( dist == alt_dist ) continue; // no symmetry to break in this case

      auto distance__k_to_k_plus_m = problem.cnfvar(k,k + m, dist);

      

      if ( dist <= alt_dist ){ // reject - Equality is redundant here
	auto altdistance__row_k_to_k_plus_m = problem.cnfvar(k, k+m, alt_dist, "HORIZONTAL");
	problem.cnf<<"-"<<distance__k_to_k_plus_m<<" -"<<altdistance__row_k_to_k_plus_m<<" 0\n";
	problem.num_clauses++;

	
	auto altdistance__row_l_minus_m_to_l = problem.cnfvar(l-m, l, alt_dist, "HORIZONTAL");
	problem.cnf<<"-"<<distance__k_to_k_plus_m<<" -"<<altdistance__row_l_minus_m_to_l<<" 0\n";
	problem.num_clauses++;
      } else if ( dist <= - alt_dist ){
      	auto altdistance__row_k_to_k_plus_m = problem.cnfvar(k, k+m, -alt_dist, "HORIZONTAL");
      	problem.cnf<<"-"<<distance__k_to_k_plus_m<<" -"<<altdistance__row_k_to_k_plus_m<<" 0\n";
      	problem.num_clauses++;
	
      	auto altdistance__row_l_minus_m_to_l = problem.cnfvar(l-m, l, -alt_dist, "HORIZONTAL");
      	problem.cnf<<"-"<<distance__k_to_k_plus_m<<" -"<<altdistance__row_l_minus_m_to_l<<" 0\n";
      	problem.num_clauses++;
      }
    }
  }
}












