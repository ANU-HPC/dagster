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
#include <math.h>
#include <algorithm>
#include <functional>
#include "SatSolver.h"
#include "../mpi_global.h"
#include <mpi.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

double getCpuTime(){ 
	struct rusage usage;
  	getrusage(RUSAGE_SELF, &usage);
  	return (usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) * 
	(1e-6) + (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec); 
}

#define HALFLIFE 128
#define _DT 32		// RSAT phase selection threshold 

struct compScores: public binary_function<unsigned, unsigned, bool>{
	Variable *vars;
	compScores(Variable *myVars) : vars(myVars) {}
	bool operator()(unsigned a, unsigned b) const{ 
		return SCORE(a) > SCORE(b); 
	}
};

SatSolver::SatSolver(Cnf &cnf, double _start_time, int decision_interval, int suggestion_size): CnfManager(cnf){
	// initialize parameters 
	
	if(decision_interval == 0) Decision_Interval = DECISION_INTERVAL;
	else Decision_Interval = decision_interval;

	if(suggestion_size == 0) Suggestion_Size = SUGGESTION_SIZE;
	else Suggestion_Size = suggestion_size;

	startTime = _start_time;
	prevTime = _start_time;

	nextRestart = luby.next() * (lubyUnit = 512);
	nextDecay = HALFLIFE;

	MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
	numSLSProcesses = numProcesses -1;
	processDLevel = (int*)calloc(numSLSProcesses, sizeof(int));
	
	suggestions = (int**)calloc(2, sizeof(int*));
	suggestions[0] = (int*)calloc(Suggestion_Size, sizeof(int));
	suggestions[0][0] = 0;	
	suggestions[1] = (int*)calloc(Suggestion_Size, sizeof(int));
	suggestions[1][0] = 0;
	onGoingGet = false;
	
	prefix = (int*)calloc(vc+1, sizeof(int));
	currentSuggestion = 0;
	currentSuggestionBuffer = 0;
	complete = -1;
	nSends = 0; 
	nGets = 0;
	int *dummy;
	MPI_Win_allocate(0, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &dummy, &window);
	
	if(processDLevel == NULL){
		fprintf(stderr, "Failed to allocate process array\n");
		exit(-1);
	}

	//Initialize process D-Levels
	for(int i = 0; i< numSLSProcesses; i++){
		processDLevel[i] = -1;
	}

	// assertUnitClauses has failed
	if(dLevel == 0) return; 

	// assert pure literals
	for(int i = 1; i <= (int) vc; i++) if(vars[i].value == _FREE){
		if(vars[i].activity[_POSI] == 0 && vars[i].activity[_NEGA] > 0)
			// ante is NULL, as opposed to empty clause for implied literals
			assertLiteral(-i, NULL);	
		else if(vars[i].activity[_NEGA] == 0 && vars[i].activity[_POSI] > 0)
			assertLiteral(i, NULL);
	}

	// initialize varOrder
	nVars = 0; for(unsigned i = 1; i <= vc; i++) 
		if(vars[i].value == _FREE && SCORE(i) > 0){
			varOrder[nVars++] = i;
			vars[i].phase = (vars[i].activity[_POSI] > vars[i].activity[_NEGA])?_POSI:_NEGA;
		}
	sort(varOrder, varOrder + nVars, compScores(vars));
	for(unsigned i = 0; i < nVars; i++) varPosition[varOrder[i]] = i; 
	nextVar = 0; 
	nextClause = clauses.size() - 1; 
	nSuggestions = 0;
	nSuggestionsTaken = 0;
	currentSLS = 0;
}

SatSolver::~SatSolver(){
	if(onGoingGet){
		MPI_Win_unlock(lockedSLS, window);
	}
	MPI_Win_free(&window);
	free(prefix);
	free(processDLevel);
	free(suggestions);
}	

