/*************************
Copyright 2020 Mark Burgess

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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <glog/logging.h>
#include <algorithm>
#include "Cnf.h"
#include "utilities.h"
#include "exceptions.h"
#include <assert.h>
#include "mpi_global.h"
#include <vector>
#include "limits.h"



//-----------------------------------------------
//
//         Constructors / Destructors
//
//-----------------------------------------------

Cnf::Cnf() {
  cc = 0;
  vc = 0;
  TEST_NOT_NULL(clauses = (int**)calloc(sizeof(int*),1))
  TEST_NOT_NULL(cl = (unsigned int*)calloc(sizeof(int*),1))
  occurence = NULL;
  numOccurence = NULL;
  neighbourVar = NULL;
  this->dereferenced = true;
}

// default call with array-of-arrays of integers defaults to a dereferencing call
Cnf::Cnf(int** new_data) : Cnf::Cnf(new_data,true) {}

// populate a CNF from a NULL terminated array-of-arrays of integers
// dereferenced is true, if the new CNF's clause data is dereferenced from the supplied array-of-arrays
// otherwise new CNFs array of arrays is exactly the supplied array-of-arrays
Cnf::Cnf(int** new_data, bool dereferenced) {
  this->dereferenced = dereferenced;
  if (new_data == NULL)
    throw BadParameterException("CNF cannot be instantiated from null data");
  if (!dereferenced)
    clauses = new_data;
  else {
    for (cc=0; new_data[cc]; cc++);
    TEST_NOT_NULL(clauses = (int**)calloc(sizeof(int*),cc+1))
    for (int i=0; i<cc; i++) {
      int cl;
      for (cl=0; new_data[i][cl]; cl++);
      TEST_NOT_NULL(clauses[i] = (int*)calloc(sizeof(int),cl+1))
      for (int j=0; j<cl; j++)
        clauses[i][j] = new_data[i][j];
    }
  }
  populate_from_clauses();
  occurence = NULL;
  numOccurence = NULL;
  neighbourVar = NULL;
}


// populate a cnf from a CNF dehydrated string of integers
Cnf::Cnf(int* dehydrated_data) {
  occurence = NULL;
  numOccurence = NULL;
  neighbourVar = NULL;
  dereferenced = true;
  hydrate(dehydrated_data);
}


// load a CNF from a file whos name is given by fname
Cnf::Cnf(const char *fname) {
  occurence = NULL;
  numOccurence = NULL;
  neighbourVar = NULL;
  dereferenced = true;
  load_DIMACS_Cnf(fname);
}

// load a CNF from a file whos name is given by fname
Cnf::Cnf(const char *fname, const vector<int> &indices) {
  occurence = NULL;
  numOccurence = NULL;
  neighbourVar = NULL;
  dereferenced = true;
  load_DIMACS_Cnf(fname, indices);
}

// load a CNF from a file whos name is given by fname
Cnf::Cnf(const char *fname, RangeSet &indices) {
  occurence = NULL;
  numOccurence = NULL;
  neighbourVar = NULL;
  dereferenced = true;
  load_DIMACS_Cnf(fname, indices);
}

// given another CNF, duplicate it (minus occurence and neighborhoods data), everythin dereferenced
Cnf::Cnf(Cnf* cnf) {
  vc = cnf->vc;
  cc = cnf->cc;
  TEST_NOT_NULL(cl = (unsigned*)calloc(sizeof(unsigned),cc+1))
  for (int i=0; i<cc; i++)
    cl[i] = cnf->cl[i];
  TEST_NOT_NULL(clauses = (int**)calloc(sizeof(int*),cc+1))
  for (int i=0; i<cc; i++) {
    TEST_NOT_NULL(clauses[i] = (int*)calloc(sizeof(int),cl[i]+1))
    for (int j=0; j<cl[i]; j++) {
      clauses[i][j] = cnf->clauses[i][j];
    }
  }
  if (cnf->occurence != NULL)
    occurence = copy_2D_int_array(cnf->occurence,cnf->numOccurence,cnf->vc*2+1);
  else
    occurence = NULL;
  if (cnf->numOccurence != NULL)
    numOccurence = copy_1D_int_array(cnf->numOccurence,cnf->vc*2+1);
  else
    numOccurence = NULL;
  if (cnf->neighbourVar != NULL)
    neighbourVar = copy_2D_int_array(cnf->neighbourVar,cnf->vc+1);
  else
    neighbourVar = NULL;
  this->dereferenced = true;
}


// given another CNF, duplicate it (minus occurence and neighborhoods data), everythin dereferenced
Cnf::Cnf(Cnf* cnf, RangeSet &set_indices) {
  cc = set_indices.size();
  TEST_NOT_NULL(clauses = (int**)calloc(sizeof(int*),cc+1))
  int i = 0;
  for (auto it = set_indices.buffer.begin(); it!=set_indices.buffer.end(); it++) {
    for (int c = (*it).first; c<=(*it).second; c++) {
      if ((c<0) || (c>=cnf->cc))
        throw ParsingException("loading CNF from indices that dont exists as clauses \n");
      clauses[i] = cnf->clauses[c];
      i++;
    }
  }
  populate_from_clauses();
  occurence = NULL;
  numOccurence = NULL;
  neighbourVar = NULL;
  this->dereferenced = false;
}


// standard destructor, free all data structures
Cnf::~Cnf(){
  if(clauses){
    if (this->dereferenced) {for(unsigned i = 0; i < cc; i++) free(clauses[i]);}
      free(clauses);
  }
  if (cl)
    free(cl);
  free_occurence_and_neighborhood_buffers();
}




//-----------------------------------------------
//
//         Input / Output
//
//-----------------------------------------------

// dump the CNF into a DIMACS formatted cnf file
void Cnf::output_dimacs(const char *fname) {
  FILE* f;
  TEST_NOT_NULL(f = fopen(fname,"w"))
  fprintf(f, "p cnf %d %d\n", vc, cc);
  for (int i=0; i<cc; i++) {
    for (int j=0; j<cl[i]; j++) {
      fprintf(f,"%i ",clauses[i][j]);
    }
    fprintf(f,"0\n");
  }
  fclose(f);
}

// dump the CNF into a DIMACS formatted cnf file
void Cnf::output_dimacs(FILE* f) {
  fprintf(f, "p cnf %d %d\n", vc, cc);
  for (int i=0; i<cc; i++) {
    for (int j=0; j<cl[i]; j++) {
      fprintf(f,"%i ",clauses[i][j]);
    }
    fprintf(f,"0\n");
  }
}

// print CNF information to terminal using printf's
void Cnf::print() {
  printf("p cnf %d %d\n", vc, cc);
  for (int i=0; i<cc; i++) {
    for (int j=0; j<cl[i]; j++) {
      printf("%i ",clauses[i][j]);
    }
    printf("0\n");
  }
  if (numOccurence != NULL) {
    printf("variable occurences\n");
    for (int i=0; i<2*vc+1; i++) {
      printf("%i, %i: ",i-vc,numOccurence[i]);
      for (int j=0; j<numOccurence[i]; j++) {
        printf("%i ",occurence[i][j]);
      }
      printf("\n");
    }
  }
  if (neighbourVar != NULL) {
    printf("variable neighbourhoods\n");
    for (int v=1; v<=vc; v++) {
      printf("%i: ",v);
      for (int* k = neighbourVar[v]; *k!=0; k++) {
        printf("%i ",*k);
      }
      printf("\n");
    }
  }
}

// returns the size of the dehydrated string (in ints)
int Cnf::get_dehydrated_size() {
  int upto = 2 + cc + 1;
  for (int i = 0; i < cc; i++)
    upto += cl[i]+1;
  return upto;
}

// convert the CNF into a linear array of integers
int Cnf::dehydrate(int *output_data) {
  int upto = 1;
  output_data[upto++] = cc;
  for (int i = 0; i < cc; i++)
    output_data[upto++]=cl[i];
  output_data[upto++]=0;
  for (int i = 0; i < cc; i++) {
    for (int j = 0; j<cl[i]; j++)
      output_data[upto++] = clauses[i][j];
    output_data[upto++]=0;
  }
  output_data[0] = upto;
  return upto;
}

// from a linear array of integers created by dehydrate function, hydrate it into a CNF
void Cnf::hydrate(int* input_data) {
  if (input_data == NULL)
    throw BadParameterException("CNF cannot hydrate NULL");
  free_occurence_and_neighborhood_buffers();
  int upto = 0;
  int size = input_data[upto++];
  int cc = input_data[upto++];
  TEST_NOT_NULL(clauses = (int**)calloc(sizeof(int*),cc+1))
  for (int i=0; i<cc; i++)
    TEST_NOT_NULL(clauses[i] = (int*)calloc(sizeof(int),input_data[upto++]+1))
  upto++;
  int clause_upto = 0;
  int clause_index_upto = 0;
  while (clause_upto < cc) {
    if (input_data[upto] == 0) {
      clause_upto++;
      clause_index_upto = 0;
    } else {
      clauses[clause_upto][clause_index_upto] = input_data[upto];
      clause_index_upto++;
    }
    upto++;
  }
  populate_from_clauses();
}


// for a given file name, open the file as a DIMACS formatted CNF and load it in to class memory
void Cnf::load_DIMACS_Cnf(const char* fname) {
  FILE *ifp;
  TEST_NOT_NULL(ifp = fopen(fname, "r"));
  load_DIMACS_Cnf(ifp);
  fclose(ifp);
}

// for a given file stream, open the file as a DIMACS formatted CNF and load it in to class memory
void Cnf::load_DIMACS_Cnf(FILE* ifp) {
  cc = 0;
  vc = 0;
  int max_clause_len = 1024, max_clause_count = 1024, *literals;
  TEST_NOT_NULL(literals = (int *) malloc(max_clause_len * sizeof(int)))

  char line[100000];
  size_t len = 100000;
  char c;
  
  // search for and read the header
  int header_vc, header_cc;
  while((c=getc(ifp)) != EOF){ 
    if (isspace(c)) continue; else ungetc(c,ifp);
    if (fgets(line, len, ifp)==NULL)
      throw ParsingException(" Unexpected line read error\n");
    if (c=='p'){
      if(sscanf(line, "p cnf %d %d", &header_vc, &header_cc) == 2)
        break;
      else
        throw ParsingException(" Invalid CNF file\n");
    }
  }
  if (header_cc==0) {
    clauses[cc] = NULL;
    cl[cc] = 0;
    free(literals);
    return;
  }
  // allocate original data buffers
  TEST_NOT_NULL(clauses = (int **) calloc(max_clause_count, sizeof(int *)))
  TEST_NOT_NULL(cl = (unsigned *) calloc(max_clause_count, sizeof(unsigned)))

  // for each line
  while((c=getc(ifp)) != EOF){
    if (isspace(c)) continue; else ungetc(c,ifp);
    // search for the first non-whitespace character
    if ((c=='-') || isdigit(c)) {
      int literal_input_count;
      int literal_input;
      int j=-1;
      // scan the line into the literals buffer one character at a time, until the zero is scanned
      do {
        j++;
        literal_input_count = fscanf(ifp, "%d", &literal_input);
        if (literal_input_count == 0)
          throw ParsingException("Invalid CNF file - cnf lines must involve digits and terminate with zero\n");
        if (j == max_clause_len) {
          max_clause_len *= 2;
          TEST_NOT_NULL(literals = (int *) realloc(literals, max_clause_len * sizeof(int)))
        }
        literals[j] = literal_input;
        if (abs(literal_input) > vc)
          vc = abs(literal_input);
      } while (literal_input != 0);
      // allocate room for the new clause
      TEST_NOT_NULL(clauses[cc] = (int *) calloc(j + 1, sizeof(int)))
      // load the new clause in checking for duplicate and contradicting literals
      for(int k = 0; k <= j; k++){
        for(int x = 0; x < k; x++) {
          if(literals[x] == literals[k])
            throw ParsingException("duplicate literals in clause in CNF file \n");
          else if (literals[x] + literals[k] == 0)
            throw ParsingException("contradicting literals in clause in CNF file \n");
        }
        clauses[cc][k] = literals[k];
      }
      // set the clause length, increment the clause count, and expand buffers as nessisary
      cl[cc] = j;
      cc++;
      if(cc+1 >= max_clause_count) {
        max_clause_count *= 2;
        TEST_NOT_NULL(clauses = (int **)realloc(clauses, max_clause_count * sizeof(int *)))
        TEST_NOT_NULL(cl = (unsigned *) realloc(cl,max_clause_count * sizeof(unsigned)))
      }
    }
    if (cc==header_cc) {
      break;
    }
    // get a new line
    if (fgets(line, len, ifp) == NULL)
      throw ParsingException(" Unexpected line read error\n");
  }
  clauses[cc] = NULL;
  cl[cc] = 0;
    if ((header_vc != vc) || (header_cc != cc))
    throw ParsingException("CNF has header that dosnt match its body - bad variable count or clause count \n");
  free(literals);
}



// for a given file name, open the file as a DIMACS formatted CNF and load it in to class memory
// only include lines specified by vector of line indices
void Cnf::load_DIMACS_Cnf(const char* fname, const vector<int> &indices) {
  FILE *ifp;
  TEST_NOT_NULL(ifp = fopen(fname, "r"));
  load_DIMACS_Cnf(ifp, indices);
  fclose(ifp);
}


// for a given file stream, open the file as a DIMACS formatted CNF and load it in to class memory
// only include lines specified by vector of line indices
void Cnf::load_DIMACS_Cnf(FILE* ifp, const vector<int> &indices) {
  cc = 0;
  vc = 0;
  int max_clause_len = 1024, max_clause_count = 1024, *literals;
  TEST_NOT_NULL(literals = (int *) malloc(max_clause_len * sizeof(int)))
  char line[100000];
  size_t len = 100000;
  char c;
  
  // search for and read the header
  int header_vc, header_cc;
  while((c=getc(ifp)) != EOF){ 
    if (isspace(c)) continue; else ungetc(c,ifp);
    if (fgets(line, len, ifp)==NULL)
      throw ParsingException(" Unexpected line read error\n");
    if (c=='p'){
      if(sscanf(line, "p cnf %d %d", &header_vc, &header_cc) == 2)
        break;
      else
        throw ParsingException(" Invalid CNF file\n");
    }
  }
  // allocate original data buffers
  TEST_NOT_NULL(clauses = (int **) calloc(max_clause_count, sizeof(int *)))
  TEST_NOT_NULL(cl = (unsigned *) calloc(max_clause_count, sizeof(unsigned)))
  
  int max_index = 0;
  int min_index = INT_MAX;
  for (auto it = indices.begin(); it!=indices.end(); it++) {
    if (*it > max_index)
      max_index = *it;
    if (*it < min_index)
      min_index = *it;
  }
  set<int> set_indices(indices.begin(), indices.end());

  // for each line
  int clause_index = 0;
  while((c=getc(ifp)) != EOF){
    if (isspace(c)) continue; else ungetc(c,ifp);
    if (clause_index==header_cc) {
      break;
    }
    // search for the first non-whitespace character
    if ((c=='-') || isdigit(c)) {
      // if clause_index in indices then include it, else bypass
      bool in_list = false;
      if ((clause_index >= min_index) && (clause_index <= max_index)) {
        if (set_indices.find(clause_index) != set_indices.end()) {
          in_list = true;
        }
      }
      clause_index++;
      if (in_list==false) {
        if (fgets(line, len, ifp)==NULL)
          throw ParsingException(" Unexpected line read error\n");
        continue;
      }
      int literal_input_count;
      int literal_input;
      int j=-1;
      // scan the line into the literals buffer one character at a time, until the zero is scanned
      do {
        j++;
        literal_input_count = fscanf(ifp, "%d", &literal_input);
        if (literal_input_count == 0)
              throw ParsingException("Invalid CNF file - cnf lines must involve digits and terminate with zero\n");
        if (j == max_clause_len) {
          max_clause_len *= 2;
          TEST_NOT_NULL(literals = (int *) realloc(literals, max_clause_len * sizeof(int)))
        }
        literals[j] = literal_input;
        if (abs(literal_input) > vc)
          vc = abs(literal_input);
      } while (literal_input != 0);
      // allocate room for the new clause
      TEST_NOT_NULL(clauses[cc] = (int *) calloc(j + 1, sizeof(int)))
      // load the new clause in checking for duplicate and contradicting literals
      for(int k = 0; k <= j; k++){
        for(int x = 0; x < k; x++) {
          if(literals[x] == literals[k])
            throw ParsingException("duplicate literals in clause in CNF file \n");
          else if (literals[x] + literals[k] == 0)
            throw ParsingException("contradicting literals in clause in CNF file \n");
        }
        clauses[cc][k] = literals[k];
      }
      // set the clause length, increment the clause count, and expand buffers as nessisary
      cl[cc] = j;
      cc++;
      if(cc+1 >= max_clause_count) {
        max_clause_count *= 2;
        TEST_NOT_NULL(clauses = (int **)realloc(clauses, max_clause_count * sizeof(int *)))
        TEST_NOT_NULL(cl = (unsigned *) realloc(cl,max_clause_count * sizeof(unsigned)))
      }
    }
    // get a new line
    if (fgets(line, len, ifp) == NULL)
      throw ParsingException(" Unexpected line read error\n");
  }
  clauses[cc] = NULL;
  cl[cc] = 0;
    if (cc!=indices.size())
    throw ParsingException("loading CNF from indices that dont exists as clauses \n");
  free(literals);
}




// for a given file name, open the file as a DIMACS formatted CNF and load it in to class memory
// only include lines specified by vector of line indices
void Cnf::load_DIMACS_Cnf(const char* fname, RangeSet &set_indices) {
  FILE *ifp;
  TEST_NOT_NULL(ifp = fopen(fname, "r"));
  load_DIMACS_Cnf(ifp, set_indices);
  fclose(ifp);
}


// for a given file stream, open the file as a DIMACS formatted CNF and load it in to class memory
// only include lines specified by vector of line indices
void Cnf::load_DIMACS_Cnf(FILE* ifp, RangeSet &set_indices) {
  cc = 0;
  vc = 0;
  int max_clause_len = 1024, max_clause_count = 1024, *literals;
  TEST_NOT_NULL(literals = (int *) malloc(max_clause_len * sizeof(int)))
  char line[100000];
  size_t len = 100000;
  char c;
  
  // search for and read the header
  int header_vc, header_cc;
  while((c=getc(ifp)) != EOF){ 
    if (isspace(c)) continue; else ungetc(c,ifp);
    if (fgets(line, len, ifp) == NULL)
      throw ParsingException(" Unexpected line read error\n");
    if (c=='p'){
      if(sscanf(line, "p cnf %d %d", &header_vc, &header_cc) == 2)
        break;
      else
        throw ParsingException(" Invalid CNF file\n");
    }
  }
  // allocate original data buffers
  TEST_NOT_NULL(clauses = (int **) calloc(max_clause_count, sizeof(int *)))
  TEST_NOT_NULL(cl = (unsigned *) calloc(max_clause_count, sizeof(unsigned)))
  
  // for each line
  int clause_index = -1;
  while((c=getc(ifp)) != EOF){
    if (clause_index==header_cc) {
      break;
    }
    if (isspace(c)) continue; else ungetc(c,ifp);
    // search for the first non-whitespace character
    if ((c=='-') || isdigit(c)) {
      // if clause_index in indices then include it, else bypass
      clause_index++;
      if (!(set_indices.find(clause_index))) {
        if (fgets(line, len, ifp) == NULL)
          throw ParsingException(" Unexpected line read error\n");
        continue;
      }
      int literal_input_count;
      int literal_input;
      int j=-1;
      // scan the line into the literals buffer one character at a time, until the zero is scanned
      do {
        j++;
        literal_input_count = fscanf(ifp, "%d", &literal_input);
        if (literal_input_count == 0)
              throw ParsingException("Invalid CNF file - cnf lines must involve digits and terminate with zero\n");
        if (j == max_clause_len) {
          max_clause_len *= 2;
          TEST_NOT_NULL(literals = (int *) realloc(literals, max_clause_len * sizeof(int)))
        }
        literals[j] = literal_input;
        if (abs(literal_input) > vc)
          vc = abs(literal_input);
      } while (literal_input != 0);
      // allocate room for the new clause
      TEST_NOT_NULL(clauses[cc] = (int *) calloc(j + 1, sizeof(int)))
      // load the new clause in checking for duplicate and contradicting literals
      for(int k = 0; k <= j; k++){
        for(int x = 0; x < k; x++) {
          if(literals[x] == literals[k])
            throw ParsingException("duplicate literals in clause in CNF file \n");
          else if (literals[x] + literals[k] == 0)
            throw ParsingException("contradicting literals in clause in CNF file \n");
        }
        clauses[cc][k] = literals[k];
      }
      // set the clause length, increment the clause count, and expand buffers as nessisary
      cl[cc] = j;
      cc++;
      if(cc+1 >= max_clause_count) {
        max_clause_count *= 2;
        TEST_NOT_NULL(clauses = (int **)realloc(clauses, max_clause_count * sizeof(int *)))
        TEST_NOT_NULL(cl = (unsigned *) realloc(cl,max_clause_count * sizeof(unsigned)))
      }
    }
    // get a new line
    if (fgets(line, len, ifp) == NULL)
      throw ParsingException(" Unexpected line read error\n");
  }
  clauses[cc] = NULL;
  cl[cc] = 0;
    if (cc!=set_indices.size()) {
      throw ParsingException("loading CNF from indices that dont exists as clauses \n");
    }
  free(literals);
}






//-----------------------------------------------
//
//         Processing
//
//-----------------------------------------------


// populate the numOccurence and occurence buffers
// which store the number of number of times each variable occurs
// and in what clauses they occur respectively
void Cnf::compute_occurance_buffers() {
  if ((occurence!=NULL) || (numOccurence!=NULL))
    return;
  TEST_NOT_NULL(numOccurence = (int*)calloc(sizeof(int),vc*2+1+1))
  TEST_NOT_NULL(occurence = (int**)calloc(sizeof(int*),vc*2+1+1))
  // count the number of occurances of each literal
  for (int i=0; i < cc; i++)
    for (int j=0; j < cl[i]; j++)
      numOccurence[clauses[i][j]+vc]++; // Increment the number of occurences of this literal in the input formula
  // allocate the occurence buffer, which stores clause indices which contain each literal
  for (int i = 0; i < vc*2+1; i++) {
    if (numOccurence[i]>0)
      TEST_NOT_NULL(occurence[i] = (int*)calloc(sizeof(int),numOccurence[i]+1))
    numOccurence[i] = 0; // set this to zero as a counter for next part
  }
  // load the data into the occurence buffer, recalculating numOccurence's.
  for (int i = 0; i < cc; i++)
    for (int j = 0; j < cl[i]; j++) {
      int pos = clauses[i][j]+vc;
      occurence[pos][numOccurence[pos]] = i;
      numOccurence[pos]++;
    }
}

#include <map>
#include <vector>
#include <set>

// compile list of neighbors for each variable, from the clauses where the variable occurs
// where a neighbor is another variable present in the same clause
// where neighbourVar is the array of neighbors for each of the variables (and is 0 terminated)
// note that numOccurence and occurence buffers must be created (calling compute_occurance_buffers) before this method called.
void Cnf::compute_variable_neighborhoods() {
  compute_occurance_buffers();
  if (neighbourVar!=NULL)
    return;
  TEST_NOT_NULL(neighbourVar = (int**)calloc(sizeof(int*),vc+1+1))
  std::vector<std::set<int> > neighbours(vc+1);
  std::vector<int> temp__variables(vc);
  
  for (auto clause_index = 0; clause_index < cc /*clause count*/; clause_index++){
    auto clause = clauses[clause_index];
    auto clause_length = cl[clause_index];
    
    size_t index__temp__variables = 0;
    for (auto literal_index = 0; literal_index < clause_length ; literal_index++){
      auto literal = clause[literal_index];
      auto variable = abs(literal);
      temp__variables[index__temp__variables++] = variable;
    }

    for (auto index = 0; index < index__temp__variables; index++){
      neighbours[temp__variables[index]].insert(temp__variables.begin(), temp__variables.begin() + index__temp__variables);
    }
  }
  
  // for each variable
  for (int i = 1; i < vc+1; i++) {
    if (neighbours[i].size() == 0) {
      TEST_NOT_NULL(neighbourVar[i] = (int*)calloc(sizeof(int),1))
      continue;
    };
    // store what variables are neighbors in neighbourVar array, terminating with 0
    // note: neighbours[i] includes i, so we need to avoid adding it
    int numNeighbours = neighbours[i].size();//std::count(neighbours, neighbours+vc+1, i);
    TEST_NOT_NULL(neighbourVar[i] = (int*)calloc(sizeof(int),numNeighbours))
    neighbourVar[i][numNeighbours-1] = 0;
    int* storeptr = neighbourVar[i]; 
    for (auto p = neighbours[i].begin(); p != neighbours[i].end(); p++) {
	if (*p != i) {
          *(storeptr++) = *p;
	}
    }
  }
}
void Cnf::compute_variable_neighborhoods__DEPRECATED() {
  compute_occurance_buffers();
  if (neighbourVar!=NULL)
    return;
  TEST_NOT_NULL(neighbourVar = (int**)calloc(sizeof(int*),vc+1))
  int* neighbours;
  TEST_NOT_NULL(neighbours = (int*)calloc(sizeof(int),vc+1))
  // quick sort all the clauses
  for (int i=0; i < cc; i++)
    qsort(clauses[i], cl[i], sizeof(int), compareAbsInts);

  // for each variable
  for (int i = 1; i < vc+1; i++) {
    // for every clause in which the variable occurs positively
    for (int j = 0; j < numOccurence[vc-i]; j++) {
      int c = occurence[vc-i][j];
      for (int k = 0; k < cl[c]; k++) { // for each literal in the clause (already quick sorted)
        neighbours[abs(clauses[c][k])] = i;
      }
    }
    // for all the clauses in which the variable occurs negatively
    for (int j = 0; j < numOccurence[vc+i]; j++) { 
      int c = occurence[vc+i][j];
      for (int k = 0; k < cl[c]; k++) { // for each literal in the clause (already quick sorted)
        neighbours[abs(clauses[c][k])] = i;
      }
    }
    // store what variables are neighbors in neighbourVar array, terminating with 0
    int numNeighbours = std::count(neighbours, neighbours+vc+1, i);
    TEST_NOT_NULL(neighbourVar[i] = (int*)calloc(sizeof(int),numNeighbours+1))
    neighbourVar[i][numNeighbours] = 0;
    int* storeptr = neighbourVar[i]; 
    for (int j = 1; j < vc+1; j++)
      if (neighbours[j] == i && j != i)
        *(storeptr++) = j;
  }
  free(neighbours);
}


