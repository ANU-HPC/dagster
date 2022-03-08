/************************************************************************************************************/
/* Generates CNF representation of Ramsey colouring with no monochromatic triangles problem with M colours  */
/* on clique k_N                                                                                            */
/************************************************************************************************************/
/* C. Gretton   - ORIGINAL : Fri 14 Feb 16:00:57 AEDT 2020                                                  */
/*              - TESTED   : Tue 18 Feb 10:28:20 AEDT 2020                                                  */
/*                                                                                                          */
/*                                                                                                          */
/*     Correspondence with Tomasz Kowalski 12-Feb-2020                                                      */
/*                                                                                                          */
/* Given a set of M colours, find a number N such                                                           */
/* that the edges of the complete graph K_N on N vertices can be                                            */
/* M-coloured in such a way that:                                                                           */
/*                                                                                                          */
/* 1. There are no monochromatic triangles.                                                                 */
/* 2. Every non-monochromatic triangle appears everywhere it can.                                           */
/*                                                                                                          */
/*                                                                                                          */
/*  References for the filed construction and the present state of the                                      */
/*  problem are:                                                                                            */
/*                                                                                                          */
/* * Kowalski, Tomasz . Representability of Ramsey relation algebras.                                       */
/*  Algebra Universalis  74  (2015),  no. 3-4, 265--275.                                                    */
/*                                                                                                          */
/* * Alm, Jeremy F.  401 and beyond: improved bounds and algorithms for                                     */
/* the Ramsey algebra search.                                                                               */
/*  J. Integer Seq.  20  (2017),  no. 8, Art. 17.8.4, 10 pp.                                                */
/*                                                                                                          */
/* * Alm, Jeremy F. ;  Andrews, David A.  A reduced upper bound for an                                      */
/* edge-coloring problem from relation algebra.                                                             */
/*  Algebra Universalis  80  (2019),  no. 2, Art. 19, 11 pp.                                                */
/*                                                                                                          */
/************************************************************************************************************/

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

#include "global.hh"
#include "problem.hh"
#include "constraints.hh"

// Representation of The Problem as CNF (see \member{problem.cnf}), and related helper functions.
Problem problem;

int main(int argc, char** argv){
  static char usage[] = "usage: -N [1..9]+[0..9]* (e.g. 5) (i.e. num vertices in clique) -M [1..9]+[0..9]* (e.g. 2) (i.e. num colours) -Z [0,N-2] (i.e. the number of edges in the 2nd node)\n";	
  if ( argc < 4 ){
    cerr<<usage<<endl;
    exit(-1);
  }
  
  // getopt related, string associated with the argument we just passed.
  extern char *optarg;

  char c;
  while((c = getopt(argc, argv, "N:M:Z:")) != -1){
    switch(c){
    case 'N':
      PARSE_ARGUMENT(problem.N,"-N:dimension")
      assert( problem.N > 1 );
      break;
    case 'M':
      PARSE_ARGUMENT(problem.M,"-M:dimension")
      assert( problem.M > 1 );
      break;
    case 'Z':
      PARSE_ARGUMENT(problem.Z,"-Z:edges")
      assert( problem.Z >= 0);
      break;
    default:
      cerr<<usage<<endl;
      exit(-1);
      break;
    }
  }// Parsing arguments

  
  problem.build_all_triangles();
  
  post__clique_edges_have_one_colour_each(problem.M, problem.N); // DONE
  problem.levNc_end = problem.num_clauses;
  
  post__triad_at_vertex_implies_corresponding_triangle_at_vertex(problem.N, problem.triangles); // DONE

  // Clique vertices
  auto vertices = vector<int>(problem.N);
  iota (std::begin(vertices), std::end(vertices), 0);
  post__coloured_3_tour_from_vertex_implies_corresponding_triad(vertices, problem.triangles); // DONE
  
  
  post__every_nonmonochromatic_triangle_appears_everywhere_it_can(problem.N,problem.nonmonochromatic_triangles); // DONE
  post__there_are_no_monochromatic_triangles(problem.N,problem.monochromatic_triangles); // DONE

  ////post__simplified__symmetry_breaking_on_colours(problem.M, problem.N);
  //post__symmetry_breaking_vertex_enumeration(problem.M, problem.N);
  //post__symmetry_breaking_on_colours_last_vertex(problem.M, problem.N);
  
  auto num_vars = problem.edge_colour__to__cnfvar.size() +
    problem.triangle_at_vertex__to__cnfvar.size() +
    problem.triad__to__cnfvar.size() +
    problem.aux.size();
  
  const auto& formula = problem.cnf.str();
  cout<<"p cnf "<<num_vars<<" "<<problem.num_clauses<<endl;
  cout<<formula;

  
  ostringstream dagfile_name;
  dagfile_name << "dag_" << problem.N << "_" << problem.M << ".dag"; 
  problem.print__dag_file(dagfile_name.str());
  
  return 0;
}
