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

#define VERSION "0.22"
#include <stdio.h>
#include <stdlib.h>
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

int main(int argc, char **argv){
	printf("c Tinisat %s\n", VERSION);
	//if(argc < 3) exit(0);
	double _start_time = _get_cpu_time();

	Cnf *cnf = new Cnf(argv[1]);
	printf("c solving %s\n", argv[1]);
	printf("c %d variables, %d clauses\n", cnf->vc, cnf->cc);
	fflush(stdout);

	SatSolver solver(*cnf, _start_time); 
	delete cnf;

	bool result = solver.run();

	// this code can be used for interfacing with SateELite
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
	
		if(result){ 
			printf("s SATISFIABLE\n");
	//		printf("s SATISFIABLE\nv ");
			solver.printSolution(stdout);
		}else printf("s UNSATISFIABLE\n");
	}
	solver.printStats();
	printf("c solved in %.2fs\n", _get_cpu_time() - _start_time);
	exit(result?10:20);
}

