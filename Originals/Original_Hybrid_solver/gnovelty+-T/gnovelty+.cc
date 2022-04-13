/* gNovelty+, version 1.1                                 */
/*                                                        */
/*      A gradient based Novelty+ for CDCL guidance       */
/*                                                        */
/*      1. Charles Gretton (charles.gretton@anu.edu.au)   */ 
/*            Australian National University              */
/*      2. Josh Milthorpe (josh.milthorpe@anu.edu.au)     */ 
/*            Australian National University              */
/*      3. Tate Kennington (tatekennington@gmail.com)     */ 
/*            University of Dunedin, New Zealand          */
/*                                                        */
/*                                                        */
/**********************************************************/
/* Based on -- gNovelty+, version 1.0                     */
/*                                                        */
/*      A greedy gradient based Novelty+                  */
/*                                                        */
/*      1. Duc Nghia Pham (duc-nghia.pham@nicta.com.au)	  */ 
/*            SAFE Program,  National ICT Australia Ltd.  */
/*            IIIS, Griffith University, Australi         */
/*                                                        */
/*      2. Charles Gretton (charles.gretton@gmail.com)    */ 
/*            University of Birmingham                    */
/*            IIIS, Griffith University, Australia        */
/*                                                        */
/*                                                        */
/**********************************************************/
/*    Consult legal.txt for legal information		  */
/**********************************************************/
/* Partial of the code is based on UBCSAT version 1.0     */
/* written by:                                            */
/* Dave A.D. Tompkins & Holger H. Hoos                    */
/*   Univeristy of British Columbia                       */
/* February 2004                                          */
/**********************************************************/

/************************************/
/* Standard includes                */
/************************************/
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <cerrno>
// #include <cmalloc>
#include <mpi.h>
#include "../mpi_global.h"
//#define IS_TRUE(Z) (atom[Z] > 0)
#define IS_TRUE(Z) (state.isTrue(Z))



/* #include "fwBase.h" */
/* #include "fwSignal.h" */

#include "global.hh"
//#include "resolvent.h"
#include "rtime.hh"

/************************************/
/* Compilation flags                */
/************************************/
#define WEIGHTING 1

//#define DETERMINISTIC_SMOOTHING 1


inline double max(double a, double b) {
    return (a > b) ? a : b;
}

inline double min(double a, double b) {
    return (a < b) ? a :b;
}

#define _EXP(X) max(0.001, 1.0 + (X)) /*Fake update that is faster than machine exponent.*/

// //=== Weighting controlled variables
// CWTYPE clauseWeight[MAXCLAUSE];		// weight of a clause
// CWTYPE weightDiff[MAXATOM+1];		// the total weight difference if a variable if flipped
// CWTYPE weightTotal;					// total clause weight

// CWTYPE pseudoStepSize;				// initialised in main()
// CWTYPE trace[MAXCLAUSE];
// CWTYPE traceDiscount;
// CWTYPE stepSize[MAXCLAUSE];

// int numWeight;						// number of clauses with weight > 1
// int weightedClause[MAXCLAUSE];		// clauses which have weight > 1:
// int whereWeight[MAXCLAUSE];		// same as for whereFalse:
// int /*smoothStages -- already defined in global,*/ weightCounter;

// int smoothProb = 100;

// //=== AdaptNovelty+ controlled variables
// int varLastChange[MAXATOM+1];
// int lastAdaptFlip;
// int lastAdaptNumFalse;

// int adaptFlag = 1;


#define _weight_getScore(VAR)		(weightDiff[VAR])
#define _weight_setScore(VAR, VAL)	(weightDiff[VAR] = (CWTYPE) VAL)
#define _weight_adjScore(VAR, VAL)	(weightDiff[VAR] += VAL)
#define _weight_incScore(VAR, CLS)	(weightDiff[VAR] += clauseWeight[CLS])
#define _weight_decScore(VAR, CLS)	(weightDiff[VAR] -= clauseWeight[CLS])

#define _num_getScore(VAR)		(numDiff[VAR])
#define _num_setScore(VAR, VAL)	(numDiff[VAR] = VAL)
#define _num_adjScore(VAR, VAL)	(numDiff[VAR] += VAL)
#define _num_incScore(VAR, CLS)	(numDiff[VAR] += 1)
#define _num_decScore(VAR, CLS)	(numDiff[VAR] -= 1)

#define setScore(VAR, VAL) {                    \
	_weight_setScore(VAR, VAL);		\
	_num_setScore(VAR, VAL);		\
    }

#define adjScore(VAR, VAL) {                    \
	_weight_adjScore(VAR, VAL);		\
	_num_adjScore(VAR, VAL);		\
    }

#define incScore(VAR, CLS) {                    \
	_weight_incScore(VAR, CLS);		\
	_num_incScore(VAR, CLS);		\
    }

#define decScore(VAR, CLS) {                    \
	_weight_decScore(VAR, CLS);		\
	_num_decScore(VAR, CLS);		\
    }


/*
  #define _num_getScore(VAR)		(numDiff[VAR])
  #define setScore(VAR, VAL)	(numDiff[VAR] = VAL)
  #define adjScore(VAR, VAL)	(numDiff[VAR] += VAL)
  #define incScore(VAR, CLS)	(numDiff[VAR] += 1)
  #define decScore(VAR, CLS)	(numDiff[VAR] -= 1)

  #define _weight_getScore(VAR)		(numDiff[VAR])
  #define _weight_adjScore(VAR, VAL)	(numDiff[VAR] += VAL)
*/

/**********************************************************/
/* ANOV parameters                                        */
/**********************************************************/


//PROBABILITY noise = 0;		// default noise (in percent)

//PROBABILITY walkProb = 5;	// default random walk probability
// PROBABILITY noise = 50;		// default noise (in percent)

int invPhi = 5;
int invTheta = 6;

/**********************************************************/
/*   Forward declarations                                 */
/**********************************************************/
void parseParameters(int argc,char *argv[]);
void parseParametersComp(int argc,char *argv[]);
void scanInt(int argc, char *argv[], int i, int *varptr);
void scanFloat(int argc, char *argv[], int i, float *varptr);
void printHeader();
void printStatsHeader();
int  compareAbsInts(const void *a, const void *b);
void initInstance();
// void init();
// int  pickVar();
// void flipAtom(int toflip);
// void updateClauseWeights_Linear();
// void updateClauseWeights_Exp();
// void updateClauseWeights_NULL();
// void smooth();
// void adaptNoveltyNoise();
// void updateStatsEndFlip();
// void updateStatsEndTry();
// void printStatsEndTry();
// CWTYPE computeCurrentCost();
// void printFinalStats();
// void printFinalStatsComp();
// void saveSolution();
void printSolution();

