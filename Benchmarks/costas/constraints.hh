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

#ifndef CONSTRAINTS_HH
#define CONSTRAINTS_HH



// CONSTRAINT -- [TREE ENCODING] At most one of the propositions in $vars$ can be true.
// In theory, can get stronger nogoods.
// In practice, can be more compact.
//
// return: index of aux variable that can be true iff exactly one prop. in $vars$ is true
//         -1 if no aux vars are required to code the constraint
//
// C. Gretton, suggests good approach for UNSAT proof at N=32, and has
// used before successfully in highly constrained fleet assignment
// problem. Was suggested by A. Grastien on August 5th 2019.
int print__tree_at_most_one(const set<int>& vars);



// CONSTRAINT -- array is constrained to be rotated & translated such that point in the left hand column (ie. index 0) is maximally high. 
// Mark Burgess, August 6th 2019
//             , updated August 7th 2019
void print__rotation_translation_constraints();

// CONSTRAINT -- [REDUNDANT] if dist(a,b) = x and dist(b,c) = y, then dist(a,c) = x+y
// Alban Grastien, August 6th 2019
void print__redundant_distances();
  
// CONSTRAINT -- [TREE ENCODING] Exactly one of the propositions in $vars$ can be true.
//
// (uses \function{print__tree_at_most_one})
void print__tree_exactly_one(const set<int>& vars);


// CONSTRAINT -- [FLAT ENCODING] At most one of the propositions in $vars$ can be true. 
int print__at_most_one(const set<int>& vars);

// CONSTRAINT -- [FLAT ENCODING] Exactly one of the propositions in $vars$ can be true. 
void print__exactly_one(const set<int>& vars);


// CONSTRAINT -- [FLAT ENCODING] Array is permutation of numbers 0..N-1
void print__setup_permutation(void (*_print__exactly_one)(const set<int>&) = &print__exactly_one );


// Update $all_different$ constraints map-RHS-is-CNF-propositions implied
// by vertical distance as index map-LHS. NOTE: assumption is that all
// horizontal distances associated with map-RHS propositions are
// equal.
void update(map<int/*distance*/,set<int> /*variables*/>& all_differents
	    ,int distance_assignment
	    ,int distance);

// Update alldifferent constraints map-RHS-is-CNF-propositions implied
// by pair of column indices map-LHS-pairs-of-column-indices
void update(map<tuple<int,int>/*kl*/,set<int> /*variables*/>& all_differents
	    , int distance_assignment
	    , const tuple<int, int>& kl);

  
// CONSTRAINT -- No two 2D vectors between grid tokens can be equal
void print__distance_constraint(int (*_print__at_most_one)(const set<int>&) = &print__at_most_one );

// CONSTRAINT -- the set of vertical distances between pairs of columns at hdist contains the value vdist
//               iff
//               one of the pairs of columns at hdist contains the value vdist
//
// Auxiliary constraint
void print__dist__exists();



void print__setup_row_heights();
void print__half_pi_rotated_distance_constraint(int (*_print__at_most_one)(const set<int>&) = &print__at_most_one );                                                                 
void print__symmetry_break__first_distance_at_highest_level_must_be_positive();
                            
void print__symmetry_break__first_distance_at_highest_level_must_be_greater_than_last();
void print__symmetry_break__first_distance_at_highest_level_must_be_greater_than_last(vector<int> aux, int lower_index, int upper_index, int m);                        
void print__symmetry_break__first_distance_at_highest_level_must_be_greater_than_corresponding_rotated_distance(); 


#endif
