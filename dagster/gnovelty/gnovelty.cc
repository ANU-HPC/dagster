/**********************************************************/
/* A gradient based Novelty+ for guiding a Conflict       */
/* Driven Clause Learning procedure. This module is       */
/* based on gNovelty+, version 1.0                        */
/*                                                        */
/*   Authors in chronological order                       */
/*                                                        */
/*      1. Charles Gretton (charles.gretton@anu.edu.au)   */ 
/*            Australian National University              */
/*                                                        */
/*      2. Josh Milthorpe (josh.milthorpe@anu.edu.au)     */ 
/*            Australian National University              */
/*                                                        */
/*      3. Tate Kennington (tatekennington@gmail.com)     */ 
/*            University of Dunedin, New Zealand          */
/*            ANU Summer Scholarship 2018/19              */
/*                                                        */
/*      4. Mark Burgess  (markburgess1989@gmail.com)      */ 
/*            Australian National University              */
/*            Research Assistant 2019/20                  */
/*                                                        */
/*                                                        */
/**********************************************************/
/* Based on -- gNovelty+, version 1.0                     */
/*                                                        */
/*      A greedy gradient based Novelty+                  */
/*                                                        */
/*      1. Duc Nghia Pham (duc-nghia.pham@nicta.com.au)   */ 
/*            SAFE Program,  National ICT Australia Ltd.  */
/*            IIIS, Griffith University, Australia        */
/*                                                        */
/*      2. Charles Gretton (charles.gretton@gmail.com)    */ 
/*            University of Birmingham                    */
/*            IIIS, Griffith University, Australia        */
/*                                                        */
/*                                                        */
/**********************************************************/
/* Part of gNovelty+ is based on UBCSAT version 1.0       */
/* written by:                                            */
/* Dave A.D. Tompkins & Holger H. Hoos                    */
/*   Univeristy of British Columbia                       */
/* February 2004                                          */
/**********************************************************/

/*************************
This file is part of Dagster.

Dagster is free software; you can redistribute it 
and/or modify it under the terms of the GNU General 
Public License as published by the Free Software 
Foundation; either version 2 of the License, or
(at your option) any later version.

Dagster is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR 
A PARTICULAR PURPOSE. See the GNU General Public 
License for more details.

You should have received a copy of the GNU General 
Public License along with Dagster.
If not, see <http://www.gnu.org/licenses/>.
*************************/

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <cerrno>

#include "gnovelty.hh"
#include "../utilities.h"
#include "../Cnf.h"
#include "../exceptions.h"

#define IS_TRUE(Z) (state->isTrue(Z))
#define Lit(CLAUSE, POSITION) (cnf->clauses[CLAUSE][POSITION])
#define Var(CLAUSE, POSITION) (abs(Lit(CLAUSE,POSITION)))
#define BIG 100000000 // quasi-infinity for integer
#define CWTYPE int
#define numAtoms (cnf->vc)

Gnovelty::Gnovelty(Cnf* new_Cnf, int noise, int _ghost__suggestion_buffer_size) {
  cnf = new_Cnf;
  cnf->compute_occurance_buffers();
  cnf->compute_variable_neighborhoods();

  this->noise = noise;
  this->_ghost__suggestion_buffer_size = _ghost__suggestion_buffer_size;
  this->solution_style = 0;
  flip = 0;
  _ghost__suggestion_index = 0;

  state = new State(cnf->vc+1);
  falseClauses = new ReversableIntegerMap(cnf->cc);
  candVar = new ReversableIntegerMap(cnf->vc+1);

  TEST_NOT_NULL(clauseWeight = (CWTYPE*)calloc(sizeof(CWTYPE),cnf->cc))
  TEST_NOT_NULL(solutionClauses = (bool*)calloc(sizeof(bool), cnf->cc))
  TEST_NOT_NULL(numTrueLit = (int*)calloc(sizeof(int),cnf->cc))
  TEST_NOT_NULL(numUnsetLit = (int*)calloc(sizeof(int),cnf->cc))
  TEST_NOT_NULL(variableScore = (CWTYPE*)calloc(sizeof(CWTYPE),cnf->vc+1))
  TEST_NOT_NULL(variableNumScore = (int*)calloc(sizeof(int),cnf->vc+1))
  TEST_NOT_NULL(varLastChange = (int*)calloc(sizeof(int),cnf->vc+1))
  TEST_NOT_NULL(inPrefix = (bool*)calloc(sizeof(bool),cnf->vc+1))
  TEST_NOT_NULL(isCandVar = (int*)calloc(sizeof(int),cnf->vc+1))
  TEST_NOT_NULL(_ghost__suggestions = (int*)calloc(sizeof(int),_ghost__suggestion_buffer_size))

  state->randomise();
  for (int i = 0; i < cnf->cc; i++)
    clauseWeight[i] = 1;
  setupScores();

  invPhi = 5;
  invTheta = 6;
  lastAdaptFlip = flip;
  lastAdaptNumFalse = falseClauses->map_size;
}