void Cnf::compute_variable_neighborhoods__DEPRECATED_2() {
  compute_occurance_buffers();
  if (neighbourVar!=NULL)
    return;
  TEST_NOT_NULL(neighbourVar = (int**)calloc(sizeof(int*),vc+1))
  int* neighbours;
  TEST_NOT_NULL(neighbours = (int*)calloc(sizeof(int),vc+1))
  int* neighbour_list;
  TEST_NOT_NULL(neighbour_list = (int*)calloc(sizeof(int),vc+1))
  int num_neighbour_list;
  int** abs_clauses = copy_2D_int_array_abs(clauses);

  // for each variable
  for (int i = 1; i < vc+1; i++) {
    num_neighbour_list = 0;
    // for every clause in which the variable occurs positively
    for (int j = 0; j < numOccurence[vc-i]; j++) {
      int c = occurence[vc-i][j];
      int clc = cl[c];
      for (int k = 0; k < clc; k++) { // for each literal in the clause
        //int var = abs(clauses[c][k]);
        int var = abs_clauses[c][k];
        if (neighbours[var] != i) { 
          neighbours[var] = i;
          neighbour_list[num_neighbour_list++]=var;
        }
      }
    }
    // for all the clauses in which the variable occurs negatively
    for (int j = 0; j < numOccurence[vc+i]; j++) { 
      int c = occurence[vc+i][j];
      int clc = cl[c];
      for (int k = 0; k < clc; k++) { // for each literal in the clause
        //int var = abs(clauses[c][k]);
        int var = abs_clauses[c][k];
        if (neighbours[var] != i) { 
          neighbours[var] = i;
          neighbour_list[num_neighbour_list++]=var;
        }
      }
    }
    // store what variables are neighbors in neighbourVar array, terminating with 0
    TEST_NOT_NULL(neighbourVar[i] = (int*)calloc(sizeof(int),num_neighbour_list))
    int* storeptr = neighbourVar[i];
    for (int j=0; j<num_neighbour_list; j++)
      if (neighbour_list[j]!=i)
        *(storeptr++) = neighbour_list[j];
  }
  for (int** aa = abs_clauses; *aa!=0; aa++)
    free(*aa);
  free(abs_clauses);
  free(neighbours);
  free(neighbour_list);
}