int *neighbourVar[MAXATOM+1];


// TRUE iff heuristic advise to CDCL is based on "ghost"-variable assignments
bool using__ghost_suggestions; // see \function{main} for initialisation/assignment

int minClauseSize, maxClauseSize;
int decisionInterval = DECISION_INTERVAL, suggestionSize = SUGGESTION_SIZE;
// void (*updateClauseWeights)() = NULL;


void* runSearchInstanceInThread(void* _searchInstance)
{
  int Nghia_tryAgain;
  double starttime;
    
  SearchInstance& searchInstance = *reinterpret_cast<SearchInstance*>(_searchInstance);
  VERBOSER(1, "Made it into the search thread.\n");

  searchInstance.totalFlips = searchInstance.totalNullFlips = 0;
  searchInstance.totalSuccessFlips = searchInstance.totalSuccessNullFlips = 0;
  searchInstance.numSuccessTries = 0;
        
  if ( (minClauseSize == 3) && (maxClauseSize == 3) ) {
    searchInstance.updateClauseWeights = &SearchInstance::updateClauseWeights_Linear;//.updateClauseWeights_Linear;
    searchInstance.smoothProb = 40;
    searchInstance.adaptFlag = 1;
    searchInstance.noise = 0;
  } else {
    //updateClauseWeights = &updateClauseWeights_Exp;
    searchInstance.updateClauseWeights = &SearchInstance::updateClauseWeights_NULL;
    searchInstance.adaptFlag = 1;
    searchInstance.noise = 0;
  }
	
  //adaptFlag = 0;

  searchInstance.printStatsHeader();
  initTimeCounter();
  int complete = 0;
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  //printf("seed = %d\n", seed);
  int *suggestions;
  MPI_Win window;
  MPI_Win_allocate(suggestionSize*sizeof(int), sizeof(int),
		   MPI_INFO_NULL, MPI_COMM_WORLD, &suggestions, &window);
  VERBOSER(1, "gnovelty -- Created Window\n");

  /* Data structures to support a mechanism to communicate recent SLS decisions to the CDCL procedure, as assignment recommendations. */
  int _ghost__suggestion_index = 0;
  vector<int> _ghost__suggestions(SUGGESTION_SIZE, 0);
    
  lowCost = BIG;
  for (Nghia_tryAgain = 1; Nghia_tryAgain <= numTries; Nghia_tryAgain ++) {
        
    starttime = elapsedTime();
    searchInstance.flip = searchInstance.nullFlip = 0;

        
    // required for the stepsize weighting
    searchInstance.pseudoStepSize = pow(10.0, -15);
    searchInstance.traceDiscount = 0.99;//static_cast<CWTYPE>(0.99);

    searchInstance.init();
		
    //numWalks = numDBests = numFreqs = numSBests = 0;

    for(int i = 0; i<suggestionSize; i++){
      suggestions[i] = 0;
    }

    /* Number of iterations of SLS */
    long int count = 0;

    MPI_Request prefixRequest;
    MPI_Recv_init(&searchInstance.nextPrefixLength, 1, MPI_INT, 0, PREFIX_LENGTH_TAG, MPI_COMM_WORLD, &prefixRequest);
    MPI_Start(&prefixRequest);

    //while ((numFalse > targetCost) && (flip - nullFlip < cutoff)) {
    while (/*count<1000000*/TINISAT_ONLY || searchInstance.numFalse > targetCost) {
      searchInstance.flip++;
      //if(!(searchInstance.flip % 100000)) 	fprintf(stderr, "%d ", searchInstance.flip);
                        
      if(searchInstance.numFalse > 0){ 
		    
	searchInstance.var = searchInstance.pickVar();
	//	fprintf(stderr, " -- %d, flip %d --", flip, var);
	//	fprintf(stderr, " -- flip %d --", var);

	//assert(var != NOVALUE);
			
	searchInstance.flipAtom(searchInstance.var);
		
	if (using__ghost_suggestions && NOVALUE != searchInstance.var){
	  _ghost__suggestions[_ghost__suggestion_index] = searchInstance.var;
	  _ghost__suggestion_index = (1 + _ghost__suggestion_index) % SUGGESTION_SIZE;
	}

		
	if (searchInstance.var != NOVALUE) {
	  searchInstance.varLastChange[searchInstance.var] = searchInstance.flip;
	}
				
	if (searchInstance.adaptFlag && searchInstance.flags){
                
	  searchInstance.adaptNoveltyNoise();
	}
            
			
	searchInstance.updateStatsEndFlip();
            
	/*if(searchInstance.saturation()){
	  std::cerr<<"Breaking due to saturation. "<<std::endl;
	  break;
	  }*/
      } // if a solution has not been found yet

      // Communicate with CDCL procedure
      int incoming;
      MPI_Status prefixStatus;
      MPI_Test(&prefixRequest, &incoming, &prefixStatus);
      if(incoming != 0){ // New advise wrt assignment constraint has been communicated

	if(searchInstance.nextPrefixLength == 0){
	  VERBOSER(1, "gnovelty -- Breaking sat found\n");
	  goto solved;
	}
	searchInstance.prefixLength = searchInstance.nextPrefixLength;
	for(int i = 0; i<numAtoms; i++){
	  searchInstance.prefix[i] = 0;
	}

	MPI_Recv(searchInstance.prefix, searchInstance.prefixLength, MPI_INT, 0, 
		 PREFIX_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	MPI_Start(&prefixRequest);	
	
	//fprintf(stderr, "Prefix Length : %d, Buffersize: %d\n", searchInstance.prefixLength, numAtoms);
	//fprintf(stderr,"gnovelty -- Prefix:");
	for(int i = 0; i<searchInstance.prefixLength; i++){
	  int currVar = searchInstance.prefix[i];
	  //fprintf(stderr, " %d", currVar);
	  if(searchInstance.state.isTrue(abs(currVar)) && currVar < 0){
	    searchInstance.flipAtom(abs(currVar));
	  }
	  else if(!searchInstance.state.isTrue(abs(currVar)) && currVar > 0){
	    searchInstance.flipAtom(abs(currVar));
	  } else {
	    // reached the end of the prefix
	    break;
	  }
	}
	//fprintf(stderr,"\n");

	
      } // ABOVE -- Rreceiving advise from CDCD procedure
      else  // BELOW -- Communicating advise to  CDCD procedure
	{ 
	//fprintf(stderr,"gnovelty -- Suggestion:");
	   	
	if (using__ghost_suggestions){
	  int counter = 0;
	  int read_index = (_ghost__suggestion_index == 0)?(SUGGESTION_SIZE - 1):(_ghost__suggestion_index - 1);
	  while (counter < SUGGESTION_SIZE){
	    int currVar = _ghost__suggestions[read_index];
	    // Set a suggestion.
	    if(currVar != 0)suggestions[counter] = searchInstance.state.isTrue(currVar)?currVar:-currVar;
	    read_index = (read_index == 0)?(SUGGESTION_SIZE - 1):(read_index - 1);
	    counter++;
	  }
	} else {
	  
	  bool inList[numAtoms+1];
	  bool inPrefix[numAtoms+1];
	  for(int i = 1; i<=numAtoms; i++){
	    inList[i] = false;
	    inPrefix[i] = false;
	  }
	  for(int i = 0; i < searchInstance.prefixLength &&
		searchInstance.prefix[i] != 0; i++) {
	    inPrefix[abs(searchInstance.prefix[i])] = true;
	  }
	  
	  for(int i = 0; i<suggestionSize; i++){
	    int curr = searchInstance.flip+1;
	    int currVar = 0;
	    for(int j = 1; j<=numAtoms; j++){ /*FORALL problem variables*/
	      int lastChange = searchInstance.varLastChange[j];
	      if(lastChange < curr /*Is epoch when value was last flipped is earlier than current*/
		 && !inList[j]     /*not already selected*/
		 && !inPrefix[j]   /*assignment not forced by CDCL*/){
		currVar = j;
		curr = lastChange;
	      }
	    }
	    // Set a suggestion
	    if(currVar != 0)suggestions[i] = searchInstance.state.isTrue(currVar)?currVar:-currVar;
	    inList[currVar] = true; /*flag as already selected*/
	    //fprintf(stderr," %d", suggestions[i]);
	  }
	}
      }
      count++;
    }

    searchInstance.trytime = elapsedTime() - starttime;
    searchInstance.updateStatsEndTry();
    //printStatsEndTry();
	 	
    complete = rank;
    MPI_Send(&complete, 1, MPI_INT, 0, COMPLETE_TAG, MPI_COMM_WORLD);

    MPI_Cancel(&prefixRequest);
    MPI_Request_free(&prefixRequest);

    //std::cerr<<"Exiting main loop. "<<std::endl;
        
    //printf("rand: %10d, dec: %10d, freq: %10d, fb: %10d, sb: %10d\n", 
    //	numWalks, numDBests, numFreqs, (flip-numWalks-numDBests-numFreqs-numSBests), numSBests);
  }

 solved:
  expertime = elapsedTime();
  if (complete == rank) {
    //printFinalStats();
    searchInstance.printFinalStatsComp(rank);
  }

  //     searchInstance.saveSolution();
  //     printSolution();
  MPI_Win_free(&window);

  /*No need to use the return value of the thread.*/
  return 0;
}


/* Charles-to-Nghia: At the moment I am going to hardcode four threads
 * into the program.*/
vector<SearchInstance*> searchInstances;

/* Charles-to-Nghia: Once a solution is found by one solver, we must
 * stop other solvers from declaring they found a solution also.*/
pthread_mutex_t solution_mutex;

#define DEFAULT_THREADS 1;
uint numThreads;// = DEFAULT_THREADS;

/************************************/
/* Main                             */
/************************************/
int main(int argc,char *argv[]) {

    MPI_Init(NULL, NULL);

    pthread_mutex_init (&solution_mutex, NULL);
    
    //printHeader();
    VERBOSER(1, "Gnovelty Started\n");

    seed = genRandomSeed();
    //parseParameters(argc, argv);
    parseParametersComp(argc, argv);
    srandom(seed);
	
    VERBOSER(1, "Done generation of random seeds.\n");
    
    //processInstance();
    /*PARSING HAPPENS HERE...*/
    initInstance();

    VERBOSER(1, "Done initialisation of instances.\n");
    SearchInstance* searchInstance; 

    uint i = 0;
    for(; i < numThreads - 1; i++){
        std::cerr<<"Starting thread "<<i<<std::endl;
        searchInstance = new SearchInstance(numAtoms, numClauses, 1000000);//CONSTANT+i+1);//random() % 1000);

        pthread_attr_setdetachstate(&searchInstance->attributes, PTHREAD_CREATE_DETACHED);
        pthread_attr_setscope(&searchInstance->attributes, PTHREAD_SCOPE_SYSTEM);
        pthread_attr_setinheritsched(&searchInstance->attributes, PTHREAD_INHERIT_SCHED);
        
        
        if(pthread_create(&searchInstance->thread,
                          &searchInstance->attributes,
                          runSearchInstanceInThread,
                          reinterpret_cast<void*>(searchInstance))){
            UNRECOVERABLE_ERROR("Failed to create thread number :: "<<i<<endl);
        }
//         std::cerr<<"Detaching "<<i<<std::endl;
//         pthread_detach(searchInstance->thread);
//         std::cerr<<"Detached "<<i<<std::endl;
    }
    
    searchInstance = new SearchInstance(numAtoms, numClauses, 1000000);//CONSTANT+i+1);
    runSearchInstanceInThread(reinterpret_cast<void*>(searchInstance));

    delete searchInstance;
 
    MPI_Finalize();

    return 0;
}

//int best[MAXATOM];
//int numBest;

int SearchInstance::pickVar(void) {
    bool DEBUG__choose_bestVar = false;
    
    CWTYPE score, bestValue, secondBestValue;
    int bestVar = NOVALUE;
    int secondBestVar, youngest, lastChange;
    register int i, j, clause, var;
    int *clsptr;
    bool inPrefix[numAtoms+1];
    for(int i = 1; i<=numAtoms; i++){
    	inPrefix[i] = false;
    }
    for(int i = 0; i<prefixLength && prefix[i] != 0; i++){
	    inPrefix[abs(prefix[i])] = true;
	    isCandVar[abs(prefix[i])] = FALSE;
    }
        
    flags = TRUE;
    if (numCandVar > 0) {	//=== greedy step
        bestValue = BIG;  //CWBIG;	//BIG;
        lastChange = flip;

        i = NOVALUE;
        clsptr = candVar;
        for (j = 0; j < numCandVar; j++, clsptr++) {
            var = *clsptr;

	    //fprintf(stderr, "gnovelty -- var : %d\n", var); 

	    //if(inPrefix[abs(var)]){
		//continue;
	    //}

#ifdef USE_setOfStates
            state.flip(var); /* FLIP _A*/
#endif
            
            score = _weight_getScore(var); 
            if (!inPrefix[abs(var)] && score < 0

#ifdef USE_setOfStates
                && (setOfStates.find(state) == setOfStates.end())
#endif
                ) {
                if (score < bestValue) {
                    bestValue = score;
                    lastChange = varLastChange[var];
                    bestVar = var;
                    DEBUG__choose_bestVar = true;
                } else if (score == bestValue) {
                    if (varLastChange[var] < lastChange) {
                        lastChange = varLastChange[var];
                        bestVar = var;
                        DEBUG__choose_bestVar = true;
                    }
                }
            } else {
                            
                i = j;		isCandVar[var] = FALSE;
#ifdef USE_setOfStates
                state.flip(var); /* (1) REVERSE FLIP _A*/
#endif
                break;
            }
#ifdef USE_setOfStates
            state.flip(var); /* (2) REVERSE FLIP _A*/
#endif
        }
        clsptr++;
		
        if (i != NOVALUE) {
            for (j = i+1; j < numCandVar; j++, clsptr++) {
                var = *clsptr;
                
#ifdef USE_setOfStates
                state.flip(var); /* FLIP _B*/
#endif
                
                score = _weight_getScore(var); 
                if (!inPrefix[abs(var)] &&score < 0 
#ifdef USE_setOfStates
                    && (setOfStates.find(state) == setOfStates.end())
#endif
                    

                    ) {
                    candVar[i++] = var;
                    if (score < bestValue) {
                        bestValue = score;
                        lastChange = varLastChange[var];
                        bestVar = var;
                        DEBUG__choose_bestVar = true;
                    } else if (score == bestValue) {
                        if (varLastChange[var] < lastChange) {
                            lastChange = varLastChange[var];
                            bestVar = var;
                            DEBUG__choose_bestVar = true;
                        }
                    }
                } else {
                    isCandVar[var] = FALSE;
                }
                
#ifdef USE_setOfStates
                state.flip(var); /* REVERSE FLIP _B*/
#endif
            }
            numCandVar = i;
        }
    }


    if (random()%100 < walkProb) {	//=== Normal AdaptNovelty+
        clause = falseClause[random() % numFalse];
	// check each variable on this clause in turn. one must be unassigned
	int firstVarIdx = random() % size[clause];
	int offset = 0;
        bestVar = Var(clause,firstVarIdx+offset);
        while(inPrefix[abs(bestVar)] && (offset++ < size[clause])) {
		bestVar = Var(clause,((firstVarIdx+offset) % size[clause]));
	}
	//numWalks++;
    } else if (numCandVar > 0) {
        flags = FALSE;
        assert(DEBUG__choose_bestVar);
        assert(bestVar < numAtoms + 1);
        //numDBests++;
        //printf("oOo"); fflush(stdout);
    } else {
	
#ifdef WEIGHTING
        (this->*updateClauseWeights)();
        //dealWithClauseWeights();
#endif
        
// #ifdef USE_setOfStates
//         //uint oldSize = setOfStates.size();
//         setOfStates.insert(state);
// #endif
		
        bestValue = secondBestValue = BIG;
        bestVar = secondBestVar = NOVALUE;

        clause = falseClause[random() % numFalse];
        clsptr = resClause[clause];
        youngest = varLastChange[abs(*clsptr)];

        for (j = 0; j < size[clause]; j++) {
            var = abs(*clsptr);

	    if(inPrefix[var]){
		    clsptr++;
		    continue;
	    }

            //score = break[var] - make[var];
            //score = _num_getScore(var);
            score = _weight_getScore(var);
            lastChange = varLastChange[var];
			
            if (lastChange > youngest) {
                youngest = varLastChange[var];
            }

            //assert((score >= bestValue) || bestVar > 0);
                        
            if ( (score < bestValue) || ((score == bestValue) && (lastChange < varLastChange[bestVar])) ) {
                secondBestVar = bestVar;
                secondBestValue = bestValue;
				
                bestVar = var;
                bestValue = score;
            } else if ( (score < secondBestValue) || ((score == secondBestValue) && (lastChange < varLastChange[secondBestVar])) ) {
                secondBestVar = var;
                secondBestValue = score;
            }
			
            clsptr++;
        }

        /*
          if ( (varLastChange[bestVar] == youngest) && (random()%100 < noise) ) {
          bestVar = secondBestVar;
          //numSBests++;
          }
        */
		
        if (bestVar >= 0 && varLastChange[bestVar] == youngest) {
            if ( (_num_getScore(bestVar) == 0) || (random()%100 < noise) ) {
                bestVar = secondBestVar;
                //numSBests++;
            }
        }
    }

    if(!inPrefix[abs(bestVar)])return bestVar;
    else{
	clause = falseClause[random() % numFalse];
	// check each variable on this clause in turn. one must be unassigned
	int firstVarIdx = random() % size[clause];
	int offset = 0;
        bestVar = Var(clause,firstVarIdx+offset);
        while(inPrefix[abs(bestVar)] && (offset++ < size[clause])) {
		bestVar = Var(clause,((firstVarIdx+offset) % size[clause]));
	}
    }
    // this should never happen!
    return NOVALUE;
}

void SearchInstance::updateClauseWeights_NULL() {
}

void SearchInstance::updateClauseWeights_Linear() {
    register int c, i, j, var;

    for (i = 0; i < numFalse; i++) {
        c = falseClause[i];

        if (++clauseWeight[c] == 2) {
            //=== Add clause to weighted list
            whereWeight[c] = numWeight;
            weightedClause[numWeight] = c;
            ++numWeight;
        }
        for (j = 0; j < size[c]; j++) {
            var = Var(c, j);
            _weight_adjScore(var, -1);
            if (!isCandVar[var] && (_weight_getScore(var) < 0) && (varLastChange[var] < flip-1)) {
                candVar[numCandVar++] = var;
                isCandVar[var] = TRUE;
            } 
        }
    }
	
#ifdef DETERMINISTIC_SMOOTHING
    if(++weightCounter > smoothProb) {
        smooth();
        weightCounter = 0;
    }
#else
    if (random()%1000 < smoothProb*10) smooth();
#endif
	
}

void SearchInstance::updateClauseWeights_Exp() {
    register int c, i, j, var;

    for (i = 0; i < numFalse; i++) {
        c = falseClause[i];

        stepSize[c] = stepSize[c] * exp(- 1.0 * pseudoStepSize *  trace[c]);
        trace[c] = traceDiscount * trace[c] + _EXP(stepSize[c]);
		
        clauseWeight[c] += stepSize[c];
		
        //		fprintf(stderr, "  oOo %d: %E -- %E -- %E oOo  ", c, stepSize[c], trace[c], clauseWeight[c]);
		
        //for (j = 0; j < size[c]; j++) _weight_adjScore(abs(resClause[c][j]), -stepSize[c]);
		
        for (j = 0; j < size[c]; j++) {
            var = Var(c, j);
            _weight_adjScore(var, -stepSize[c]);
            if (!isCandVar[var] && (_weight_getScore(var) < 0.0) && (varLastChange[var] < flip-1)) {
                candVar[numCandVar++] = var;
                isCandVar[var] = TRUE;
            } 
        }
    }
}

void SearchInstance::smooth() {
    register int c, i, j;
		 
    int startNumWeight = numWeight;

    for (i = 0; i < startNumWeight; i++) {
        c = weightedClause[i];
        if (--clauseWeight[c] == 1) {
            --numWeight;
            weightedClause[whereWeight[c]] = weightedClause[numWeight];
            whereWeight[weightedClause[numWeight]] = whereWeight[c];
        }
        if (numTrueLit[c] == 0) {
            for (j = 0; j < size[c]; j++) _weight_adjScore(Var(c,j), 1);
        } else if (numTrueLit[c] == 1) _weight_adjScore(critVar[c], -1);
    }
    ++smoothStages;
}



bool SearchInstance::saturation() {
#ifndef USE_setOfStates
	return false;
#else 
    return setOfStates.is_saturated();
#endif
}

void SearchInstance::adaptNoveltyNoise() {
    if ( (flip - lastAdaptFlip) > (numClauses / invTheta) ) {
        noise += (int) ((100 - noise) / invPhi);
        lastAdaptFlip = flip;
        lastAdaptNumFalse = numFalse;
    } else if (numFalse < lastAdaptNumFalse) {
        noise -= (int) (noise / invPhi / 2);
        lastAdaptFlip = flip;
        lastAdaptNumFalse = numFalse;
    }
}

void SearchInstance::init() {
    register int i, j, lit;
    
//         clauseWeight = new CWTYPE[numClauses];
//         weightDiff = new CWTYPE[numAtoms+1];
//         trace = new CWTYPE[numClauses];
//         stepSize = new CWTYPE[numClauses];
//         weightedClause = new int[numClauses];
//         whereWeight = new int[numClauses];
//         varLastChange = new int[numAtoms+1];
//         candVar = new int[numAtoms+1];
//         isCandVar = new int[numAtoms+1];
//         falseClause = new int[numClauses];
//         whereFalse = new int[numClauses];
//         numDiff = new int[numAtoms+1];
//         numMake = new int[numAtoms+1];
//         numBreak = new int[numAtoms+1];
//         numTrueLit = new int[numClauses];
//         critVar = new int[numClauses];

    
    /* Charles-to-Nghia:
     * BEGIN =-- Initialisation of the
     * state and the
     * partialState.*/
    VERBOSER(1, "About to make a state with :: "<<numAtoms+1<<" atoms."<<endl);
    //state = State(numAtoms + 1);

#ifdef USE_partialState 
    vector<uint> includedInPartialState;
    for(uint i = 0; i < numAtoms + 1; i++) if (random() % 2) includedInPartialState.push_back(i);
    partialState.operator=(PartialState(includedInPartialState));
#endif
        
    /* Charles-to-Nghia:
     * END =-- Initialisation of the
     * state and the
     * partialState.*/
        


    
    for (i = 0; i < numClauses; i++) {
        VERBOSER(1, "Setting the number of true literals for clause "<<i<<" to zero."<<endl);
        assert(0 != numTrueLit);
        numTrueLit[i] = 0;
        clauseWeight[i] = 1;
		
        trace[i] = 0.0;
        stepSize[i] = pseudoStepSize;
    }

    numFalse = 0;
    numWeight = 0;
    nullFlip = 0;
    smoothStages = weightCounter = 0;

    for (i = 1; i < numAtoms+1; i++) {
        setScore(i, 0);
        varLastChange[i] = 0;
    }

    for (i = 1; i < numAtoms+1; i++) {
        

        if(random()%2){//atom[i]){
            state.flipOn(i);
#ifdef USE_partialState 
            partialState.flipOn__withTrackingTest(i);
#endif
        } else {
            state.flipOff(i);
#ifdef USE_partialState 
            partialState.flipOff__withTrackingTest(i);
#endif
        }

#ifdef USE_ATOM
        atom[i] = (state.isTrue(i))?1:0;
#endif
    }

    /* Charles-to-Nghia: We have
     * a new state, so we will
     * want to hash that.*/
#ifdef USE_setOfStates
    setOfStates.insert(state);
#endif

#ifdef USE_setOfPartialStates
    setOfPartialStates.insert(partialState);
#endif        
        
	
    
    //=== Initialize the gradient cost
    for (i = 0; i < numClauses; i++) {
        for (j = 0; j < size[i]; j++){
            lit = Lit(i,j);
            if ((lit > 0) == IS_TRUE(abs(lit))){//atom[abs(lit)]) {
                numTrueLit[i]++;
                critVar[i] = abs(lit);
            }
        }
        if (numTrueLit[i] == 0) {
            whereFalse[i] = numFalse;
            falseClause[numFalse] = i;
            numFalse++;
		 
            //=== these variables make clause i
            for (j = 0; j < size[i]; j++) {
                decScore(Var(i,j), i);
            }
        } else if (numTrueLit[i] == 1) {
            //=== this variable breaks clause i
            incScore(critVar[i], i);
        }
    }
    lowCost = numFalse;
	
    numCandVar = 0;
    for (i = 1; i < numAtoms+1; i++) {
        if (_weight_getScore(i) < 0) {
            candVar[numCandVar++] = i;
            isCandVar[i] = TRUE;
        } else {
            isCandVar[i] = FALSE;
        }
    }

    // Adapt Novelty+ Noise
    if (adaptFlag) {
        lastAdaptFlip = flip;
        lastAdaptNumFalse = numFalse;
        noise = 0;
    }
}

void SearchInstance::flipAtom(int toflip) {
    register int i, j, c, var;
    int toEnforce;		//=== literal to enforce
    int numocc;
    int *occptr, *ptr;
    
    if (toflip == NOVALUE) {
        totalNullFlips++;
        nullFlip++;
        return;
    }
	
    //	CWTYPE oldScore = getScore(toflip);

    if (IS_TRUE(toflip)) toEnforce = -toflip;
    else toEnforce = toflip;

    //=== Flip the variable here.
#ifdef USE_ATOM
    atom[toflip] = 1-atom[toflip];
#endif

    /* Charles-to-Nghia: Truth
     * value of proposition
     * \argument{toFlip} has
     * changed. I now update
     * both the state and the
     * partial state to reflect
     * this.*/
    state.flip(toflip);
    
#ifdef USE_setOfStates
        //uint oldSize = setOfStates.size();
        setOfStates.insert(state);
#endif
    
#ifdef USE_partialState
    partialState.flip__withTrackingTest(toflip);
#endif

        
    /* Charles-to-Nghia: Keep
     * track of the states (and
     * partial states) we have
     * already visited, by
     * storing them in a
     * hash.*/
#ifdef USE_setOfStates
    ;
    //setOfStates.insert(state);
#endif
        
#ifdef USE_setOfPartialStates
    setOfPartialStates.insert(partialState);
#endif
        
	
    for (ptr = neighbourVar[toflip]; *ptr != NOVALUE; ptr++) {
        i = *ptr;	
        if (_weight_getScore(i) >= 0) isCandVar[i] = FALSE;
        else isCandVar[i] = TRUE;
    }
	
    //=== Examine all occurrences of literals with toEnforce with old sign.
    //=== (these literals are made false; numTrueLit must be decremented in that clause)
    numocc = numOccurence[MAXATOM-toEnforce];
    occptr = occurence[MAXATOM-toEnforce];
    for (i = 0; i < numocc ;i++) {
        c = *(occptr++);
        if (--numTrueLit[c] == 0) {
            // clause c is no longer satisfied by lit with toflip; it's now false.
            falseClause[numFalse] = c;
            whereFalse[c] = numFalse++;
			
            //=== Decrement toflip's break.
            decScore(toflip, c);
            //=== Increment make of all vars in clause c.
            for (j = 0; j < size[c]; j++) {
                decScore(Var(c,j), c);
                //if (Var(c,j) == toflip) printf("DEC %d== ", c);
                //fflush(stdout);
            }
        } else if (numTrueLit[c] == 1) {
            //=== Find the lit in this clause that makes it true, and inc its break.
            for (j = 0; j < size[c]; j++) {
                var = Var(c,j);
                if ((Lit(c,j) > 0) == IS_TRUE(var)/*atom[var]*/) {
                    critVar[c] = var;
                    incScore(var, c);
                    break;
                }
            }
        }
    }

    //=== Examines all occurrences of literal with toEnforce with new sign 
    //=== (these literals are made true; numTrueLit must be incremented in that clause)
    numocc = numOccurence[MAXATOM+toEnforce];
    occptr = occurence[MAXATOM+toEnforce];
    for (i = 0; i < numocc; i++) {
        c = *(occptr++);
        if (++numTrueLit[c] == 1){
            falseClause[whereFalse[c]] = falseClause[--numFalse];
            whereFalse[falseClause[numFalse]] = whereFalse[c];

            critVar[c] = toflip;
            incScore(toflip, c);
            //=== Decrement the make of all vars in clause c.
            for (j = 0; j < size[c]; j++) {
                incScore(Var(c,j), c);
                //if (Var(c,j) == toflip) printf("ASC %d== ", c);
                //fflush(stdout);
            }
        } else if (numTrueLit[c] == 2){
            //=== Find the lit in this clause other than toflip that makes it true,
            //=== and decrement its break.
            for (j=0; j<size[c]; j++){
                var = Var(c,j);
                if( ((Lit(c,j) > 0) == /*atom[var]*/ IS_TRUE(var)) && (toflip != var) ) {
                    decScore(var, c);/* There is a
                                      * change to make
                                      * - break as
                                      * break is
                                      * decremented*/
                    break;
                }
            }
        }
    }
	
    /*
      if (oldScore != 0) {
      assert(oldScore != getScore(toflip));
      } else {
      printf("Zero cost move\n");
      fflush(stdout);
      }
    */
    
    for (ptr = neighbourVar[toflip]; *ptr != NOVALUE; ptr++) {
        i = *ptr;	
        if (_weight_getScore(i) < 0) {
            if (!isCandVar[i]) {
                assert(numCandVar < numAtoms + 1);
                            
                candVar[numCandVar++] = i;
                isCandVar[i] = TRUE;
            }
        }
    }
}


void parseParameters(int argc,char *argv[]) {
    int i;
	
    numTries = DEFAULT_NUMTRIES;
    cutoff = DEFAULT_CUTOFF_FLIPS;
    targetCost = DEFAULT_TARGETCOST;
    printSol = DEFAULT_PRINTSOL;
	
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i],"-seed") == 0) scanInt(argc,argv,++i,&seed);
        else if (strcmp(argv[i],"-cutoff") == 0) scanInt(argc,argv,++i,&cutoff);
        else if (strcmp(argv[i],"-tries") == 0) scanInt(argc,argv,++i,&numTries);
        else if (strcmp(argv[i],"-targetcost") == 0) scanInt(argc,argv,++i,&targetCost);
        else if (strcmp(argv[i],"-noise") == 0) scanInt(argc,argv,++i,&GLOBAL__noise);
        else if (strcmp(argv[i],"-walkprob") == 0) scanInt(argc,argv,++i,&GLOBAL__walkProb);
        else if (strcmp(argv[i],"-smoothprob") == 0) scanInt(argc,argv,++i,&GLOBAL__smoothProb);
        else if (strcmp(argv[i],"-printsol") == 0) printSol = TRUE;
        else {
            if (strcmp(argv[i],"-help")!=0 && strcmp(argv[i],"-h")!=0 )
                fprintf(stderr, "\nBad argument %s\n\n", argv[i]);
            fprintf(stderr, "General parameters:\n");
            fprintf(stderr, "  -seed N (integer, default: quasi-random based on system time)\n");
            fprintf(stderr, "  -cutoff N (integer, default: %d)\n", cutoff);
            fprintf(stderr, "  -tries N (integer, default: %d)\n", numTries);
            fprintf(stderr, "  -noise N (percent, default: %d)\n", GLOBAL__noise);
            fprintf(stderr, "  -walkprob N (percent, default: %d)\n", GLOBAL__walkProb);
            fprintf(stderr, "  -smoothprob N (percent, default: %d)\n", GLOBAL__smoothProb);
            fprintf(stderr, "  -targetcost N = find assignments of cost <= N (MAXSAT) (integer, default: %d)\n", targetCost);
            fprintf(stderr, "  -printsol\n");
            fprintf(stderr, "\nUsage:\n");
            fprintf(stderr, "  sls -parameters < instance.cnf\n\n");
            exit(-1);
        }
    }
}