int SatSolver::selectLiteral__sls(){
  unsigned x = 0;
  
  if (numSLSProcesses > 0) {
    int decisions = dLevel - 1;
    int tier = decisions/Decision_Interval;
    int dLevelIndex = tier*Decision_Interval;

    // print a graphical representation of current decision level
    /*
      printf("\r");
      for (int i=0; i<dLevel; i++) {
      if (i%DECISION_INTERVAL==0) printf("|");
      else printf("*");
      }
      for (int i=dLevel; i<80; i++) printf(" ");
    */
    if(decisions % Decision_Interval == 0 && processDLevel[currentSLS] != dLevelIndex){
      // Setup SLS process for this decision level
      //fprintf(stdout, "\nAssigning SLS process: %d, to dLevel: %d\n",
      //		currentSLS+1, dLevel);
			
      processDLevel[currentSLS] = dLevel;
			
      if(nSends>0){
	MPI_Wait(&prefixLengthRequest, MPI_STATUS_IGNORE);
	MPI_Wait(&prefixRequest, MPI_STATUS_IGNORE);
      }

      int prefixLength = 0;
      int currentIndex = 0;
      for(int i = 1; i<=vc; i++){
	if(!FREE(i)){
	  prefix[currentIndex] = (vars[VAR(i)].value==_POSI?1:-1)*i;
	  prefixLength++;
	  currentIndex++;
	}
      }
			
      //fprintf(stderr, "Tinisat -- Prefix Length: %d\n", prefixLength);
      if (prefixLength > 0) {
        nSends++;
        MPI_Isend(&prefixLength, 1, MPI_INT, currentSLS+1, 
                  PREFIX_LENGTH_TAG, MPI_COMM_WORLD, &prefixLengthRequest);
        MPI_Isend(prefix, prefixLength, MPI_INT, currentSLS+1, 
                  PREFIX_TAG, MPI_COMM_WORLD, &prefixRequest);

        currentSLS = (currentSLS+1) % numSLSProcesses;
      }
    } else {
      if (currentSuggestion >= Suggestion_Size || suggestions[currentSuggestionBuffer][currentSuggestion] == 0) {
				
	// There are no more suggestions. Try to get some more.
	int highestDLevelBelowCurrent = 0;
	int bestSLS = -1;
	int nextSuggestionBuffer = (currentSuggestionBuffer+1)%2;
	suggestions[currentSuggestionBuffer][0] = 0;

	for (int i=0; i<numSLSProcesses; i++) {
	  if (processDLevel[i] > highestDLevelBelowCurrent && processDLevel[i] < dLevel) {
	    highestDLevelBelowCurrent = processDLevel[i];
	    bestSLS = i;
	  }
	}
	if (bestSLS >= 0) {
					
	  if(onGoingGet){ /*We already have a get into the backbuffer*/
	    MPI_Win_unlock(lockedSLS, window);
	    nGets++;
	    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, bestSLS+1, 
			 MPI_MODE_NOCHECK, window);
	    MPI_Get(suggestions[currentSuggestionBuffer], 
		    Suggestion_Size, MPI_INT, bestSLS+1,
		    0, Suggestion_Size, MPI_INT, window);
	    onGoingGet = true;
	    lockedSLS = bestSLS+1;
	    currentSuggestionBuffer = nextSuggestionBuffer;

	  }
	  else{/*We aren't getting any suggestions*/
	    nGets++;
	    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, bestSLS+1, 
			 MPI_MODE_NOCHECK, window);
	    MPI_Get(suggestions[currentSuggestionBuffer], 
		    Suggestion_Size, MPI_INT, bestSLS+1,
		    0, Suggestion_Size, MPI_INT, window);
	    MPI_Win_unlock(bestSLS+1, window);
						
	    nGets++;
	    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, bestSLS+1, 
			 MPI_MODE_NOCHECK, window);
	    MPI_Get(suggestions[nextSuggestionBuffer], 
		    Suggestion_Size, MPI_INT, bestSLS+1,
		    0, Suggestion_Size, MPI_INT, window);
	    onGoingGet = true;
	    lockedSLS = bestSLS+1;

	  }
					
	  currentSuggestion = 0;

	  // Get suggestion from SLS process
	  //printf("Receiving from SLS process: %d, at dLevel: %d\n",
	  //bestSLS+1, dLevel);
			
	  /*nGets++;
	    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, bestSLS+1, MPI_MODE_NOCHECK, window);
	    MPI_Get(suggestions[currentSuggestionBuffer], SUGGESTION_SIZE, MPI_INT, bestSLS+1,
	    0, SUGGESTION_SIZE, MPI_INT, window);
	    MPI_Win_unlock(bestSLS+1, window);
		
	    currentSuggestion = 0;*/
	}
      }

      while (currentSuggestion < Suggestion_Size && suggestions[currentSuggestionBuffer][currentSuggestion] != 0) {
	// still have more suggestions to try
	int s = suggestions[currentSuggestionBuffer][currentSuggestion];
	currentSuggestion++;
	nSuggestions++;
	if(s == 0 || VAR(s) > vc) continue;
	if(vars[VAR(s)].value == _FREE){
	  //fprintf(stderr, "Using Suggestion %d\n", s);
	  nSuggestionsTaken++;
	  return s;
	}
      } 
	
    }
  } // if (numSLSProcesses > 0)

  return x;
}


