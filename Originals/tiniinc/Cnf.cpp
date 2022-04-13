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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "Cnf.h"

Cnf::Cnf(char *fname): vc(0), cc(0), clauses(NULL), lc(0){
	FILE *ifp;
	if ((ifp = fopen(fname, "r")) == NULL){ 
		fprintf(stderr, "Cannot open file: %s\n", fname);
		exit(0);
	}

	unsigned j, k, x, clause_index = 0, max_clause_len = 1024;
	int *literals = (int *) malloc(max_clause_len * sizeof(int));
	
	char line[100000];
	size_t len = 100000;
	char c; 
	
	while((c=getc(ifp)) != EOF){ 
		if (isspace(c)) continue; else ungetc(c,ifp);
		fgets(line, len, ifp);
		if (c=='p'){
			if(sscanf(line, "p cnf %d %d", &vc, &cc) == 2){
				clauses = (int **) calloc(cc, sizeof(int *));
				cl = (unsigned *) calloc(cc, sizeof(unsigned));
				break;
			}else{
				fprintf(stderr, "Invalid CNF file\n");
				exit(0);
			}
		}
	}

	while((c=getc(ifp)) != EOF && clause_index < cc){
		if (isspace(c)) continue; else ungetc(c,ifp);
		if ((c=='-') || isdigit(c)) {
			for(j=0; fscanf(ifp, "%d", &(literals[j])), literals[j]!=0;)
				if(++j == max_clause_len){
					max_clause_len *= 2;
					literals = (int *) realloc(literals, max_clause_len * sizeof(int));
				}
			clauses[clause_index] = (int *) calloc(j + 1, sizeof(int));
			bool skip_clause = false;
			for(k = 0; k <= j;){
				bool duplicate_literal = false;
				for(x = 0; x < k; x++) {
					if(literals[x] == literals[k]){ 
						duplicate_literal = true;
						break;
					}else if(literals[x] + literals[k] == 0){
						skip_clause = true;
						break;
					}
				}
				if(skip_clause) break;
				if(duplicate_literal){
					literals[k] = literals[--j]; 
					literals[j] = 0;
				}else{
					clauses[clause_index][k] = literals[k];
					k++;
				}
			}
			if(skip_clause) free(clauses[clause_index]); 
			else lc += (cl[clause_index++] = j); 
		}
		fgets(line, len, ifp);
	}

	free(literals);
	fclose(ifp);
	if(cc > clause_index) {
		cc = clause_index;
		clauses = (int **)  realloc(clauses, clause_index * sizeof(int *));
	}
}

Cnf::~Cnf(){
	if(clauses){ 
		for(unsigned i = 0; i < cc; i++) free(clauses[i]);
		free(clauses);
	}
	free(cl);
}