void parseParametersComp(int argc,char *argv[]) {
    numTries = DEFAULT_NUMTRIES;
    cutoff = DEFAULT_CUTOFF_FLIPS;
    targetCost = DEFAULT_TARGETCOST;
    printSol = DEFAULT_PRINTSOL;
    numThreads = DEFAULT_THREADS;
    
    char c;
    char* fname = NULL;
    if(argc == 2) fname = argv[1];
    while((c = getopt(argc, argv, "d:s:f:r:a:")) != -1){
      switch(c){
      case 'd': // decision interval
	decisionInterval = atoi(optarg);
	break;
      case 's': // suggestions size
	suggestionSize = atoi(optarg);
	break;
      case 'f': // file name (DIMACS CNF)
	fname = optarg;
	break;
      case 'r': // NOT USED BY LOCAL SEARCH
	break;
      case 'a': // How to advise CDCL
	char* advise_schema = optarg;
	if ( 0 == strcmp(advise_schema, "ghosts") ){
	  using__ghost_suggestions = true;
	} else {
	  using__ghost_suggestions = false;
	}
      }
    }	

    if(fname == NULL){
       	fname = argv[argc-1];
    }

    fpInput = fopen(fname, "r");
    if (fpInput == NULL) {
        fprintf(stderr, "ERROR - Cannot open input file\n");
        exit(-1);
    }
	
    /*if (argc > 2) scanInt(argc, argv, 2, &seed);
    
    if (argc > 3) scanInt(argc, argv, 3, &numThreads);*/
}

