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
#include <set>
#include <algorithm>
#include "CnfManager.h"

#ifdef UPDEBUG 
	#define DB(x) x
#else 
	#define DB(x) 
#endif

CnfManager::CnfManager(Cnf &cnf){
	vars = new Variable[(vc = cnf.vc) + 1];
	dLevel = 1;
	nDecisions = nConflicts = nRestarts = 0; 
	varOrder = (unsigned *) calloc(vc + 1, sizeof(unsigned));
	varPosition = (unsigned *) calloc(vc + 1, sizeof(unsigned));
	int *zero = stackTop = (int *) calloc(vc + 1, sizeof(int));

	// implication lists in lieu of watch lists for binary clauses
	// temporary storage
	vector<vector<int> > imp[2];	
	imp[0].resize(vc + 1);
	imp[1].resize(vc + 1);

	// mark bottom of stack with variable 0
	vars[*(stackTop++) = 0].dLevel = 0;
	vars[0].value = _FREE;

	// create litPool
	int *p = litPool = (int *) calloc(litPoolCapacity = (cnf.lc + cnf.cc) * 2, sizeof(int));
	if(litPool == NULL){
		fprintf(stderr, "Unable to allocate %lu bytes of memory\n", (long unsigned) litPoolCapacity * sizeof(int));
		printf("s UNKOWN\n");
		exit(0);
	}
	litPools.push_back(litPool);

	// populate litPool
	unsigned i, j;
	for(i = 0; i < cnf.cc; i++){
		if(cnf.clauses[i][1] == 0){		// unit clause
			int lit = cnf.clauses[i][0];
			if(FREE(lit)) setLiteral(*(stackTop++) = lit, zero);
			else if(RESOLVED(lit)){
				printf("c contradictory unit clauses %d, %d\n", lit, -lit);
				printf("s UNSATISFIABLE\n");
				exit(20);
			}
		}else if(cnf.clauses[i][2] == 0){	// binary clause
			int lit0 = cnf.clauses[i][0];
			int lit1 = cnf.clauses[i][1];
			imp[SIGN(lit0)][VAR(lit0)].push_back(lit1);
			imp[SIGN(lit1)][VAR(lit1)].push_back(lit0);
			vars[VAR(lit0)].activity[SIGN(lit0)]++;
			vars[VAR(lit1)].activity[SIGN(lit1)]++;
		}else{
			// set up watches
			WATCHLIST(cnf.clauses[i][0]).push_back(p);
			WATCHLIST(cnf.clauses[i][1]).push_back(p);

			// copy literals to litPool
			for(j = 0; (*(p++) = cnf.clauses[i][j]); j++){
				vars[VAR(*(p-1))].activity[SIGN(*(p-1))]++;
			}
		}
	}
	litPoolSize = litPoolSizeOrig = (p - litPool); 
	
	// binary clause implication lists
	for(i = 1; i <= vc; i++) for(j = 0; j <= 1; j++){
		// last element is 0
		// first three elements are ([], lit, 0)
		// serve as antecedent for all implications
		// serve as conflicting clause with [] filled
		vars[i].imp[j] = (int *) calloc(imp[j][i].size() + 4, sizeof(int));
		vars[i].imp[j][1] = i * ((j == _POSI)?1:-1);
		unsigned k = 3;
		for(vector<int>::iterator it = imp[j][i].begin(); it != imp[j][i].end(); it++)
			vars[i].imp[j][k++] = *it;
	}

	// assert unit clauses
	assertUnitClauses();
}

CnfManager::~CnfManager(){
	for(vector<int *>::iterator it = litPools.begin(); it != litPools.end(); free(*(it++)));
	while(*(--stackTop)); free(stackTop);
	for(unsigned i = 1; i <= vc; i++) for(unsigned j = 0; j <= 1; j++) free(vars[i].imp[j]);
	free(varOrder); free(varPosition); delete [] vars;
}

bool CnfManager::assertUnitClauses(){
	for(int *p = stackTop - 1; *p; p--){
		int lit = *p;
		*p = *(--stackTop);
		if(!assertLiteral(lit, litPool + litPoolSize - 1)){
		       backtrack(dLevel - 1);
	       	       return false;
		}
	}
	return true;
}