Gnovelty::Gnovelty(Cnf* new_Cnf, int noise, int _ghost__suggestion_buffer_size, int solution_style)
 : Gnovelty::Gnovelty(new_Cnf,noise,_ghost__suggestion_buffer_size) {
  this->solution_style = solution_style;
}

// set the weight of the variable,
inline void Gnovelty::adjScore(int var, int val) {
  variableScore[var] = (CWTYPE)(variableScore[var] + val);}
inline void Gnovelty::incScore(int var, int cls) {
  adjScore(var, clauseWeight[cls]); 
  variableNumScore[var]++;
}
inline void Gnovelty::decScore(int var, int cls) {
  adjScore(var, -clauseWeight[cls]); 
  variableNumScore[var]--;
}

// set the inPrefix array to true, and purge variable from candVar
inline void Gnovelty::set_inPrefix_TRUE(int var) {
  candVar->remove(var);// purge the variable is it is candVar
  inPrefix[var] = true;
}
// set the inPrefix array to false, and add it to candVar definition if score is less than zero
inline void Gnovelty::set_inPrefix_FALSE(int var) {
  candVar->append(var);
  inPrefix[var] = false;
}

// reset the noise level according to parameters
void Gnovelty::adaptNoveltyNoise() {
  if ( (flip - lastAdaptFlip) > (cnf->cc / invTheta) ) {
    noise += (int) ((100 - noise) / invPhi);
    lastAdaptFlip = flip;
    lastAdaptNumFalse = falseClauses->map_size;
  } else if (falseClauses->map_size < lastAdaptNumFalse) {
    noise -= (int) (noise / invPhi / 2);
    lastAdaptFlip = flip;
    lastAdaptNumFalse = falseClauses->map_size;
  }
}

// given the prefix array of integers (at prefixLength size), flip the atoms inside the gnovelty appropriately
int Gnovelty::loadPrefix(int* prefix, int prefixlength) {
  for(int i = 0; i < prefixlength && prefix[i] != 0; i++) { // HACK: parse the prefix to see if it is loadable
    if (abs(prefix[i]) >= candVar->size) { //if cannot be loaded
      return 1;
    }
  }
  for(int i = 1; i<=numAtoms; i++)
    set_inPrefix_FALSE(i);
  for(int i = 0; i < prefixlength && prefix[i] != 0; i++) { // parse in the prefix
    int var = prefix[i];
    set_inPrefix_TRUE(abs(var));
    if (state->isTrue(abs(var)) && var < 0) // If we are not already consistent with that literal, then make us consistent
      flipAtom(abs(var));
    else if (!state->isTrue(abs(var)) && var > 0) // As above for the opposite parity.
      flipAtom(abs(var));
    else if (prefix[i] == 0) // prematurely reached the end of the prefix
      break;
  }
  return 0;
}

// for a suggestion buffer (of size suggestion_size) load the variables in the order they were last selected into the buffer
void Gnovelty::loadSuggestion(int* suggestions, int suggestion_size) {
  bool* inList;
  TEST_NOT_NULL(inList = (bool*)calloc(sizeof(bool),numAtoms+1))
  for(int i = 0; i<suggestion_size; i++){ // For the number of suggestions I am required to send to the CDCL procedure.
    int curr = flip+1;
    int var = 0;
    for(int j = 1; j<=numAtoms; j++){ /*FORALL problem variables*/
      int lastChange = varLastChange[j]; // Get the iteration at which that variable was changed.
      if(lastChange < curr /*Is epoch when value was last flipped is earlier than current*/
         && !inList[j]     /*not already selected*/
         && !inPrefix[j]   /*assignment not forced by CDCL*/){
        var = j;
        curr = lastChange;
      }
    }
    // Set a suggestion
    if(var != 0)
      suggestions[i] = state->isTrue(var)?var:-var;
    else
      suggestions[i] = 0;
    inList[var] = true; /*flag as already selected*/
  }
  free(inList);
}

