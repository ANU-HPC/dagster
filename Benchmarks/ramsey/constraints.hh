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

#ifndef CONSTRAINTS_HH
#define CONSTRAINTS_HH

#include "global.hh"
#include "problem.hh"



#define FLIP(A,B) { \
  auto temp = B; \
  B = A; \
  A = temp; \
} \


// CONSTRAINT -- [TREE ENCODING] At most one of the propositions in $vars$ can be true.
// In theory, can get stronger nogoods.
// In practice, can be more compact.
//
// return: index of aux variable that can be true iff exactly one prop. in $vars$ is true
//         -1 if no aux vars are required to code the constraint
//
int print__tree_at_most_one(const set<int>& vars);

// CONSTRAINT -- [TREE ENCODING] Exactly one of the propositions in $vars$ can be true.
//
// (uses \function{print__tree_at_most_one})
void print__tree_exactly_one(const set<int>& vars);


// CONSTRAINT -- [FLAT ENCODING] At most one of the propositions in $vars$ can be true. 
int print__at_most_one(const set<int>& vars);

// CONSTRAINT -- [FLAT ENCODING] Exactly one of the propositions in $vars$ can be true. 
void print__exactly_one(const set<int>& vars);

// CONSTRAINT -- a clique edge must be coloured by exactly one colour
void post__clique_edges_have_one_colour_each(int M, const E& ij);
void post__clique_edges_have_one_colour_each(int M, int N);

// CONSTRAINT -- Here, a "triad" is a sequence of 3 colours linked to a clique vertex
//               if triad is satisfied, then the unique indicated triangle is witnessed at the corresponding vertex
//               if a triangle is satisfied at a clique vertex, then at least one corresponding triad is satisfied
void post__triad_at_vertex_implies_corresponding_triangle_at_vertex(int N, const Triangles& triangles);

// CONSTRAINT -- a coloured 3-tour is a path on the N-clique of length 3, departing and returning to some vertex.
//               If such a tour indicates a triad, then the triad is satisfied
//               If a triad is satisfied, then one or more tours associated with that triad occur
void post__coloured_3_tour_from_vertex_implies_corresponding_triad(int i, int j, int k, const Triangle& triangle);
void post__coloured_3_tour_from_vertex_implies_corresponding_triad(int vertex, int N, const Triangles& triangles);
void post__coloured_3_tour_from_vertex_implies_corresponding_triad(const vector<int>& vertices, const Triangles& triangles);

// CONSTRAINT -- there must be no monochromatic triangles witnessed at a clique vertex
void post__there_are_no_monochromatic_triangles(int N, const Triangles& monochromatic_triangles);

// CONSTRAINT -- all nonmonochromatic triangles must be witnessed at all clique vertices
void post__every_nonmonochromatic_triangle_appears_everywhere_it_can(int N, const Triangles& nonmonochromatic_triangles);


// Added by Mark Burgess 23rd April 2021
// simplified partial symmetry breaking by assuming the first edge is the first colour, the second edge is one of the first two colours, the third edge is one of three colours, etc.
void post__simplified__symmetry_breaking_on_colours(int colours, int N);
// a lex-leader symmetry breaking constraint on the vertices of the problem  as defined by the colours they make in connection with the other vertices
void post__symmetry_breaking_vertex_enumeration(int colours, int N);
// colour/vertex enumeration on the final vertex
void post__symmetry_breaking_on_colours_last_vertex(int colours, int N);
// add symmetry breaking on number of colours in order
void add_colour_quantifier_symmetry_breaking(int colours, int N);

#endif