inline void CnfManager::setLiteral(int lit, int *ante){
	vars[VAR(lit)].value = SIGN(lit);
	vars[VAR(lit)].ante = ante;
	vars[VAR(lit)].dLevel = dLevel;
}

bool CnfManager::assertLiteral(int lit, int *ante){
	int *newStackTop = stackTop;
	setLiteral(*(newStackTop++) = lit, ante);
	DB(printf("%d: %d =>", dLevel, lit);)

	while(stackTop < newStackTop){
		// the literal resolved (as opposed to set)
		int lit = NEG(*(stackTop++));	

		// implications via binary clauses
		int *impList = IMPLIST(lit);
		for(int *imp = impList + 3; true; imp++){
			// implication
			if(FREE(*imp)){
				if(*imp == 0) break; 	// end of list
				DB(printf(" %d", *imp);)
				setLiteral(*(newStackTop++) = *imp, impList + 1); 
			// contradiction
			}else if(RESOLVED(*imp)){
				DB(printf(" [%d]\n", *imp);)
				nConflicts++;
				stackTop = newStackTop;
				*impList = *imp;	// make up temporary binary clause
				learnClause(impList);	// for clause learning purposes
				return false;
			}
		}

		// other implications
		vector<int *> &watchList = WATCHLIST(lit);
		for(vector<int *>::iterator it = watchList.begin(); it != watchList.end(); it++){
			// identify the two watched literals
			int *first = *it, *watch, *otherWatch;
			if(*first == lit){ watch = first; otherWatch = first + 1; }
			else{ watch = first + 1; otherWatch = first; }

			// clause satisfied, no need to check further
			if(SET(*otherWatch)) continue;

			// look for free/true literal
			int *p = first + 2;
			bool found = true;
			while(RESOLVED(*p)) p++;
			if(*p == 0) found = false;

			// free/true literal found, swap
			if(found){
				// watch p
				WATCHLIST(*p).push_back(first);

				// unwatch watch
				*(it--) = watchList.back();
				watchList.pop_back();

				// swap literals
				int x = *watch; *watch = *p; *p = x;

			// free/true variable not found, check other watch
			}else{
				// implication
				if(FREE(*otherWatch)){
					DB(printf(" %d", *otherWatch);)
					setLiteral(*(newStackTop++) = *otherWatch, first + 1); 

					// move implied literal to beginning of clause
					if(otherWatch != first){
						int x = *otherWatch; *otherWatch = *first; *first = x;
					}
				// contradiction
				}else if(RESOLVED(*otherWatch)){
					DB(printf(" [%d]\n", *otherWatch);)
					nConflicts++;
					stackTop = newStackTop;
					learnClause(first);
					return false;
				}
			}
		}
	}
	DB(printf("\n");)
	return true;
}