// add a unitary clause to the CNF, 
// function clears all occurance and neighbor buffer information
// so these might need to be recomputed.
void Cnf::add_unitary_clause(int unit) {
  assert(unit!=0);
  cc++;
  TEST_NOT_NULL(clauses = (int**)realloc(clauses, sizeof(int*)*(cc+1)))
  clauses[cc]=0;
  TEST_NOT_NULL(cl = (unsigned int*)realloc(cl, sizeof(int)*(cc+1)))
  cl[cc]=0;
  cl[cc-1] = 1;
  TEST_NOT_NULL(clauses[cc-1] = (int*)calloc(sizeof(int),2))

  int var = abs(unit);
  if (var>vc) {
    vc = var;
    free_occurence_and_neighborhood_buffers();
  }
  clauses[cc-1][0] = unit;
  
  // update the occurence buffers (if not reset) with the new unit clause (neighborhood information dosnt change even if exists)
  if ((occurence!=NULL) && (numOccurence!=NULL)) {
    numOccurence[unit+vc]++;
    occurence[unit+vc] = (int*)realloc(occurence[unit+vc],sizeof(int)*(numOccurence[unit+vc]+1));
    occurence[unit+vc][numOccurence[unit+vc]-1] = cc-1;
    occurence[unit+vc][numOccurence[unit+vc]] = 0;
  }
}


