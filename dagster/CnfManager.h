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

#ifndef _CNF_MANAGER
#define _CNF_MANAGER
#include <vector>
#include <deque>
#include <cstddef>
#include "math.h"
#include "Cnf.h"
#include "message.h"
using namespace std;

#define _NEGA 0		// variable values are negative
#define _POSI 1		// positive, or
#define _FREE 2		// free

#define SIGN(lit) 	(((lit) > 0)?_POSI:_NEGA)
#define VAR(lit)	(abs(lit))
#define NEG(lit)	(-(lit))

#define FREE(lit) 	(vars[VAR(lit)].value == _FREE) //** returns true if the litteral's variable is free valued
#define SET(lit)	(vars[VAR(lit)].value == SIGN(lit)) //** returns true if variable already resolved in the sign of the supplied litteral
#define RESOLVED(lit)	(vars[VAR(lit)].value == SIGN(NEG(lit))) //** returns true if variable allready resolved oppositely to the sign of the supplied literal

#define IMPLIST(lit) 	(vars[VAR(lit)].imp[SIGN(lit)]) //** return the implication list of the literal
#define WATCHLIST(lit) 	(vars[VAR(lit)].watch[SIGN(lit)]) //** return the watch-list of the literal
#define SCORE(var)	(vars[(var)].activity[0]+vars[(var)].activity[1]) //** returns the score of the variable, the sum of its activities (which decay periodically) indicative of its prsence positive and negative. 

struct Variable{
	bool mark;			// used in 1-UIP derivation - in learnClause() function
	bool mark2;			// used in detecting variable redundancy - in verify_and_trim_Solution() function
	bool phase;			// suggested phase for decision - inherited from whether variable is more commonly positive or negative in original CNF - in inherited SatSolver() constructor
	char value;			// _POSI, _NEGA, _FREE
	unsigned dLevel;		// decision level where var is set
	int *ante;			// antecedent clause if implied
	unsigned activity[2];		// scores for literals
	int *imp[2];			// implication lists for binary clauses
	vector<int *> watch[2];		// watch lists for other clauses
	Variable(): mark(false), value(_FREE){
	 	activity[0] = activity[1] = 0; 
		watch[0].clear(); watch[1].clear();
	};
};

class CnfManager{
public:
	Cnf* cnf;

	unsigned vc; 			// variable count 
	Variable *vars; 		// array of variables
	unsigned *varOrder;		// variables ordered by score
	unsigned *varPosition;		// variable position in varOrder
	unsigned nextVar;		// starting point in varOrder

	int *litPool; 			// array of literals as in clauses
	unsigned litPoolSize; 		// literal pool size
	unsigned litPoolSizeOrig; 	// original clauses only
	unsigned litPoolCapacity;	// capacity of current litPool
	vector<int *> litPools;		// all litPools created
	vector<int *> clauses;		// array of conflict clauses 
	int nextClause; 		// starting point to look for unsatisfied conflict clause	

	int *stackTop; 			// decision/implication stack
	unsigned aLevel;		// assertion level
	unsigned dLevel; 		// decision level
	unsigned nConflicts;		// num of conflicts
	deque<int> conflictLits;	// stores conflict literals
	deque<int> tmpConflictLits;	// ditto, temporary
	int *conflictClause;		// points to learned clause in litPool 

	void setLiteral(int lit, int *ante);	// set value, ante, level 
	bool assertLiteral(int lit, int *ante);	// set literal and perform unit propagation
	bool assertUnitClauses();		// assert initial unit clauses
	bool decide(int lit);			// increment dLevel and call assertLitreal
	void learnClause(int *firstLit);	// store learned clause in conflictLits and call addClause
	virtual int addClause();			// add conflictLits to litPool and set up watches
	bool assertCL();			// assert literal implied by conflict clause
	void backtrack(unsigned level);		// undo assignments in levels > level 
	void scoreDecay();			// divide scores by constant
	void updateScores(int *first);		// update variable scores and positions

	CnfManager(Cnf* cnf);
	~CnfManager();

	void printSolution(FILE *);
	void printSolution(std::ostream &);
	void load_into_message(Message* m);
	void load_into_message(Message* m, RangeSet &r);
	void load_into_deque(deque<int> &d, RangeSet &r);
	void load_into_deque(deque<int> &d);
	void load_into_deque(deque<int> &d, RangeSet &r, bool positives_only);
	void load_into_deque(deque<int> &d, bool positives_only);
	void load_marked_into_message(Message* m);
    bool solver_unit_contradiction;
};

inline bool CnfManager::assertCL(){
	return assertLiteral(*conflictClause, conflictClause + 1);
}

inline bool CnfManager::decide(int lit){
	dLevel++;
	return assertLiteral(lit, NULL);
}

inline void CnfManager::backtrack(unsigned bLevel){
	for(unsigned var; vars[var = VAR(*(stackTop - 1))].dLevel > bLevel;){
		if(vars[var].dLevel < dLevel) vars[var].phase = vars[var].value;
		vars[var].value = _FREE;
		if(varPosition[var] < nextVar) nextVar = varPosition[var];
		stackTop--;
	}
	dLevel = bLevel;
}

inline void CnfManager::scoreDecay(){
	// this may slightly disturb var order
	// e.g., (7 + 7) => (3 + 3) whereas (6 + 8) => (3 + 4)
	for(unsigned i = 1; i <= vc; i++){
		vars[i].activity[0]>>=1;
		vars[i].activity[1]>>=1;
	}
}
#endif