void scanInt(int argc, char *argv[], int i, int *varptr) {
    if (i>=argc || sscanf(argv[i],"%i",varptr)!=1){
        fprintf(stderr, "Bad argument %s\n", i<argc ? argv[i] : argv[argc-1]);
        exit(-1);
    }
}


void scanFloat(int argc, char *argv[], int i, float *varptr) {
    if (i>=argc || sscanf(argv[i],"%f",varptr)!=1){
        fprintf(stderr, "Bad argument %s\n", i<argc ? argv[i] : argv[argc-1]);
        exit(-1);
    }
}

int compareAbsInts(const void *a, const void *b) {
    int *va = (int *) a;
    int *vb = (int *) b;
    return ( abs(*va) - abs(*vb) );
}


void initInstance() {
    int i, j, lit, freestore, lastc, nextc;
    int *storeptr = 0;
    char buf[100];
    int *neighbours;
    int numNeighbours;
    int numocc, c, var, k;
    int *occptr, *ptr;

    lastc = '#';
    while (lastc != 'p') {
        while ((lastc = fgetc(fpInput)) == 'c') {
            while ((nextc = fgetc(fpInput)) != EOF && nextc != '\n');
        }
    }

    ungetc(lastc, fpInput);
    if (fscanf(fpInput, "p %s %i %i", buf, &numAtoms, &numClauses) != 3) {
        fprintf(stderr,"ERROR - Bad input file!\n");
        exit(-1);
    }
    if (strcmp("cnf", buf) != 0 ) {
        fprintf(stderr, "ERROR - Bad input file\n");
        exit(-1);
    }
    if (numAtoms > MAXATOM) {
        fprintf(stderr,"ERROR - too many atoms\n");
        exit(-1);
    }

	neighbours = new int[numAtoms+1];
	
    freestore = 0;
    numLiterals = 0;
    for (i = 0; i < MAXLITERAL+1; i++) numOccurence[i] = 0;

    minClauseSize = maxClauseSize = NOVALUE;
    for (i = 0; i < numClauses; i++) {
        size[i] = -1;
        if (freestore < MAXLENGTH) {
            storeptr = new int[STOREBLOCK];//(int *) malloc( sizeof(int) * STOREBLOCK );
            freestore = STOREBLOCK;
        }
        resClause[i] = storeptr;
        //		cost[i] = 1;

        do {
            size[i]++;
            if (size[i] > MAXLENGTH) {
                fprintf(stderr, "ERROR - clause too long\n");
                exit(-1);
            }
            if (fscanf(fpInput, "%i ", &lit) != 1) {
                fprintf(stderr, "ERROR - Bad input file\n");
                exit(-1);
            }
            if (lit != 0) {
                *(storeptr++) = lit;
                freestore--;
                numLiterals++;
                numOccurence[lit+MAXATOM]++;
            }
        } while (lit != 0);
        qsort(resClause[i], size[i], sizeof(int), compareAbsInts);
        if ( (size[i] < minClauseSize) || (minClauseSize == NOVALUE) ) minClauseSize = size[i];
        if ( (size[i] > maxClauseSize) || (maxClauseSize == NOVALUE) ) maxClauseSize = size[i];
    }
    fclose(fpInput);
    
    if (size[0] == 0) {
        fprintf(stderr,"ERROR - incorrect problem format or extraneous characters\n");
        exit(-1);
    }

    for (i = 0; i < MAXLITERAL+1; i++) {
        if (freestore < numOccurence[i]) {
            storeptr = new int[STOREBLOCK];//(int *) malloc( sizeof(int) * STOREBLOCK );
            freestore = STOREBLOCK;
        }
        occurence[i] = storeptr;
        freestore -= numOccurence[i];
        storeptr += numOccurence[i];
        numOccurence[i] = 0;
    }

    for (i = 0; i < numClauses; i++) {
        for (j = 0; j < size[i]; j++) {
            occurence[Lit(i,j)+MAXATOM][numOccurence[Lit(i,j)+MAXATOM]] = i;
            numOccurence[Lit(i,j)+MAXATOM]++;
        }
    }
	
    for (i = 1; i < numAtoms+1; i++) neighbours[i] = 0;

    for (i = 1; i < numAtoms+1; i++) {
        numNeighbours = 0;

        numocc = numOccurence[MAXATOM-i];
        occptr = occurence[MAXATOM-i];
        for (j = 0; j < numocc ; j++) {
            c = *(occptr++);
            for (k = 0, ptr = resClause[c]; k < size[c]; k++, ptr++) {
                var = abs(*ptr);
                if ((neighbours[var] != i) && (var != i)) {
                    neighbours[var] = i;
                    numNeighbours++;
                }
            }
        }

        numocc = numOccurence[MAXATOM+i];
        occptr = occurence[MAXATOM+i];
        for (j = 0; j < numocc ; j++) {
            c = *(occptr++);
            for (k = 0, ptr = resClause[c]; k < size[c]; k++, ptr++) {
                var = abs(*ptr);
                if ((neighbours[var] != i) && (var != i)) {
                    neighbours[var] = i;
                    numNeighbours++;
                }
            }
        }
		
        if (freestore < ++numNeighbours) {
            storeptr = new int[STOREBLOCK];//(int *) malloc( sizeof(int) * STOREBLOCK );
            freestore = STOREBLOCK;
        }
        neighbourVar[i] = storeptr;
        freestore -= numNeighbours;
		
        for (j = 1; j < numAtoms+1; j++) {
            if (neighbours[j] == i) {
                *(storeptr++) = j;
                if (--numNeighbours == 1) {
                    *(storeptr++) = NOVALUE;
                    break;
                }
            }
        }
    }
	delete[] neighbours;
}