// add a clause to the CNF, the input clause must be zero terminated.
// function clears all occurance and neighbor buffer information
// so these might need to be recomputed.
void Cnf::add_clause(int* clause) {
  if (clause == NULL)
    throw BadParameterException("CNF cannot add_clause with NULL");
  // discover the size of the new clause
  int size;
  for (size=0; clause[size]!=0; size++);
  // alter the clauses array
  cc++;
  TEST_NOT_NULL(clauses = (int**)realloc(clauses, sizeof(int*)*(cc+1)))
  clauses[cc]=0;
  TEST_NOT_NULL(cl = (unsigned int*)realloc(cl, sizeof(int)*(cc+1)))
  cl[cc]=0;
  cl[cc-1] = size;
  TEST_NOT_NULL(clauses[cc-1] = (int*)calloc(sizeof(int),size+1))
  // load in new clause
  for (int i=0; i<size; i++) {
    int literal = clause[i];
    int var = abs(literal);
    if (var>vc) {
      vc = var; // increase the variable count
      free_occurence_and_neighborhood_buffers();// trigger occurence/neighborhood buffer reset()
    }
    clauses[cc-1][i] = literal;
  }
  
  // update the occurence buffers with the new clause
  if ((occurence!=NULL) && (numOccurence!=NULL)) {
    for (int i=0; i<size; i++) {
      int unit = clause[i];
      numOccurence[unit+vc]++;
      occurence[unit+vc] = (int*)realloc(occurence[unit+vc],sizeof(int)*(numOccurence[unit+vc]+1));
      occurence[unit+vc][numOccurence[unit+vc]-1] = cc-1;
      occurence[unit+vc][numOccurence[unit+vc]] = 0;
    }
  }
  
  // update neighborhood information
  if (neighbourVar!=NULL) {
    for (int i=0; i<size; i++) {
      int v1 = abs(clause[i]);
      // get size of i's neighborhood
      int neighborhood_size = 0;
      while(neighbourVar[v1][neighborhood_size]!=0) neighborhood_size++;
      vector<int> pushes_for_i;
      pushes_for_i.clear();
      for (int j=0; j<size; j++) {
        if (j!=i) { // for each pairwise variables in the clause
          int v2 = abs(clause[j]);
          //check if variable j is already in i's neighborhood
          bool already = false;
          for (int k=0; neighbourVar[v1][k]!=0; k++) {
            if (neighbourVar[v1][k] == v2) {
              already = true;
              break;
            }
          }
          if (!already) { // if not already, then queue it to add to i's neighborhood
            pushes_for_i.push_back(v2);
          }
        }
      }
      // resize and add new elements to neighborhood
      neighbourVar[v1] = (int*)realloc(neighbourVar[v1],sizeof(int)*(neighborhood_size+pushes_for_i.size()+1));
      neighbourVar[v1][neighborhood_size+pushes_for_i.size()]=0;
      for (int l=0; l<pushes_for_i.size(); l++) {
        neighbourVar[v1][neighborhood_size+l] = pushes_for_i[l];
      }
    }
  }
}


