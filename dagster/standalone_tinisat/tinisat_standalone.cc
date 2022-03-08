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

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <cerrno>
#include <time.h>

#include "../Cnf.h"
#include "../Cnf.cpp"
#include "../CnfManager.h"
#include "../CnfManager.cpp"
#include "../CnfHolder.h"
#include "../CnfHolder.cpp"
#include "../SatSolver.h"
#include "../SatSolver.cpp"
#include "../utilities.h"
#include "../utilities.cpp"
#include "../strengthener/Work.h"
#include "../strengthener/Work.cc"
#include "../strengthener/MpiBuffer.h"
#include "../strengthener/MpiBuffer.cc"
#include "../ReversableIntegerMap.h"
#include "../ReversableIntegerMap.cc"
#include "../arguments.h"
#include "../arguments.cpp"

using namespace std;

Arguments command_line_arguments; // holder for parsed command line arguments
CnfHolder* cnf_holder; // the cnf_holder object for retrieving CNF components for a dag


int main(int argc,char *argv[]) {
  if (argc < 1) {
    printf("must pass CNF file <filename> [FILE_OUT] [COUNTING]\n");
    return 1;
  }
  bool counting = false;
  if (argc>=4)
    counting = strcmp(argv[3],"true")==0;
  
  std::string str ("");
  clock_t tStart = clock();
  srandom(genRandomSeed());
  
  // load the CNF into new solver
  Cnf* cnf = new Cnf((const char*)argv[1]);
  SatSolver* solver = new SatSolver(cnf, 5, 5, NULL, NULL, true, false, str, 0);
  
  int ret_code;
  std::deque<int> conflicts;
  int solutions = 0;
  FILE *ofp;
  
  // if outputting to file, clear the file
  if(argc > 2)
    if ((ofp = fopen(argv[2], "w")) != NULL)
      fclose(ofp);
  
  // start a loop searching for all solutions (if counting)
  do {
    // run the solver while it is returning status code 2 (ie. continuing)
    ret_code = 2;
    while (ret_code==2) {ret_code=solver->run();}
    
    // if solution
    if (ret_code==1) {
      solutions++;
      // print to file or console
      if(argc > 2){ 
        if ((ofp = fopen(argv[2], "a")) != NULL){
          if (solutions==1) 
            fprintf(ofp, "SAT\n");
          solver->printSolution(ofp);
          fclose(ofp);
        }
      }else{
        if (solutions==1)
          printf("s SATISFIABLE\n");
        solver->printSolution(stdout);
      }
      
      // add the negation of the solution to the solver and refresh it
      conflicts.clear();
      solver->load_into_deque(conflicts);
      for (int i=0; i<conflicts.size(); i++)
        conflicts[i] *= -1;
      solver->solver_add_conflict_clause(conflicts);
      ret_code = solver->reset_solver();
    }
    // and repeat while it is SAT
  } while ((ret_code==1) && (counting==1));
  if ((argc<=2) && (solutions==0))
    printf("s UNSATISFIABLE\n");
  
  delete solver;
  delete cnf;
  printf("\nTime taken: %.5fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
  return 0;
}




