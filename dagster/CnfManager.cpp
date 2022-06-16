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
Public License along with Tinisat.
If not, see <http://www.gnu.org/licenses/>.
*************************/

#include <stdio.h>
#include <stdlib.h>
#include <set>
#include <algorithm>
#include "CnfManager.h"
#include "exceptions.h"
#include "mpi_global.h"

#ifdef UPDEBUG 
	#define DB(x) x
#else 
	#define DB(x) 
#endif

CnfManager::CnfManager(Cnf* cnf){
	num_set_literal = 0;
	nextVar = 0;
	dLevel = 1;
	nConflicts = 0;
	solver_unit_contradiction = false;

	//** dLevel starts at 1, as level 0 is the singular root literal 0

	this->cnf = cnf;
	vc = cnf->vc;
	vars = new Variable[vc + 1];
	TEST_NOT_NULL(varOrder = (unsigned*) calloc(vc + 1, sizeof(unsigned)))
	TEST_NOT_NULL(varPosition = (unsigned*) calloc(vc + 1, sizeof(unsigned)))
	int *zero;
    TEST_NOT_NULL(zero = (int*) calloc(vc + 1, sizeof(int)))
	stackTop = zero;

	//** vc+1 as, root literal has a value too.

	// implication lists in lieu of watch lists for binary clauses
	// temporary storage
	vector<vector<int> > imp[2];	
	imp[0].resize(vc + 1);
	imp[1].resize(vc + 1);
	//** implication lists come in two, one for positive implications, the other for negative implications (but indexed the other way around)

	// mark bottom of stack with variable 0
	//** as FREE, and move stackTop to next position
	zero[0] = 0;
	stackTop++;
	vars[0].dLevel = 0;
	vars[0].value = _FREE;

	// create litPool
	int lc = 0;
	for (int i=0; i<cnf->cc; i++)
		//lc += cnf->cl[i];
		if (cnf->cl[i]>lc)
			lc = cnf->cl[i];
	//litPoolCapacity = (lc + cnf->cc) * 2;
	litPoolCapacity = (lc) * 2;
	int *p;
    TEST_NOT_NULL(p = (int*)calloc(litPoolCapacity, sizeof(int)))
	litPool = p;
	litPools.push_back(litPool);

	// populate litPool
	unsigned i, j;
	for(i = 0; i < cnf->cc; i++){
		if(cnf->clauses[i][0] == 0) {
			solver_unit_contradiction = true;
			break;
		}
		if(cnf->clauses[i][1] == 0){		// unit clause
			int lit = cnf->clauses[i][0];  //** if unset add to the top of the stack, with the zero literal as ancdeant
			if(FREE(lit)) setLiteral(*(stackTop++) = lit, zero);
			else if(RESOLVED(lit)){
				solver_unit_contradiction = true;  //TODO: check that this flag fails other operations, without causing memory leaks
				break;
				//printf("c contradictory unit clauses %d, %d\n", lit, -lit);
				//printf("s UNSATISFIABLE\n");
				//exit(20);
			}
		}else if(cnf->clauses[i][2] == 0){	// binary clause
			int lit0 = cnf->clauses[i][0];
			int lit1 = cnf->clauses[i][1];
			imp[SIGN(lit0)][VAR(lit0)].push_back(lit1);	//** create the implication lists, if VAR(lit0) is evaluated positive, 
			imp[SIGN(lit1)][VAR(lit1)].push_back(lit0); //** then imp[0][VAR(lit0)] will contain the alternative litteral that must be true (and conversely)
			vars[VAR(lit0)].activity[SIGN(lit0)]++;
			vars[VAR(lit1)].activity[SIGN(lit1)]++; //** activity is how many times the literal is referenced in the calculus.
		}else{
			// set up watches
			WATCHLIST(cnf->clauses[i][0]).push_back(cnf->clauses[i]);	//** watchlist has positive and negative depending on the literal
			WATCHLIST(cnf->clauses[i][1]).push_back(cnf->clauses[i]);	//** loads the watchlist with the clause index in the litpool with which it is first or second member

			// copy literals to litPool
			//** setting activities as you go.
			//for(j = 0; (*(p++) = cnf->clauses[i][j]); j++){
			//	vars[VAR(*(p-1))].activity[SIGN(*(p-1))]++;
			//}
			for(j = 0; cnf->clauses[i][j]; j++){
				vars[VAR(cnf->clauses[i][j])].activity[SIGN(cnf->clauses[i][j])]++;
			}
		}
	}
	//** litpoolCapacity is alloc'ed memory block size, litpoolSize/Orig is actual length of all non-unary&binary clauses
	litPoolSizeOrig = (p - litPool);
	litPoolSize = litPoolSizeOrig;
	
	// binary clause implication lists
	for(i = 1; i <= vc; i++) for(j = 0; j <= 1; j++){
		// last element is 0
		// first three elements are ([], lit, 0)
		// serve as antecedent for all implications
		// serve as conflicting clause with [] filled
		TEST_NOT_NULL(vars[i].imp[j] = (int *) calloc(imp[j][i].size() + 4, sizeof(int))) //** implication lists are static, calloc is appropriate
		vars[i].imp[j][1] = i * ((j == _POSI)?1:-1);						//** a variable always implies its negative is false.
		unsigned k = 3;
		for(vector<int>::iterator it = imp[j][i].begin(); it != imp[j][i].end(); it++) //** directly load the contents of the implication list into the variable's implication list
			vars[i].imp[j][k++] = *it;
	}

	// assert unit clauses
	assertUnitClauses();
}