// given another CNF, adjoin that one to this.
void Cnf::join(Cnf* c) {
  if (c == NULL)
    throw BadParameterException("CNF cannot join with NULL CNF");
  int old_cc = cc;
  cc = cc + c->cc;
  TEST_NOT_NULL(clauses = (int**)realloc(clauses, sizeof(int*)*(cc + 1)))
  TEST_NOT_NULL(cl = (unsigned int*)realloc(cl, sizeof(int)*(cc + 1)))
  clauses[cc]=0;
  cl[cc]=0;
  for (int i=0; i<c->cc; i++) {
    TEST_NOT_NULL(clauses[old_cc+i] = (int*)calloc(sizeof(int),c->cl[i]+1))
    cl[old_cc+i] = c->cl[i];
    for (int j=0; j<c->cl[i]; j++) {
      int lit = c->clauses[i][j];
      int var = abs(lit);
      clauses[old_cc+i][j] = lit;
      if (var>vc) {
        vc = var;
        free_occurence_and_neighborhood_buffers();// trigger occurence/neighborhood buffer reset()
      }
    }
  }
  
  // if we have both occurence buffers existant
  if ((occurence!=NULL) && (numOccurence!=NULL)) {
    if ((c->occurence!=NULL) && (c->numOccurence!=NULL)) {
      // for all the variables in the second
      for (int v=1; v<=c->vc; v++) {
        if (c->numOccurence[c->vc+v] >0) { // if there is positive occurence to update
          occurence[vc+v] = (int*)realloc(occurence[vc+v],sizeof(int)*(numOccurence[vc+v]+c->numOccurence[c->vc+v]+1)); // reallocate occurence buffer
          for (int i=0; i<c->numOccurence[c->vc+v]; i++)
            occurence[vc+v][numOccurence[vc+v]+i] = c->occurence[c->vc+v][i]+old_cc; // fill buffer
          occurence[vc+v][numOccurence[vc+v]+c->numOccurence[c->vc+v]] = 0;
          numOccurence[vc+v] += c->numOccurence[c->vc+v];
        }
        if (c->numOccurence[c->vc-v] >0) { // if there is negative occurence to update
          occurence[vc-v] = (int*)realloc(occurence[vc-v],sizeof(int)*(numOccurence[vc-v]+c->numOccurence[c->vc-v]+1)); // reallocate occurence buffer
          for (int i=0; i<c->numOccurence[c->vc-v]; i++)
            occurence[vc-v][numOccurence[vc-v]+i] = c->occurence[c->vc-v][i]+old_cc; // fill buffer
          occurence[vc-v][numOccurence[vc-v]+c->numOccurence[c->vc-v]] = 0;
          numOccurence[vc-v] += c->numOccurence[c->vc-v];
        }
      }
    }
  } else { // safer to erase data than have wrong data or risk calling and update
    free_occurence_and_neighborhood_buffers();
  }
  
  // update neighborhood information
  if ((neighbourVar!=NULL) && (c->neighbourVar!=NULL)) {
    // for each variable in new cnf
    for (int v=1; v<= c->vc; v++) {
      int neighbourhood_size = 0;
      while (neighbourVar[v][neighbourhood_size]!=0) neighbourhood_size++;
      vector<int> new_neighbours;
      new_neighbours.clear();
      for (int k=0; c->neighbourVar[v][k]!=0; k++) { // for each neighbour in the neighbourhood of each variable in the new cnf
        bool found = false;
        for (int i=0; i<neighbourhood_size; i++) { // if the neighbour is not in the new neighbourhood
          if (neighbourVar[v][i] == c->neighbourVar[v][k]) {
            found = true;
            break;
          }
        }
        if (!found) // append it to a queue to add it to the new neighbourhood
          new_neighbours.push_back(c->neighbourVar[v][k]);
      }
      if (new_neighbours.size() > 0) { // if new neighbours for variable v, then append new neighbours
        neighbourVar[v] = (int*)realloc(neighbourVar[v],sizeof(int)*(neighbourhood_size+new_neighbours.size()+1));
        for (int i=0; i<new_neighbours.size(); i++)
          neighbourVar[v][neighbourhood_size+i] = new_neighbours[i];
        neighbourVar[v][neighbourhood_size+new_neighbours.size()] = 0;
      }
    }
  } else { // safer to erase data than have wrong data or risk calling and update
    free_occurence_and_neighborhood_buffers();
  }
}












