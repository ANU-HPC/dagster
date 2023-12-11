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

#ifndef PROBLEM_HH
#define PROBLEM_HH

#include "global.hh"

// Edge (i.e. undirected graph)
typedef pair<int, int> E;

// Edge (first)  with colour (second)
typedef pair<E, int> EC;

// A triangle is a sequence/list of 3 colours
class Triangle : public vector<int>
{
public:
  //  Printing triangles to an output stream 
  friend ostream& operator<<(ostream&, const Triangle& t);

  // Triangle with colours <a,b,c>
  // Triangle \member{id} shall be initialised to value of static \member{ID}
  // NOTE: Constructs all permutations of the triangle <a,b,c>, and stores them in \member{permutations}
  Triangle(int a, int b, int c);
  
  // All permutations of $*this$
  const set<vector<int> >& get__permutations() const;

  // (see \member{id})
  int get__id() const;
protected:
  // Intention is to have a unique id for every canonical triangle -- i.e. only one instance of \class{Triangle} for each element in \member{permutations}
  int id;

  // If I create a new instance of \class{Triangle}, what will its \member{id} be.
  static int ID;

  // Permutations of *this (see \member{Triangle()})
  set<vector<int> > permutations;
};

// friend of  \class{Triangle}
ostream& operator<<(ostream&, const Triangle& t);

// Container of triangles
typedef set<Triangle> Triangles;

/* DIMACS CNF of the following problem occurs at \member{cnf}                                               */
/*                                                                                                          */
/*  (see \module{constraints})                                                                              */
/*                                                                                                          */
/* Given a set of M colours, find a number N such                                                           */
/* that the edges of the complete graph K_N on N vertices can be                                            */
/* M-coloured in such a way that:                                                                           */
/*                                                                                                          */
/* 1. There are no monochromatic triangles.                                                                 */
/* 2. Every non-monochromatic triangle appears everywhere it can.                                           */
/*                                                                                                          */
class Problem{
public:
  // POPULATE -- \member{nonmonochromatic_triangles}
  //          -- \member{monochromatic_triangles}
  //          -- \member{triangles}
  void build_all_triangles();

  
  // PROPOSITION -- A new auxiliary proposition
  int cnfauxvar();
  
  // PROPOSITION -- A new auxiliary proposition related to lex leader symmetry breaking
  int cnfauxvar_lex();
  
  // PROPOSITION -- triangle occurs at vertex
  int cnfvar(int triangle, int vertex);
  
  // PROPOSITION -- length-3 tour of colours $permutation$ occurs from $vertex$
  int cnfvar(vector<int> permutation, int vertex);
  
  // PROPOSITION -- edge is (i,j) s.t. i<j, and this has colour \var{ec.second}
  int cnfvar(EC ec);
  
  // Number of vertices in clique K_{N}
  int N;
  
  // Number of colours in triangles 
  int M;
  
  // Global counter, incremented each time we need a new proposition.
  int nextcnfvar = 1;

  // Global counter, incremented each time we add a clause. 
  int num_clauses = 0;

  // Triangles that need to be witnessed by every clique vertex
  Triangles nonmonochromatic_triangles;
  
  // Triangles that must not be witnessed by every clique vertex
  Triangles monochromatic_triangles;
  
  // All possible triangles that can be crated using the number of colours \member{M}
  Triangles triangles;

  // Mappings from problem objects to propositions in \member{cnf}
  map<EC, int> edge_colour__to__cnfvar;
  map<pair<int,int>, int> triangle_at_vertex__to__cnfvar;
  map<pair<vector<int>, int>, int> triad__to__cnfvar;

  // aux (see \member{cnfauxvar()}) iff colour((i,j),A) AND colour((j,k),B) AND colour((i,k), C)
  // permutation -> aux if permutation == <A,B,C>
  void report__witnesses(int permutation, int witness);
  map<int, set<int> > triad__to__witnesses;

  // Set of auxiliary variables created by \member{cnfauxvar()}
  set<int> aux;

  // holder of auxiliary variables comparing if edge, v1-v2 is less than v3-v4. auxiliary variables are less-than, and greater than respectively
  vector<vector<vector<vector<vector<tuple<int,int>>>>>> less_than_greater_than_aux_vars;

  // output to dag file
  void print__dag_file(string filename);

  /* CNF representation of problem.*/
  ostringstream cnf;

  // integers storing clauses where specific constraints start/stop
  int levNa_begin, levNa_end, levNb_begin,levNb_end;
  int levNc_end;
  int Z = 4; // the number of edges in the second node
};

#endif
