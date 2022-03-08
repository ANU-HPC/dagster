/*********************************************************************************************************/
/* Generates CNF representation of Costas array problem of dimension N.                                  */
/*********************************************************************************************************/
/* C. Gretton   - ORIGINAL: 3 Aug 22:48:45 AEST 2019                                                     */ 
/*                UPDATED:  6 Aug 11:14:05 AEST 2019  BUGFIX+TESTING                                     */ 
/*                Wed 22 Jan 11:09:13 AEDT 2020 Symmetry                                                 */
/* J. Milthorpe - UPDATED: 23 DEC 11:09:00 AEST 2019  Ouputs Problem Decomposition with Directed Acyclic */
/*                                                    Graph.                                             */
/*********************************************************************************************************/

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
#include "constraints.hh"

Problem problem;

int main(int argc, char** argv){

  static char usage[] = "usage: -N [1..9]+[0..9]* (e.g. 32) (i.e. dimension of Costas array) [-F (FLAT|TREE)] [-b] [-s]\n";	
  if ( argc < 2 ){
    cerr<<usage<<endl;
    exit(-1);
  }
  
  // getopt related, string associated with the argument we just passed.
  extern char *optarg;

  char* at_most_one_constraint_form = NULL;
  // if true, divide problem into only two nodes; else, one node per level
  bool bisect = false;
  bool super_simple = false;
  char c;
  while((c = getopt(argc, argv, "N:F:b:s")) != -1){
    switch(c){
    case 'N':
      PARSE_ARGUMENT(problem.N,"-N:dimension")
	//problem.N = atoi(optarg);
      assert( problem.N > 1 );
      break;
    case 'F':
      //PARSE_ARGUMENT(at_most_one_constraint_form,"-F:constraint-structure")
      at_most_one_constraint_form = optarg;
      break;
    case 'b':
      bisect = true;
      break;
    case 's':
      super_simple = true;
      break;
    default:
      cerr<<usage<<endl;
      exit(-1);
      break;
    }
  }// Parsing arguments
 
  ostringstream dagfile_name;
  dagfile_name << "costas_" << problem.N << ".dag"; 
  
  if ( 0 == at_most_one_constraint_form || string("FLAT") == string(at_most_one_constraint_form) ){
    print__setup_permutation();
    print__distance_constraint();

    // print__setup_row_heights();                                                                                   // Wed 22 Jan 11:09:13 
    // print__half_pi_rotated_distance_constraint();                                                                 // Wed 22 Jan 11:09:13    
    
    // print__symmetry_break__first_distance_at_highest_level_must_be_positive();                                    // Wed 22 Jan 11:09:13
    // print__symmetry_break__first_distance_at_highest_level_must_be_greater_than_last();                           // Wed 22 Jan 11:09:13
    // print__symmetry_break__first_distance_at_highest_level_must_be_greater_than_corresponding_rotated_distance(); // Wed 22 Jan 11:09:13
    
    //print__redundant_distances();
//    print__rotation_translation_constraints();
    print__dist__exists();
    //problem.print__dag_file(dagfile_name.str(), bisect);
    if (!super_simple) {
	    problem.print__dag_file_simple(dagfile_name.str());
	} else {
	    problem.print__dag_file_super_simple(dagfile_name.str());
	}
  } else if ( string("TREE") == string(at_most_one_constraint_form) ){
    print__setup_permutation(&print__tree_exactly_one);
    print__distance_constraint(&print__tree_at_most_one);
    //print__redundant_distances();
//    print__rotation_translation_constraints();
    print__dist__exists();
    //problem.print__dag_file(dagfile_name.str(), bisect);
    if (!super_simple) {
	    problem.print__dag_file_simple(dagfile_name.str());
	} else {
	    problem.print__dag_file_super_simple(dagfile_name.str());
	}
    //assert(!problem.dist__exists__cnfvar.size());
    //assert(problem.aux.size());
  } else {
    cerr<<"UNRECOVERABLE ERROR: Unknown at-most-one constraint form specified == "<<string(at_most_one_constraint_form)<<". \n";
    exit(-1);
  }
  
  auto num_vars = problem.distance__to__cnfvar.size() + problem.rowassignment__to__cnfvar.size() + problem.dist__exists__cnfvar.size() + problem.aux.size() +
    problem.horizontal_distance__to__cnfvar.size();// UPDATED Wed 22 Jan 11:09:13  
  const auto& formula = problem.cnf.str();
  cout<<"p cnf "<<num_vars<<" "<<problem.num_clauses<<endl;
  cout<<formula;
  return 0;
}