// for a suggestion buffer (of size suggestion_size) load the most recent flips into the buffer
// the ghost suggestions is a list of literals consistent with the last series of flips performed by the SLS.
void Gnovelty::loadSuggestion_ghost(int* suggestions, int suggestion_size) {
    int read_index = (_ghost__suggestion_index == 0)?(_ghost__suggestion_buffer_size - 1):(_ghost__suggestion_index - 1); // The index to start reading ghost suggestions from.
    for (int counter = 0; counter < suggestion_size; counter++) {
      int var = _ghost__suggestions[read_index];
      if(var != 0)suggestions[counter] = state->isTrue(var)?var:-var;
      read_index = (read_index == 0)?(_ghost__suggestion_buffer_size - 1):(read_index - 1);
    }
}

// pick a variable by checking each variable of a random clause in turn.
// returning the one that is not assigned in the prefix, otherwise return NOVALUE
int Gnovelty::pickVar_random() {
  int clause = falseClauses->map[random() % (falseClauses->map_size)];
  int firstVarIdx = random() % cnf->cl[clause];
  int offset = 0;
  int var = Var(clause,firstVarIdx+offset);
  while(inPrefix[var] && (offset++ < cnf->cl[clause])) {
    var = Var(clause,((firstVarIdx+offset) % cnf->cl[clause]));
  }
  if (!inPrefix[abs(var)])
    return var;
  return NOVALUE;
}


// purge entries from CandVar array of variables with non-negative score.
void Gnovelty::purge_candVar() {
  for (int i=0; i<candVar->map_size;) {
    int var = candVar->map[i];
    if (variableScore[var]>=0)
      candVar->remove(var);
    else
      i++;
  }
}


// reset entries in CandVar array of variables with non-negative score and are not in prefix.
void Gnovelty::reset_candVar() {
  candVar->reset();
  for (int var = 1; var <= cnf->vc; var++)
    if ((variableScore[var] < 0) && (!inPrefix[var]))
      candVar->append(var);
}


// pick a variable which has the most negative score (and which also is the oldest last changed, in the event of a tie)
// from among the variables which have a score less than zero and are not in the prefix (ie. they are in the candVar array)
// if there are no variables to pick from returns NOVALUE
int Gnovelty::pickVar_greedy() {
  int bestVar = NOVALUE;
  CWTYPE bestValue = BIG;
  int lastChange = flip;
  purge_candVar(); // need to purge the candVar array before iterating over it

  for (int i=0; i<candVar->map_size; i++) {
    int var = candVar->map[i];
    CWTYPE score = variableScore[var];
    if ((score < bestValue) || ((score == bestValue) && (varLastChange[var] < lastChange))) {
      bestValue = score;
      lastChange = varLastChange[var];
      bestVar = var;
    }
  }
  return bestVar;
}

// select a random false clause, and from the variables in the clause that are not in the prefix 
// and find the best and second best one, that has the most negative score (and which also is the oldest last changed, in the event of a tie)
// if there are no variables to pick from returns NOVALUE
int Gnovelty::pickVar_normal() {
  int bestVar = NOVALUE;
  int secondBestVar = NOVALUE;
  CWTYPE bestValue = BIG;
  CWTYPE secondBestValue = BIG;

  int clause = falseClauses->map[random() % (falseClauses->map_size)];
  int youngest = varLastChange[abs(cnf->clauses[clause][0])];
  for (int j = 0; j < cnf->cl[clause]; j++) {
    int var = abs(cnf->clauses[clause][j]);
    if(inPrefix[var]) continue;
    CWTYPE score = variableScore[var];
    int lastChange = varLastChange[var];
    if (lastChange > youngest)
      youngest = lastChange;
    if ( (score < bestValue) || ((score == bestValue) && (lastChange < varLastChange[bestVar])) ) {
      secondBestVar = bestVar;
      secondBestValue = bestValue;
      bestVar = var;
      bestValue = score;
    } else if ( (score < secondBestValue) || ((score == secondBestValue) && (lastChange < varLastChange[secondBestVar])) ) {
      secondBestVar = var;
      secondBestValue = score;
    }
  }
  // noisy swapping best for second best? (Note: this is the only variableNumScore is used... feels dodgy, but preserving behavior of old novelty code)
  /*if ( (varLastChange[bestVar] == youngest) && (random()%100 < noise) )
    bestVar = secondBestVar;*/
  if ( (bestVar != NOVALUE) && (varLastChange[bestVar] == youngest) && ( (variableNumScore[bestVar] == 0) || (random()%100 < noise) ) )
    bestVar = secondBestVar;
  return bestVar;
}

