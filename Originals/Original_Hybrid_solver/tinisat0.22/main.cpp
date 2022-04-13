/*************************
Copyright 2007 Jinbo Huang

This file is part of Tinisat.

Tinisat is free software; you can redistribute it 
and/or modify it under the terms of the GNU General 
Public License as published by the Free Software 
Foundation; either version 2 of the License, or
(at your option) any later version.

Tinisat is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR 
A PARTICULAR PURPOSE. See the GNU General Public 
License for more details.

You should have received a copy of the GNU General
Public License along with Tinisat; if not, write to
the Free Software Foundation, Inc., 51 Franklin St, 
Fifth Floor, Boston, MA  02110-1301  USA
*************************/

#define VERSION "0.23"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
#include <ctime>
double _get_cpu_time(){ 
  return (double) clock() / CLOCKS_PER_SEC;
}
#else
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
double _get_cpu_time(){ 
	struct rusage usage;
  	getrusage(RUSAGE_SELF, &usage);
  	return (usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) * 
	(1e-6) + (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec); 
}
#endif
#include "SatSolver.h"
#include <mpi.h>

void runTinisat(int argc, char** argv) {
	double _start_time = _get_cpu_time();

	static char usage[] = "usage: %s -f cnf_file -d decision_interval (e.g. 30) -s suggestion_size (e.g. 3) -r heuristic_rotation (slsfirst or cdclfirst)  \n";
	
	// getopt related, string associated with the argument we just passed.
	extern char *optarg;

	// getopt related, the number of arguments we received.
	extern int optind;
	
	if ((optind+1) > argc) { // here, +N is the number of required arguments
	  printf("optind = %d, argc=%d\n", optind, argc);
	  fprintf(stderr, usage, argv[0]);
	  exit(1);
	} 

	// Logging.
	FILE* f = fopen("output", "a");
	
	int decision_interval = 0, suggestion_size = 0;
	char c;
	char* CNF_file = NULL;
	char* heuristic_rotation_scheme = NULL;
	if(argc == 2) CNF_file = argv[1]; // This is a default behaviour of SAT solvers, and
	                                  // therefore we leave this in case we would like to
	                                  // participate in a competition.
	
	while((c = getopt(argc, argv, "d:s:f:r:a:")) != -1){
	  switch(c){
	  case 'd':
	    decision_interval = atoi(optarg);
	    break;
	  case 's':
	    suggestion_size = atoi(optarg);
	    break;
	  case 'f':
	    CNF_file = optarg;
	    break;
	  case 'r':
	    heuristic_rotation_scheme = optarg;
	    break;
	  case 'a':
	    break;
	  case '?':
	    break;
	  }
	}// Parsing arguments	

	if(CNF_file == NULL){
	       	CNF_file = argv[argc-1];
	}
	
	heuristic_rotation = slsfirst;
	if(0 == strcmp("slsfirst", heuristic_rotation_scheme)){
	  heuristic_rotation = slsfirst;
	} else if (0 == strcmp("cdclfirst", heuristic_rotation_scheme)) {
	  heuristic_rotation = cdclfirst;
	}
	
	Cnf *cnf = new Cnf(CNF_file);
	printf("Tinisat -- c solving %s\n", argv[1]);
	printf("Tinisat -- c %d variables, %d clauses\n", cnf->vc, cnf->cc);
	fflush(stdout);
	
	
	SatSolver solver(*cnf, _start_time, decision_interval, suggestion_size); 
	delete cnf;

	bool result = solver.run();

	int numProcesses;
	MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

	/* this code can be used for interfacing with SateELite
	if(argc > 2){
		if(result){ 
			FILE *ofp;
			if ((ofp = fopen(argv[2], "w")) != NULL){ 
				fprintf(ofp, "SAT\n");
				solver.printSolution(ofp);
				fclose(ofp);
			}
		}
	}else{
	*/
		if(result){
			if(solver.complete == numProcesses){
				// Tinisat found the solution
				printf("Tinisat -- s SATISFIABLE\n");
		//		printf("s SATISFIABLE\nv ");
				//solver.printSolution(stdout);
			}	
		}else printf("Tinisat --s UNSATISFIABLE\n");
	//}
	double timeTaken = _get_cpu_time() - _start_time;
	
	solver.printStats();
	printf("Tinisat -- c solved in %.2f seconds\n", timeTaken);
	
	fprintf(f, "%.2f\n", timeTaken);
	fclose(f);
}

int main(int argc, char **argv){
	MPI_Init(NULL,NULL);
	printf("c Tinisat %s\n", VERSION);
	if(argc < 2) MPI_Abort(MPI_COMM_WORLD, -1);

	runTinisat(argc, argv);

	MPI_Finalize();
	return 0;
}
