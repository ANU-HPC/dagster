/**********************************************************/
/* global.h, version 1.0                                  */
/*                                                        */
/* Duc Nghia Pham (d.n.pham@griffith.edu.au)              */ 
/*   IIIS, Griffith University, Australia                 */
/* February 2005                                          */
/**********************************************************/

#include <iostream>
//#include <cmalloc>
/*---------------------------------

  Old fashioned debugging

  ---------------------------------*/
#define DEBUG_LEVEL 0
#define DEBUG_GET_CHAR(Y) {if(Y > DEBUG_LEVEL) {char ch; cin>>ch;} }

#define VERBOSE(X) {cerr<<"INFO :: "<<X<<endl;}
#define VERBOSER(Y, X) {if(Y > DEBUG_LEVEL)cerr<<"INFO ("<<Y<<") :: "<<X;}

/*---------------------------------

  Macros for fatal and lower-level errors.

  ---------------------------------*/
#define UNRECOVERABLE_ERROR(X) {cerr<<"UNRECOVERABLE ERROR :: "<<X;assert(0);exit(0);}

#define QUERY_UNRECOVERABLE_ERROR(Q,X) {if(Q)UNRECOVERABLE_ERROR(X);}

#define WARNING(X) {}//{cerr<<"WARNING :: "<<X<<endl;}


#ifndef __MY_GLOBAL_H__
#define __MY_GLOBAL_H__

//=== If the constant NT is set the program compiles under Windows NT.
//=== Currently, the timing then reflects user time.
#ifdef WIN32
	#define NT 1
#endif

/************************************/
/* Standard includes                */
/************************************/
#include <cstdio>
#include <cstring>
#include <cstdlib>
//#include <math.h>

#ifdef WIN32
	#define random() rand()
	#define srandom(seed) srand(seed)
#endif

/**********************************************************/
/* DLS specific variables                                 */
/**********************************************************/
#define CWTYPE int
//#define CWTYPE double

#define PROBABILITY int

/***********************************************************/
/* Wrappers                                                */
/***********************************************************/
#define Lit(CLAUSE, POSITION) (resClause[CLAUSE][POSITION])
#define Var(CLAUSE, POSITION) (abs(Lit(CLAUSE,POSITION)))

//static int scratch;
//#define ABS(x) ((scratch=(x))>0?(scratch):(-scratch))

/**********************************************************/
/* Constants                                              */
/**********************************************************/
#define MAXATOM 5000000			// maximum possible number of atoms
#define MAXCLAUSE 50000000		// maximum possible number of clauses
#define MAXLITERAL 2*MAXATOM		// maximum possible number of literals
#define MAXLENGTH 5000			// max num of literals which can be in a clause
#define STOREBLOCK 2000000		// size of block to malloc each time

#define CWBIG 1E30f			// quasi-infinity for CWTYPE
#define BIG 100000000			// quasi-infinity for integer

#define TRUE 1
#define FALSE 0
#define NOVALUE -1

#define DEFAULT_NUMTRIES 1		// default number of tries
#define DEFAULT_CUTOFF_FLIPS 100000	// default maximal number of flips per try
#define DEFAULT_TARGETCOST 0		// default cost to reach in order to have a solution
#define DEFAULT_PRINTSOL TRUE		// default flag determining whether to output solution

/**********************************************************/
/* Basic variables and data structures                    */
/**********************************************************/
int numAtoms;				// number of atoms in input formula
int numClauses;			// number of clauses in input formula
int numOrigClauses;		// number of clauses in the original input formula
int numLiterals;			// number of literals in input formula


int *resClause[MAXCLAUSE];		// indexed as resClause[clause_num][literal_num]
int size[MAXCLAUSE];			// length of each clause

#include"State.hh"

#include"PartialState.hh"

PROBABILITY GLOBAL__noise = 50;
int GLOBAL__smoothProb =  100;
PROBABILITY GLOBAL__walkProb = 1;	// default random walk probability


class SearchInstance
{
public:
    
    //#define USE_setOfStates 1

   
/* Charles-to-Nghia: You will want to make a new hash for every subset
 * you are interesting in tracking. This is kind of obvious but I
 * wanted to check that you were going to do this. We do not want to
 * have to appeal to the mapping from propositions to elements in the
 * bitvector while we are hashing, otherwise I believe things will
 * become too slow. Incidentally, for the moment this is tracking
 * states from an arbitrary set of atoms chosen in the call to
 * \function{initInstance}.*/
//SetOfStates setOfPartialStates;

 
/* Charles-to-Nghia: Comment this out if you don't want to hash partial states.*/
//#define USE_setOfPartialStates 1



    SearchInstance(uint numAtoms,
                   uint numClauses,
                   uint juhan__hash_array_size = 1000000)
        :juhan__hash_array_size(juhan__hash_array_size),
         smoothProb(GLOBAL__smoothProb),
         updateClauseWeights(NULL),
         adaptFlag(1),
         noise(GLOBAL__noise),
         walkProb(GLOBAL__walkProb),
         state(numAtoms+1)
         //smoothProb(GLOBAL__smoothProb),
         //noise(GLOBAL__noise)
    {
#ifdef USE_setOfStates 
        setOfStates = SetOfStates(juhan__hash_array_size);
#endif
        
        clauseWeight = new CWTYPE[numClauses];
        weightDiff = new CWTYPE[numAtoms+1];
        trace = new CWTYPE[numClauses];
        stepSize = new CWTYPE[numClauses];
        weightedClause = new int[numClauses];
        whereWeight = new int[numClauses];
        varLastChange = new int[numAtoms+1];
        candVar = new int[numAtoms+1];
        isCandVar = new int[numAtoms+1];
        falseClause = new int[numClauses];
        whereFalse = new int[numClauses];
        numDiff = new int[numAtoms+1];
        numMake = new int[numAtoms+1];
        numBreak = new int[numAtoms+1];
        numTrueLit = new int[numClauses];
        critVar = new int[numClauses];
        prefix = new int[numAtoms];
	prefixLength = 0;
	nextPrefixLength = 0;
    }
    