// for a variable toflip, flip the variable's state, and examine the clauses in which it occurs
// if the variable's flip makes the clause false then all the variables of the clause have decreased weight
// if the variable's flip makes the clause true by one variable then that variable has incresaed weight
// if the variable's flip makes the clause true by more than one variable, then the variables do not get a decrease or increase 
//
// NOTE: that this 'old_style' flip, does not keep consistency between the CandVar array and thoes variables which have negative score
// particularly as the neighborhood of a variable does not include the variable itself, so if a variable is flipped, and its score becomes negative
// it is NOT added to the candVar array, this functions as a weird tabu mechanism.
void Gnovelty::flipAtom(int toflip) {
  if (toflip == NOVALUE)
    return;
  //assert(!inPrefix[toflip]);

  int toEnforce;        //=== literal to enforce  
  if (IS_TRUE(toflip)) toEnforce = -toflip;
  else toEnforce = toflip;

  flip++; // Just record keeping, of the number of "moves" the local search has made. 
  varLastChange[toflip] = flip; // keep record the the variable was flipped on this flip
  state->flip(toflip); // actually flip the variable

int* ptr = cnf->neighbourVar[toflip];
  for (; *ptr != 0; ptr++) {
    int var = *ptr;
    if (variableScore[var] >= 0) isCandVar[var] = false;
    else isCandVar[var] = true;
  }

  //=== Examine all occurrences of literals with toEnforce with old sign.
  //=== (these clauses are made false; numTrueLit must be decremented in that clause, and the literals' score must be updated)
  int numocc = cnf->numOccurence[numAtoms-toEnforce];
  int* occ = cnf->occurence[numAtoms-toEnforce];
  for (int i = 0; i < numocc; i++) {
    int c = occ[i];
    if (--numTrueLit[c] == 0) { // if flipping makes the clause false
      falseClauses->append(c);  // clause c is no longer satisfied by lit with toflip; it's now false.
      decScore(toflip, c); //=== Decrement toflip's weight.
      for (int j = 0; j < cnf->cl[c]; j++) //=== Increment make of all vars in clause c.
        decScore(Var(c,j), c);
    } else if (numTrueLit[c] == 1) { // it is true only by one literal
      for (int j = 0; j < cnf->cl[c]; j++) {
        //=== Find the lit in this clause that makes it true, and inc its weight.
        int var = Var(c,j);
        if ((Lit(c,j) > 0) == IS_TRUE(var)) {
          incScore(var, c);
          break;
        }
      }
    }
  }
  //=== Examines all occurrences of literal with toEnforce with new sign 
  //=== (these clauses are made true; numTrueLit must be incremented in that clause, and the literals' score must be updated)
  numocc = cnf->numOccurence[numAtoms+toEnforce];
  occ = cnf->occurence[numAtoms+toEnforce];
  for (int i = 0; i < numocc; i++) {
    int c = occ[i];
    if (++numTrueLit[c] == 1) { // flipping makes this clause true only by one literal
      falseClauses->remove(c); // clause no longer false
      incScore(toflip, c);  //increase the weight of the one literal
      for (int j = 0; j < cnf->cl[c]; j++)  //=== Decrement the weight of all vars in clause c.
        incScore(Var(c,j), c);
    } else if (numTrueLit[c] == 2){ // flipping makes this clause true by more than one variable
      //=== Find the lit in this clause other than toflip that makes it true, and decrement its break.
      for (int j=0; j<cnf->cl[c]; j++){
        int var = Var(c,j);
        if( ((Lit(c,j) > 0) == IS_TRUE(var)) && (toflip != var) ) {
          decScore(var, c);
          break;
        }
      }
    }
  }

  // after the score updates, calculate the differences in isCandVar array
  // for selective addition to candVar array.
  // NOTE: candVar array is NOT just the array of variables with negative score.
  for (int* ptr = cnf->neighbourVar[toflip]; *ptr != 0; ptr++) {
    int var = *ptr;
    if ((variableScore[var] < 0) && (!isCandVar[var]) && (!inPrefix[var]))
      candVar->append(var);
  }
  // register the flip in the _ghost_suggestion buffer
  _ghost__suggestions[_ghost__suggestion_index] = toflip;
  _ghost__suggestion_index = (1 + _ghost__suggestion_index) % _ghost__suggestion_buffer_size;
}