void printHeader() {
    printf("c +------------------------------+\n");
    printf("c | gNovelty+, version 1.0       |\n");
    printf("c +------------------------------+\n");
    printf("c gnovelty+ -h for help\nc \n");
}


void SearchInstance::printStatsHeader() {
//     printf("c Problem instance: numAtoms = %i, numClauses = %i, numLiterals = %i\n",numAtoms, numClauses, numLiterals);
//     printf("c General parameters: seed=%d, cutoff=%d, targetCost = %d, print solution: ", seed, cutoff, targetCost);
//     printf( printSol?"yes":"no");
//     printf("\nc gNovelty+ parameters: noise = %d, walkprob = %d, smoothprob = %d", noise, walkProb, smoothProb);
//     printf("\nc tries = %d\nc \n",numTries);

    //printf("  best    flips     final     total     total\n");
    //printf("  cost    until    #unsat     flips   seconds\n");
    //printf("  this     best      this      this      this\n");
    //printf("   try     soln       try      try        try\n");
    fflush(stdout);
}


void SearchInstance::printStatsEndTry() {
    printf("%6d %8d %9d %9ld %9.2lf", lowCost, lowFlips, numFalse, flip - nullFlip, trytime);
    printf("\n");
    fflush(stdout);
}


/* FIX: There is a bug at the moment causing the unwanted condition
 * "computeCurrentCost() != numFalse" to become true*/