int SatSolver::selectLiteral__vsids(){
  unsigned x = 0;
  for(unsigned i = nextVar; i < nVars; i++){
    if(vars[varOrder[i]].value == _FREE){
      x = varOrder[i];
      nextVar = i + 1;

      // RSAT phase selection
      int d = vars[x].activity[_POSI] - vars[x].activity[_NEGA];
      if(d > _DT) return x; else if(-d > _DT) return -x;
      else return (vars[x].phase == _POSI)?(x):-(int)(x);
    }
  }
  return 0;
}

int SatSolver::selectLiteral__conflict(){
  unsigned x = 0;
  
  // pick best var in unsatisfied conflict clause nearest to top of stack
  // but only search 256 clauses
  int lastClause = nextClause > 256 ? (nextClause - 256) : 0;
  for(int i = nextClause; i >= lastClause; i--){
    int *p = clauses[nextClause = i];

    // skip satisfied clauses
    bool sat = false; 
    for(; (*p); p++) if(SET(*p)){ sat = true; break; }
    if(sat) continue;

    // traverse again, find best variable of clause
    int score = -1;
    for(p = clauses[i]; (*p); p++) if(FREE(*p) && ((int) SCORE(VAR(*p))) > score){
	x = VAR(*p); score = SCORE(x);
      }

    // RSAT phase selection
    int d = vars[x].activity[_POSI] - vars[x].activity[_NEGA];
    if(d > _DT) return x; else if(-d > _DT) return -x;
    else return (vars[x].phase == _POSI)?(x):-(int)(x);
  }

  return x;
}
  
int SatSolver::selectLiteral(){
  unsigned x = 0;
  if (slsfirst == heuristic_rotation){ // The first thing to try is to select a variable using an SLS process. 
    x = selectLiteral__sls();
  }
  if ( 0 == x){
    x = selectLiteral__conflict();
  }

  if (0 == x && cdclfirst == heuristic_rotation){ // Local search has not been tried/successful, and we attempted to choose a variable from a conflict clause.
    x = selectLiteral__sls();
  }
  
  if (0 == x){// USE VSIDS
    x = selectLiteral__vsids();
  }

  //assert(0 != x);
  
  return x;
}

