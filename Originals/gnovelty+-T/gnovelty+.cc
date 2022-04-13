/**********************************************************/
/* gNovelty+, version 1.0                                 */
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


int minClauseSize, maxClauseSize;
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

    //printf("seed = %d\n", seed);

    lowCost = BIG;
    for (Nghia_tryAgain = 1; Nghia_tryAgain <= numTries; Nghia_tryAgain ++) {
        std::cerr<<"Into the main loop. "<<std::endl;
        
        starttime = elapsedTime();
        searchInstance.flip = searchInstance.nullFlip = 0;

        
        // required for the stepsize weighting
        searchInstance.pseudoStepSize = pow(10.0, -15);
        searchInstance.traceDiscount = 0.99;//static_cast<CWTYPE>(0.99);

        searchInstance.init();
		
        //numWalks = numDBests = numFreqs = numSBests = 0;

        
        //while ((numFalse > targetCost) && (flip - nullFlip < cutoff)) {
        while (searchInstance.numFalse > targetCost) {
            searchInstance.flip++;
            //if(!(searchInstance.flip % 100000)) 	fprintf(stderr, "%d ", searchInstance.flip);
                        
            searchInstance.var = searchInstance.pickVar();
            //	fprintf(stderr, " -- %d, flip %d --", searchInstance.flip, searchInstance.var);
            //	fprintf(stderr, " -- flip %d --", var);

            //assert(var != NOVALUE);
			
            searchInstance.flipAtom(searchInstance.var);
			
            searchInstance.varLastChange[searchInstance.var] = searchInstance.flip;
				
            if (searchInstance.adaptFlag && searchInstance.flags){
                
                searchInstance.adaptNoveltyNoise();
            }
            
			
            searchInstance.updateStatsEndFlip();
            
            //if(searchInstance.saturation()){
            //    std::cerr<<"Breaking due to saturation. "<<std::endl;
            //    break;
            //}
        }
        
        searchInstance.trytime = elapsedTime() - starttime;
        searchInstance.updateStatsEndTry();
        //printStatsEndTry();

        
        std::cerr<<"Exiting main loop. "<<std::endl;
        
        //printf("rand: %10d, dec: %10d, freq: %10d, fb: %10d, sb: %10d\n", 
        //	numWalks, numDBests, numFreqs, (flip-numWalks-numDBests-numFreqs-numSBests), numSBests);
    }
    expertime = elapsedTime();
    //printFinalStats();
    searchInstance.printFinalStatsComp();

//     searchInstance.saveSolution();
    printSolution();

    //std::cerr<<"Sleeping supposedely. "<<std::endl;
    //while(true)sleep(100);
    exit(10);
    
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
int numThreads;// = DEFAULT_THREADS;

/************************************/
/* Main                             */
/************************************/
int main(int argc,char *argv[]) {

    
    pthread_mutex_init (&solution_mutex, NULL);
    
    VERBOSER(1, "Printing headder.\n");
    
    printHeader();

    seed = genRandomSeed();
    //parseParameters(argc, argv);
    parseParametersComp(argc, argv);
    srandom(seed);
	
    VERBOSER(1, "Done generation of random seeds.\n");
    
    //processInstance();
    /*PARSING HAPPENS HERE...*/
    initInstance();

    uint CONSTANT = 100;
    
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
   
	
    return 0;
}

//int best[MAXATOM];
//int numBest;

int SearchInstance::pickVar(void) {
    bool DEBUG__choose_bestVar = false;
    
    CWTYPE score, bestValue, secondBestValue;
    int bestVar, secondBestVar, youngest, lastChange;
    register int i, j, clause, var;
    int *clsptr;

        
    flags = TRUE;
    if (numCandVar > 0) {	//=== greedy step
        bestValue = BIG;  //CWBIG;	//BIG;
        lastChange = flip;

        i = NOVALUE;
        clsptr = candVar;
        for (j = 0; j < numCandVar; j++, clsptr++) {
            var = *clsptr;

#ifdef USE_setOfStates
            state.flip(var); /* FLIP _A*/
#endif
            
            score = _weight_getScore(var); 
            if (score < 0

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
                if (score < 0 
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
        bestVar = Var(clause,(random() % size[clause]));
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
		
        if (varLastChange[bestVar] == youngest) {
            if ( (_num_getScore(bestVar) == 0) || (random()%100 < noise) ) {
                bestVar = secondBestVar;
                //numSBests++;
            }
        }
    }

    return bestVar;
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
    //return setOfStates.is_saturated();
    return false;
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
    
    fpInput = fopen(argv[1], "r");
    if (fpInput == NULL) {
        fprintf(stderr, "ERROR - Cannot open input file\n");
        exit(-1);
    }
	
    if (argc > 2) scanInt(argc, argv, 2, &seed);
    
    if (argc > 3) scanInt(argc, argv, 3, &numThreads);
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
        if (printSol == TRUE) printSolution();
    } else printf("ASSIGNMENT NOT FOUND\n");
}

void SearchInstance::printFinalStatsComp() {
    if (numSuccessTries > 0) {
        printf("s SATISFIABLE\n");
        //if (printSol == TRUE) printSolution();
        printf("c Done in %f seconds\n", expertime);
//         exit(10);
    } else {
        printf("s UNKNOWN\n");
//         exit(0);
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
	
	printf(""); // removed by c. gretton for 'spin' script compatibility
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