// scan through clauses and set numTrueLit array as well as setting variable scores and candVar and falseClauses arrays appropriately
void Gnovelty::setupScores() {
  falseClauses->reset();
  for (int var = 1; var <= cnf->vc; var++) {
    variableScore[var] = 0;
    variableNumScore[var] = 0;
  }
  for (int i = 0; i < cnf->cc; i++) {
    int critVar;
    numTrueLit[i] = 0;
    numUnsetLit[i] = 0;
    for (int j = 0; j < cnf->cl[i]; j++) {
      int lit = Lit(i,j);
      if ((lit > 0) == IS_TRUE(abs(lit))) {
        numTrueLit[i]++;
        critVar = abs(lit);
      }
    }
    if (numTrueLit[i] == 0) { // if the clause is false, set the falseClauses array to include it
      falseClauses->append(i);
      for (int j = 0; j < cnf->cl[i]; j++) //=== these variables make clause i true, decrease its score to encourage
        decScore(Var(i,j), i);
    } else if (numTrueLit[i] == 1) //=== flipping this variable breaks clause i and makes it false, increase its score to discourage
      incScore(critVar, i);
  }
  reset_candVar();
}



// for a CNF clause of integers 'clause' add it to the CNF and reapply all operations to make Gnovelty account for new clause
void Gnovelty::addClause(int* clause) {
  int old_variable_count = cnf->vc;
  // add clause to CNF and recompute neighborhoods and occurence buffers
  cnf->add_clause(clause);
  cnf->compute_occurance_buffers();
  cnf->compute_variable_neighborhoods();
  // redimension clauseWeight and numTrueLit, numUnsetLIt (dependant on cnf->cc)
  TEST_NOT_NULL(clauseWeight = (CWTYPE*)realloc(clauseWeight, sizeof(CWTYPE)*cnf->cc))
  clauseWeight[cnf->cc-1] = 1;
  TEST_NOT_NULL(solutionClauses = (bool*)realloc(solutionClauses, sizeof(bool)*cnf->cc))
  solutionClauses[cnf->cc-1] = false;
  TEST_NOT_NULL(numTrueLit = (int*)realloc(numTrueLit, sizeof(int)*cnf->cc))
  TEST_NOT_NULL(numUnsetLit = (int*)realloc(numUnsetLit, sizeof(int)*cnf->cc))
  // redimension all other data structures (dependant on cnf->vc)
  TEST_NOT_NULL(variableScore = (CWTYPE*)realloc(variableScore,sizeof(CWTYPE)*(cnf->vc+1)))
  TEST_NOT_NULL(variableNumScore = (int*)realloc(variableNumScore,sizeof(int)*(cnf->vc+1)))
  candVar->resize(cnf->vc+1);
  TEST_NOT_NULL(isCandVar = (int*)realloc(isCandVar,sizeof(int)*(cnf->vc+1)))
  TEST_NOT_NULL(varLastChange = (int*)realloc(varLastChange,sizeof(int)*(cnf->vc+1)))
  TEST_NOT_NULL(inPrefix = (bool*)realloc(inPrefix,sizeof(bool)*(cnf->vc+1)))
  // redimension classes falseClauses, and state
  falseClauses->increase_size(1);
  state->resize(cnf->vc+1);
  // recompute variable scores and update candVar,falseClauses,numTrueLit arrays
  for (int i = old_variable_count+1; i<= cnf->vc; i++) {
    inPrefix[i] = false;
    varLastChange[i] = 0;
  }
  setupScores();
  // reset Adapt novelty noise settings for new solution
  invPhi = 5;
  invTheta = 6;
  lastAdaptFlip = flip; // Adapt Novelty+ Noise stuff
  lastAdaptNumFalse = falseClauses->map_size;
  noise = 0;
}