bool SatSolver::run(){
	if(dLevel == 0) return false; 		// assertUnitClauses has failed

	MPI_Irecv(&complete, 1, MPI_INT, MPI_ANY_SOURCE, COMPLETE_TAG, MPI_COMM_WORLD, &completionRequest);

	for(int lit; (lit = selectLiteral());){ // pick decision literal
		
		if(STATS_FREQUENCY != 0 && !(nDecisions%STATS_FREQUENCY)) printStatsInline();
		
		if(!decide(lit)) do{ 		// decision/conflict
			int incoming;
			MPI_Test(&completionRequest, &incoming, MPI_STATUS_IGNORE);
			if (incoming != 0) {
				signalCompletion();
				if (nSends > 0) {
					MPI_Cancel(&prefixLengthRequest);
					MPI_Request_free(&prefixLengthRequest);
					MPI_Cancel(&prefixRequest);
					MPI_Request_free(&prefixRequest);
				}
				// one of the SLS processes found a solution
				return true;
			}
			
			// conflict has occurred in dLevel 1, unsat 
			if(aLevel == 0) {
				complete = -1;
				signalCompletion();
				return false;
			}

			// score decay
			if(nConflicts == nextDecay){
				nextDecay += HALFLIFE;
				scoreDecay();
			}

			// rewind to top of clause stack
			nextClause = clauses.size() - 1; 

			// restart at dLevel 1 
			if(nConflicts == nextRestart){
				nRestarts++;
				nextRestart += luby.next() * lubyUnit;
				for (int i=0; i<numSLSProcesses; i++) {
					processDLevel[i] = -1;
				}

				backtrack(1);
				if(dLevel != aLevel) break;

			// partial restart at aLevel 
			} else {
				// after backtracking, ignore any SLS instances with a prefix above aLevel
				for (int i=0; i<numSLSProcesses; i++) {
					if (processDLevel[i] > aLevel) processDLevel[i] = -1;
				}
				backtrack(aLevel);
			}
		}while(!assertCL());		// assert conflict literal
	}
	complete = 0;
	signalCompletion();
	MPI_Cancel(&completionRequest);
	if(!verifySolution()){ printf("s UNKOWN\n"); MPI_Abort(MPI_COMM_WORLD, -1); }
	return true;	
}

// send a prefix length 0 to each process, informing them of completion
void SatSolver::signalCompletion() {
	for (int i=1; i<=numSLSProcesses; i++) {
		if (i != complete) {
			int noPrefix = 0;
			MPI_Send(&noPrefix, 1, MPI_INT, i, PREFIX_LENGTH_TAG, MPI_COMM_WORLD);
		}
	}
}

bool SatSolver::verifySolution(){
	int lit, *pool = litPools[0];
	for(unsigned i = 0; i < litPoolSizeOrig;){
		bool satisfied = false;
		while((lit = pool[i++])) if(SET(lit)){
			satisfied = true;
			while(pool[i++]);
			break;
		}
		if(!satisfied) return false;
	}
	return true;
}

void SatSolver::printSolution(FILE *ofp){
	fprintf(ofp, "Tinisat -- ");
	for(unsigned i = 1; i <= vc; i++)
		if(vars[i].value == _POSI) fprintf(ofp, "%d ", i);
		else if(vars[i].value == _NEGA) fprintf(ofp, "-%d ", i);
	fprintf(ofp, "0\n");
}

void SatSolver::printStatsInline(){
	double timeTaken = getCpuTime() - startTime;
	double deltaTime = getCpuTime() - prevTime;
	printf("[%.2f](%d) Decisions/s(overall/instant): %.2f/%.2f, Suggestions taken/received %d/%d, Sends/Gets %d/%d\n", timeTaken, nDecisions, nDecisions/timeTaken, (nDecisions-nDecisionsPrev)/deltaTime,nSuggestionsTaken, nSuggestions, nSends, nGets);
	prevTime = getCpuTime();
	nDecisionsPrev = nDecisions;
}

void SatSolver::printStats(){
	double timeTaken = getCpuTime() - startTime;

	printf("Tinisat -- c %d decisions, %d conflicts, %d restarts\n", nDecisions, nConflicts, nRestarts);
	printf("Time Elapsed: %.2f, Decisions per second: %0.2f\n", timeTaken, nDecisions/timeTaken);
	printf("Suggestions received %d taken %d\n", nSuggestions, nSuggestionsTaken);
	printf("Sends Performed: %d, Gets Performed: %d\n", nSends, nGets);

}