//-----------------------------------------------
//
//         Private methods
//
//-----------------------------------------------


// recalculate cc,cl,vc from clauses
void Cnf::populate_from_clauses() {
  int length = 0;
  for (int i=0; clauses[i]!=NULL; i++)
    length++;
  cc = length;
  TEST_NOT_NULL(cl = (unsigned int*)calloc(sizeof(unsigned int),cc+1))
  vc = 0;
  for (int i=0; clauses[i]!=NULL; i++) {
    int j=0;
    for (j=0; clauses[i][j]!= 0; j++)
      if (abs(clauses[i][j]) > vc)
        vc = abs(clauses[i][j]);
    cl[i] = j;
  }
}


// delete neighbor and occurance buffer information (insofar as they exist)
void Cnf::free_occurence_and_neighborhood_buffers() {
  if (occurence != NULL) {
    for (int i=0; i<vc*2+1; i++)
      if (occurence[i] != NULL)
        free(occurence[i]);
    free(occurence);
    occurence = NULL;
  }
  if (numOccurence != NULL) {
    free(numOccurence);
    numOccurence = NULL;
  }
  if (neighbourVar != NULL) {
    for (int i = 1; i < vc+1; i++)
      if (neighbourVar[i] != NULL)
        free(neighbourVar[i]);
    free(neighbourVar);
    neighbourVar = NULL;
  }
}