// select and flip a variable, in accordance with Gnovelty algorithm, the newStyle algorithm has muhc less 
// updateClauseWeight calls than te oldStyle
int Gnovelty::step_newStyle(int walkProb, bool adaptFlag) {
  if (falseClauses->map_size == 0) // If the problem is satisfied by the current assignment \member{state}
    return 1;
  int var = NOVALUE;
  if (random() % 100 < walkProb) { //=== Normal AdaptNovelty+
    var = pickVar_random();
  } else {
    var = pickVar_greedy();
    if (var == NOVALUE) {
      updateClauseWeights();
      var = pickVar_normal();
    }
  }
  if (var != NOVALUE)
    flipAtom(var);
  if (adaptFlag && (candVar->map_size == 0))
    adaptNoveltyNoise();
  return 0;
}

// select and flip a variable, in direct accordance with old Gnovelty
int Gnovelty::step(int walkProb, bool adaptFlag) {
  if (falseClauses->map_size == 0) // If the problem is satisfied by the current assignment \member{state}
    return 1;
  int var = NOVALUE;
  int flags = 1;
  var = pickVar_greedy();
  if (random()%100 < walkProb) { //=== Normal AdaptNovelty+
    var = pickVar_random();
  } else if (candVar->map_size > 0) {
    flags = 0;
  } else {
    updateClauseWeights();
    var = pickVar_normal();
  }
  if (var != NOVALUE)
    flipAtom(var);
  if (adaptFlag && flags) {
    adaptNoveltyNoise();
  }
  return 0;
}

// creates a data_structure holding the soluiton of the problem the gnovelty has come to, and returns it.
// returns NULL if gnovelty is not on a solution, and also attempts to detect and purge supurfluous variables 
// using the numUsetLit array to simulate the unsetting of variables.
// and also adds the negation of the solution to the gnovelty instance.
int* Gnovelty::processSolution() {
  if (falseClauses->map_size != 0) // If the problem is satisfied by the current assignment \member{state}
    return NULL;
  if (solution_style >0)
    for (int i=0; i<cnf->cc; i++) // wipe numUnsetLit array
      numUnsetLit[i] = 0;
  int* solution;
  TEST_NOT_NULL(solution = (int*)calloc(sizeof(int),cnf->vc+1))
  int index = 0;
  for (int i=1; i<=cnf->vc; i++) { // for each state literal
    int literal = state->isTrue(i)?i:-i;
    if (solution_style == 0) {
      solution[index++] = literal;
    } else {
      int numocc = cnf->numOccurence[numAtoms+literal];
      int* occ = cnf->occurence[numAtoms+literal];
      bool admit = false;
      for (int i = 0; i < numocc; i++) { // for all non-solution-clauses in which the literal occurs
        int c = occ[i];
        if ((solutionClauses[c] == false) || (solution_style==1))
          if (numTrueLit[c] - numUnsetLit[c] <= 1) // if unsetting the literal will make a clause false, dont add it to solution
            admit = true;
      }
      if (admit == false) // if the litteral is not added to the solution, unset it.
        for (int i = 0; i < numocc; i++)
          numUnsetLit[occ[i]]++;
      else // otherwise add the litteral to the solution
        solution[index++] = literal;
    }
  }
  for (int i=0; solution[i]!=0; i++)
    solution[i] *= -1;
  addClause(solution);
  solutionClauses[cnf->cc-1] = true;
  for (int i=0; solution[i]!=0; i++)
    solution[i] *= -1;
  return solution;
}