    void (SearchInstance::*updateClauseWeights)(void);// = NULL;
    
    PROBABILITY walkProb;
    
    int var;
    
    long int flip;				// counter for flips
    long int nullFlip;			// counter for nullFlips

    int* prefix;
    int prefixLength;
    int nextPrefixLength;

    //=== Weighting controlled variables
    CWTYPE* clauseWeight;//[MAXCLAUSE];		// weight of a clause
    CWTYPE* weightDiff;//[MAXATOM+1];		// the total weight difference if a variable if flipped
    CWTYPE weightTotal;					// total clause weight

    CWTYPE pseudoStepSize;				// initialised in main()
    CWTYPE* trace;//[MAXCLAUSE];
    CWTYPE traceDiscount;
    CWTYPE* stepSize;//[MAXCLAUSE];

    int numWeight;						// number of clauses with weight > 1
    int* weightedClause;//[MAXCLAUSE];		// clauses which have weight > 1:
    int* whereWeight;//[MAXCLAUSE];		// same as for whereFalse:
    int /*smoothStages -- already defined in global,*/ weightCounter;

    int smoothProb;// = 100;

    //=== AdaptNovelty+ controlled variables
    int* varLastChange;//[MAXATOM+1];
    int lastAdaptFlip;
    int lastAdaptNumFalse;

    int adaptFlag;// = 1;

    State state;
#ifdef USE_setOfStates 
    SetOfStates setOfStates;
#endif
    
#ifdef USE_partialState 
    PartialState partialState;//(vector<uint>());
    SetOfStates setOfPartialStates;
#endif

    int* candVar;//[MAXATOM+1];
    int* isCandVar;//[MAXATOM+1];
    int numCandVar;
    
    int flags;
    int numWalks, numDBests, numFreqs, numSBests;
    
    PROBABILITY noise;// = 50;		// default noise (in percent)
    
    void printStatsHeader();
    void printStatsEndTry() ;
    void updateStatsEndTry() ;
    void updateStatsEndFlip() ;
    void printFinalStats(void) ;
    void printFinalStatsComp(int)  ;
    CWTYPE computeCurrentCost() ;
    void saveSolution(void) ;
    
    void init();
    int  pickVar();
    void flipAtom(int toflip);
    
    void updateClauseWeights_Linear();
    void updateClauseWeights_Exp();
    void updateClauseWeights_NULL();
    
    void smooth();
    void adaptNoveltyNoise();

    bool saturation();
    
    /**********************************************************/
    /*  Global Statistics                                     */
    /**********************************************************/
    long int totalFlips;			// total number of flips in all tries so far
    long int totalNullFlips;		// total num of nullFlips in all tries so far
    long int totalSuccessFlips;		// total num of flips in succesful tries so far
    long int totalSuccessNullFlips;		// total num of null flips in succesful tries so far
    int numSuccessTries;			// total number of successful tries so far
    int smoothStages;			// number of times the smoothing was performed
    

    int numFalse;			// number of false clauses
    int* falseClause;//[MAXCLAUSE];		// clauses that are false:
    int* whereFalse;//[MAXCLAUSE];		// where each clause is listed in falseClause:
					// whereFalse[c] = i <=> c is ith unsat. clause

    
    int* numDiff;//[MAXATOM+1];
    
    int* numMake;//[MAXATOM+1];			// number of clauses var makes when flipped.
    int* numBreak;//[MAXATOM+1];		// number of clauses var breaks when flipped.

    int* numTrueLit;//[MAXCLAUSE];		// number of true literals in each clause
    int* critVar;//[MAXCLAUSE];		// the literal for clauses critically sat
    
    /*Thread and thread attributes for the local search.*/
    pthread_t thread;
    pthread_attr_t attributes;
    
    uint juhan__hash_array_size;
    
    double trytime;
};


int *occurence[MAXLITERAL+1];		// where each literal occurs: indexed as
					// occurence[literal+MAXATOM][occurence_num]
int numOccurence[MAXLITERAL+1];		// number of times each literal occurs



int solution[MAXATOM+1];		// where the last solution gets saved
//int cost[MAXCLAUSE];			// cost associated with each clause
int costAllFalse;			// sum of costs associated w/ all false clauses
int lowCost;				// best costAllFalse seen so far
int lowFlips;				// number of flips it took to reach lowCost

FILE *fpInput;


/**********************************************************/
/* General parameters                                     */
/**********************************************************/
int seed;				// seed used for randomization
int numTries;				// number of tries
int cutoff;				// maximal number of flips per try
int targetCost;				// cost to reach in order to have a solution
int printSol;				// flag determining whether to output solution

/**********************************************************/
/* Timing                                                 */
/**********************************************************/
double expertime;

#endif