void SearchInstance::updateStatsEndTry() {
    totalFlips += flip;

    if (numFalse <= targetCost) {
        while(pthread_mutex_trylock (&solution_mutex) == EBUSY)sleep(1);
    
        saveSolution();
        numSuccessTries++;
        totalSuccessFlips += flip;
        totalSuccessNullFlips += nullFlip;
        pthread_mutex_unlock (&solution_mutex);
    }
    if (computeCurrentCost() != numFalse) {
        fprintf(stderr, "Program error, verification of assignment cost fails!\n");
        fprintf(stderr, "Program error :: Currest cost is %d, and numFalse is %d \n", computeCurrentCost(), numFalse);
        exit(-1);
    }
}


void SearchInstance::updateStatsEndFlip() {
    if (numFalse < lowCost) {
        lowCost = numFalse;
        lowFlips = flip - nullFlip;
		
        //fprintf(stderr, "New best %5d at flip# %10d\n", lowCost, flip); 
    }
}

void SearchInstance::printFinalStats(void) {
    printf("\ntotal elapsed seconds = %f\n", expertime);
    printf("average flips per second = %ld\n", (long)((totalFlips - totalNullFlips)/expertime));
    printf("number solutions found = %d\n", numSuccessTries);
    printf("final success rate = %f\n", ((double)numSuccessTries * 100.0)/numTries);
    printf("average length successful tries including nulls = %li\n",
           numSuccessTries ? (totalSuccessFlips/numSuccessTries) : 0);
    if (numSuccessTries > 0) {
        printf("mean flips for successful tries = %.6f\n",
               numSuccessTries ? ((double) (totalSuccessFlips - totalSuccessNullFlips)/numSuccessTries) : 0);
    }

    if (numSuccessTries > 0) {
        printf("ASSIGNMENT FOUND\n");
        //if (printSol == TRUE) printSolution();
    } else printf("ASSIGNMENT NOT FOUND\n");
}