// creates a data_structure holding the soluiton of the problem the gnovelty has come to, and returns it.
// returns NULL if gnovelty is not on a solution, and also attempts to detect and purge supurfluous variables 
// using the numUsetLit array to simulate the unsetting of variables.
// and also adds the negation of the solution to the gnovelty instance.
int* Gnovelty::processSolution(vector<int> &variables) {
  set<int> variable_set(variables.begin(), variables.end());
  if (falseClauses->map_size != 0) // If the problem is satisfied by the current assignment \member{state}
    return NULL;
  if (solution_style >0)
    for (int i=0; i<cnf->cc; i++) // wipe numUnsetLit array
      numUnsetLit[i] = 0;
  int* solution;
  TEST_NOT_NULL(solution = (int*)calloc(sizeof(int),cnf->vc+1))
  int index = 0;
  int* solution2;
  TEST_NOT_NULL(solution2 = (int*)calloc(sizeof(int),cnf->vc+1))
  int index2 = 0;
  for (int i=1; i<=cnf->vc; i++) { // for each state literal
    int literal = state->isTrue(i)?i:-i;
    if (solution_style == 0) {
      solution[index++] = literal;
    } else {
      int numocc = cnf->numOccurence[numAtoms+literal];
      int* occ = cnf->occurence[numAtoms+literal];
      bool admit = false;
      for (int i = 0; i < numocc; i++) { // for all non-solution-clauses in which the literal occurs
        int c = occ[i];
        if ((solutionClauses[c] == false) || (solution_style==1))
          if (numTrueLit[c] - numUnsetLit[c] <= 1) // if unsetting the literal will make a clause false, dont add it to solution
            admit = true;
      }
      if (admit == false) // if the litteral is not added to the solution, unset it.
        for (int i = 0; i < numocc; i++)
          numUnsetLit[occ[i]]++;
      else // otherwise add the litteral to the solution
        solution[index++] = literal;
    }
  }
  for (int i=0; solution[i]!=0; i++) { // for each state literal
    if (variable_set.find(abs(solution[i])) != variable_set.end()) {
      solution2[index2++] = -solution[i];
    }
  }
  addClause(solution2);
  free(solution2);
  solutionClauses[cnf->cc-1] = true;
  return solution;
}


// creates a data_structure holding the soluiton of the problem the gnovelty has come to, and returns it.
// returns NULL if gnovelty is not on a solution, and also attempts to detect and purge supurfluous variables 
// using the numUsetLit array to simulate the unsetting of variables.
// and also adds the negation of the solution to the gnovelty instance.
int* Gnovelty::processSolution(set<int> &variable_set) {
  if (falseClauses->map_size != 0) // If the problem is satisfied by the current assignment \member{state}
    return NULL;
  if (solution_style >0)
    for (int i=0; i<cnf->cc; i++) // wipe numUnsetLit array
      numUnsetLit[i] = 0;
  int* solution;
  TEST_NOT_NULL(solution = (int*)calloc(sizeof(int),cnf->vc+1))
  int index = 0;
  int* solution2;
  TEST_NOT_NULL(solution2 = (int*)calloc(sizeof(int),cnf->vc+1))
  int index2 = 0;
  for (int i=1; i<=cnf->vc; i++) { // for each state literal
    int literal = state->isTrue(i)?i:-i;
    if (solution_style == 0) {
      solution[index++] = literal;
    } else {
      int numocc = cnf->numOccurence[numAtoms+literal];
      int* occ = cnf->occurence[numAtoms+literal];
      bool admit = false;
      for (int i = 0; i < numocc; i++) { // for all non-solution-clauses in which the literal occurs
        int c = occ[i];
        if ((solutionClauses[c] == false) || (solution_style==1))
          if (numTrueLit[c] - numUnsetLit[c] <= 1) // if unsetting the literal will make a clause false, dont add it to solution
            admit = true;
      }
      if (admit == false) // if the litteral is not added to the solution, unset it.
        for (int i = 0; i < numocc; i++)
          numUnsetLit[occ[i]]++;
      else // otherwise add the litteral to the solution
        solution[index++] = literal;
    }
  }
  for (int i=0; solution[i]!=0; i++) { // for each state literal
    if (variable_set.find(abs(solution[i])) != variable_set.end()) {
      solution2[index2++] = -solution[i];
    }
  }
  addClause(solution2);
  free(solution2);
  solutionClauses[cnf->cc-1] = true;
  return solution;
}