CnfManager::~CnfManager(){
	for(vector<int *>::iterator it = litPools.begin(); it != litPools.end(); free(*(it++)));
	while(*(--stackTop)) { }
       	free(stackTop);
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

//** set the variable to have the value of the literal, setting ante to be the variable basis for the decision, at the current dLevel
void CnfManager::setLiteral(int lit, int *ante){
	vars[VAR(lit)].value = SIGN(lit);
	vars[VAR(lit)].ante = ante;
	vars[VAR(lit)].dLevel = dLevel;
	num_set_literal++;
}

//** assert the value of the literal, and deal with all implications, adding them to the stack
//** and processing the implication's implications respectively until contradiction or natural finish
//** returning false on contradiction or true if natural finish.
//** updating the watchlists of longer clauses
bool CnfManager::assertLiteral(int lit, int *ante){
	int *newStackTop = stackTop;
	*newStackTop = lit;
	newStackTop += 1;
	setLiteral(lit, ante);
	DB(printf("%d: %d =>", dLevel, lit);)

	while(stackTop < newStackTop){
		// the literal resolved (as opposed to set)
		//** get the NEGATIVE litteral at stacktop and increment
		int lit = NEG(*stackTop);
		stackTop += 1;

		// implications via binary clauses
		int *impList = IMPLIST(lit);
		for(int *imp = impList + 3; true; imp++){
			// implication
			if(FREE(*imp)){
				if(*imp == 0) break; 	// end of list
				DB(printf(" %d", *imp);)
				*newStackTop = *imp;
				newStackTop++;
				setLiteral(*imp, impList + 1); 
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


// add a clause stored in conflictLits
// return the index added in clauses, otherwise -1 if not added to list.
int CnfManager::addClause(){
	unsigned size = conflictLits.size();

	// create new litPool if necessary
	if(litPoolSize + size + 1 > litPoolCapacity){
		do litPoolCapacity *= 2; while(size+1 > litPoolCapacity);
		litPool = (int *) malloc(litPoolCapacity * sizeof(int));
		while(litPool == NULL && litPoolCapacity*2 > size+1){
			litPoolCapacity /= 2;
			litPool = (int *) malloc(litPoolCapacity * sizeof(int));
		}
		TEST_NOT_NULL(litPool)
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
	if (size>1) {
	  return clauses.size()-1;
	} else {
	  return -1;
	}
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


void CnfManager::printSolution(FILE *ofp) {
  for (unsigned i = 1; i <= vc; i++)
    if (vars[i].value == _POSI)
      fprintf(ofp, "%d ", i);
    else if (vars[i].value == _NEGA)
      fprintf(ofp, "-%d ", i);
  fprintf(ofp, "0\n");
}

void CnfManager::printSolution(std::ostream &os) {
  for (unsigned i = 1; i <= vc; i++)
    if (vars[i].value == _POSI)
      os << i << " ";
    else if (vars[i].value == _NEGA)
      os << "-" << i << " ";
  os << "0";
}

void CnfManager::load_into_message(Message* m) {
  m->assignments.clear();
  for (unsigned i = 1; i <= vc; i++) {
    if (vars[i].value == _POSI)
      m->assignments.push_back(i);
    else if (vars[i].value == _NEGA)
      m->assignments.push_back(-i);
  }
}

void CnfManager::load_into_message(Message* m, RangeSet &r) {
  m->assignments.clear();
  for (auto var = r.buffer.begin(); var != r.buffer.end(); var++) {
    for (int variable = (*var).first; variable <= (*var).second; variable++) {
      if (variable <= vc) {
        if (vars[variable].value == _POSI) {
          m->assignments.push_back(variable);
        } else if (vars[variable].value == _NEGA) {
          m->assignments.push_back(-variable);
        }
      }
    }
  }
}

void CnfManager::load_into_deque(deque<int> &d, RangeSet &r) {
  load_into_deque(d,r,false);
}
void CnfManager::load_into_deque(deque<int> &d, RangeSet &r, bool positives_only) {
  d.clear();
  for (auto var = r.buffer.begin(); var != r.buffer.end(); var++) {
    for (int variable = (*var).first; variable <= (*var).second; variable++) {
      if (variable <= vc) {
        if ((vars[variable].value == _POSI) && (!positives_only)) {
          d.push_back(variable);
        } else if (vars[variable].value == _NEGA) {
          d.push_back(-variable);
        }
      }
    }
  }
}


void CnfManager::load_into_deque(deque<int> &d) {
  load_into_deque(d,false);
}
void CnfManager::load_into_deque(deque<int> &d, bool positives_only) {
  d.clear();
  for (int variable = 1; variable <= vc; variable++) {
    if (variable <= vc) {
      if ((vars[variable].value == _POSI) && (!positives_only)) {
        d.push_back(variable);
      } else if (vars[variable].value == _NEGA) {
        d.push_back(-variable);
      }
    }
  }
}


void CnfManager::load_marked_into_message(Message* m) {
  m->assignments.clear();
  for (unsigned i = 1; i <= vc; i++) {
    if (vars[i].mark2) {
      if (vars[i].value == _POSI)
        m->assignments.push_back(i);
      else if (vars[i].value == _NEGA)
        m->assignments.push_back(-i);
    }
  }
}

bool CnfManager::is_solver_unit_contradiction() {
  return solver_unit_contradiction;
}


bool CnfManager::assertCL(){
	return assertLiteral(*conflictClause, conflictClause + 1);
}

bool CnfManager::decide(int lit){
	dLevel++;
	return assertLiteral(lit, NULL);
}

void CnfManager::backtrack(unsigned bLevel){
	for(unsigned var; vars[var = VAR(*(stackTop - 1))].dLevel > bLevel;){
		if(vars[var].dLevel < dLevel) vars[var].phase = vars[var].value;
		vars[var].value = _FREE;
		if(varPosition[var] < nextVar) nextVar = varPosition[var];
		stackTop--;
	}
	dLevel = bLevel;
}

void CnfManager::scoreDecay(){
	// this may slightly disturb var order
	// e.g., (7 + 7) => (3 + 3) whereas (6 + 8) => (3 + 4)
	for(unsigned i = 1; i <= vc; i++){
		vars[i].activity[0]>>=1;
		vars[i].activity[1]>>=1;
	}
}