void SearchInstance::printFinalStatsComp(int rank) {
    if (numSuccessTries > 0) {
        printf("gnovelty -- s SATISFIABLE (found by proc %d with prefix length %d)\n", rank, prefixLength);
        //if (printSol == TRUE) printSolution();
        printf("gnovelty -- c Done in %f seconds\n", expertime);
    } else {
        printf("gnovelty -- s UNKNOWN\n");
    }
}

CWTYPE SearchInstance::computeCurrentCost() {
    int i, j, sat;
    CWTYPE result = 0;

    for (i = 0; i < numClauses; i++) {
        sat = FALSE;
        for (j = 0; j < size[i]; j++) {
            if ( (Lit(i,j) > 0) == IS_TRUE(Var(i, j))/*atom[Var(i,j)] */){
                sat = TRUE;
                break;
            }
        }
        //if (!sat) result += cost[i];
        if (!sat) result++;
    }
    return result;
}


void SearchInstance::saveSolution(void) {
    register int i;

    for (i = 1; i < numAtoms+1; i++) 
        solution[i] = (IS_TRUE(i))?1:0;//atom[i];
	
    //saveUnitVariableValue();
}


void printSolution(void) {
	register int i;
	
	printf("gnovelty -- v ");
	// bug fixed: should be numAtoms+1
	for (i = 1; i < numAtoms+1; i++) 
		printf("%i ", solution[i] == 1 ? i : -i);
	printf("0\n");
	fflush(stdout);
}

// void printSolution(void) {
//     register int i;
	
//     for (i = 1; i < numAtoms+1; i++) 
//         printf("v %i\n", solution[i] == 1 ? i : -i);
//     printf("v 0\n");
//     fflush(stdout);
// }