// creates a data_structure holding the soluiton of the problem the gnovelty has come to, and returns it.
// returns NULL if gnovelty is not on a solution, and also attempts to detect and purge supurfluous variables 
// using the numUsetLit array to simulate the unsetting of variables.
// and also adds the negation of the solution to the gnovelty instance.
int* Gnovelty::processSolution(RangeSet &variable_set) {
  if (falseClauses->map_size != 0) // If the problem is satisfied by the current assignment \member{state}
    return NULL;
  if (solution_style >0)
    for (int i=0; i<cnf->cc; i++) // wipe numUnsetLit array
      numUnsetLit[i] = 0;
  int* solution;
  TEST_NOT_NULL(solution = (int*)calloc(sizeof(int),cnf->vc+1))
  int index = 0;
  int* solution2;
  TEST_NOT_NULL(solution2 = (int*)calloc(sizeof(int),cnf->vc+1))
  int index2 = 0;
  for (int i=1; i<=cnf->vc; i++) { // for each state literal
    int literal = state->isTrue(i)?i:-i;
    if (solution_style == 0) {
      solution[index++] = literal;
    } else {
      int numocc = cnf->numOccurence[numAtoms+literal];
      int* occ = cnf->occurence[numAtoms+literal];
      bool admit = false;
      for (int i = 0; i < numocc; i++) { // for all non-solution-clauses in which the literal occurs
        int c = occ[i];
        if ((solutionClauses[c] == false) || (solution_style==1))
          if (numTrueLit[c] - numUnsetLit[c] <= 1) // if unsetting the literal will make a clause false, dont add it to solution
            admit = true;
      }
      if (admit == false) // if the litteral is not added to the solution, unset it.
        for (int i = 0; i < numocc; i++)
          numUnsetLit[occ[i]]++;
      else // otherwise add the litteral to the solution
        solution[index++] = literal;
    }
  }
  for (int i=0; solution[i]!=0; i++) { // for each state literal
    if (variable_set.find(abs(solution[i]))) {
      solution2[index2++] = -solution[i];
    }
  }
  addClause(solution2);
  free(solution2);
  solutionClauses[cnf->cc-1] = true;
  return solution;
}




void Gnovelty_updateClauseWeights_Linear::addClause(int *clause) {
  Gnovelty::addClause(clause);
  weightedClauses->increase_size(1);
}

// the updateClauseWight functions....
void Gnovelty_updateClauseWeights_Linear::updateClauseWeights() {
  for (int i = 0; i < falseClauses->map_size; i++) {
    int c = falseClauses->map[i];
    clauseWeight[c] += 1;
    if (clauseWeight[c] == 2) 
      weightedClauses->append(c);  //=== Add clause to weighted list
    for (int j = 0; j < cnf->cl[c]; j++)
      adjScore(Var(c, j), -1);
  }
  if (random()%1000 < smoothProb*10) smooth();
  reset_candVar();
}

void Gnovelty_updateClauseWeights_Linear::smooth() {
  for (int i = 0; i < weightedClauses->map_size; i++) {
    int c = weightedClauses->map[i];
    clauseWeight[c] -= 1;
    if (clauseWeight[c] == 1) {
      weightedClauses->remove(c); // remove c, and decrement i so that it proceeds with the next element proerly
      i--;
    }
    if (numTrueLit[c] == 0)
      for (int j = 0; j < cnf->cl[c]; j++) adjScore(Var(c,j), 1);
    //else if (numTrueLit[c] == 1)
    //  adjScore(critVar[c], -1);
  }
  reset_candVar();
}

/*
void SearchInstance_updateClauseWeights_Exp::updateClauseWeights() {
    register int c, i, j, var;
    for (i = 0; i < numFalse; i++) {
        c = falseClause[i];
        stepSize[c] = stepSize[c] * exp(- 1.0 * pseudoStepSize *  trace[c]);
        trace[c] = traceDiscount * trace[c] + _EXP(stepSize[c]);
        clauseWeight[c] += stepSize[c];
        for (j = 0; j < cnf->cl[c]; j++) {
            var = Var(c, j);
            adjScore(var, -stepSize[c]);
        }
    }
}
*/