void CnfManager::learnClause(int *first){
	// contradiction in level 1, instance unsat
	if(dLevel == 1){ aLevel = 0; return; }

	// update var scores and positions
	updateScores(first);

	// clear temporary storage
	conflictLits.clear();
	unsigned curLevelLits = 0;

	// mark all literals in conflicting clause
	// push to tmpConflictLits those set prior to current dLevel
	for(tmpConflictLits.clear(); *first; first++){
		// drop known backbone literals
		if(vars[VAR(*first)].dLevel == 1) continue;

		if(vars[VAR(*first)].dLevel < dLevel) tmpConflictLits.push_back(*first);
		else curLevelLits++;
		vars[VAR(*first)].mark = true;
	}

	// generate 1-UIP conflict clause
	int lit; while(true){
		// pop literal from stack as in backtrack
		lit = *(--stackTop); 
		unsigned var = VAR(lit);
		vars[var].value = _FREE;
		if(!vars[var].mark){
			if(varPosition[var] < nextVar) nextVar = varPosition[var];
			continue;
		}

		// unmark
		vars[var].mark = false;
		
		// if not decision, update scores for the whole ante clause
		if(vars[var].ante) updateScores(vars[var].ante - 1);

		// update nextVar
		if(varPosition[var] < nextVar) nextVar = varPosition[var];

		// UIP reached
		if(curLevelLits-- == 1) break;

		// else, replace with antecedent (resolution)
		for(int *ante = vars[var].ante; *ante; ante++){
			if(vars[VAR(*ante)].mark || vars[VAR(*ante)].dLevel == 1) continue;
			if(vars[VAR(*ante)].dLevel < dLevel) tmpConflictLits.push_back(*ante);
			else curLevelLits++;
			vars[VAR(*ante)].mark = true;
		}
	}

	// conflict clause minimization
	// compute assertion level (aLevel) 
	// make sure front of conflictLits is a literal from assertion level
	aLevel = 1; 
	deque<int>::iterator it;
	for(it = tmpConflictLits.begin(); it != tmpConflictLits.end(); it++){
		bool redundant = true;
		int *ante = vars[VAR(*it)].ante;
		if(ante == NULL) redundant = false;
		else for(; *ante; ante++) if(!vars[VAR(*ante)].mark){ redundant = false; break; }
		if(!redundant){
			if(vars[VAR(*it)].dLevel > aLevel){
			       	aLevel = vars[VAR(*it)].dLevel;	
		       		conflictLits.push_front(*it);
			}else conflictLits.push_back(*it);
		}
	}

	// clear variable marks
	for(it = tmpConflictLits.begin(); it != tmpConflictLits.end(); it++)
		vars[VAR(*it)].mark = false;

	// unique lit from current dLevel pushed last
	conflictLits.push_back(-lit);

	// add clause to litPool and set up watches
	addClause();

	DB(	printf("   [aLevel: %d]", aLevel);
		for(deque<int>::iterator it = conflictLits.begin(); it != conflictLits.end(); it++)
			printf(" %d", *it);
		printf("\n");
	)
}

inline void CnfManager::addClause(){
	unsigned size = conflictLits.size();

	// create new litPool if necessary
	if(litPoolSize + size + 1 > litPoolCapacity){
		litPoolCapacity *= 2;
		litPool = (int *) malloc(litPoolCapacity * sizeof(int));
		while(litPool == NULL && litPoolCapacity > litPoolSizeOrig){
			litPoolCapacity /= 2;
			litPool = (int *) malloc(litPoolCapacity * sizeof(int));
		}
		if(litPool == NULL){
			printf("c unable to allocate %lu bytes of memory\n", (long unsigned) litPoolCapacity * sizeof(int));
			printf("s UNKOWN\n");
			exit(0);
		}
		litPools.push_back(litPool);
		litPoolSize = 0;
	}
	
	// clause starts here
	conflictClause = litPool + litPoolSize;

	// first literal is the unique literal from current level
	litPool[litPoolSize++] = conflictLits.back();

	if(size > 1){
		// add clause to list
		clauses.push_back(conflictClause);

		// second literal is one from assertion level
		litPool[litPoolSize++] = conflictLits.front();

		// set up 2 watches
		WATCHLIST(conflictLits.back()).push_back(conflictClause);
		WATCHLIST(conflictLits.front()).push_back(conflictClause);
		
		// copy rest of literals to litPool
		for(unsigned i = 1; i < size - 1;) litPool[litPoolSize++] = conflictLits[i++];
	}

	// end of clause
	litPool[litPoolSize++] = 0;
}

void CnfManager::updateScores(int *p){
	for(; *p; p++){
		unsigned v = VAR(*p);
		vars[v].activity[SIGN(*p)]++;
		unsigned it = varPosition[v];
		
		// variable already at beginning
		if(it == 0) continue;
		unsigned score = SCORE(v);

		// order hasn't been violated
		if(score <= SCORE(varOrder[it - 1])) continue;

		// promote var up the order, using binary search from zChaff04
		int step = 0x400, q;
		for(q = ((int) it) - step; q >= 0; q -= step) 
			if(SCORE(varOrder[q]) >= score) break;
		for(q += step, step >>= 1; step > 0; step >>= 1){
			if(q - step >= 0) if(SCORE(varOrder[q - step]) < score)
				q -= step;
		}

		// swap it and q	
		varOrder[it] = varOrder[q]; varPosition[v] = q; 
		varPosition[varOrder[q]] = it; varOrder[q] = v;	
	}
}
