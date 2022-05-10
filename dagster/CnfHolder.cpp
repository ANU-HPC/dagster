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


#include "CnfHolder.h"
#include "exceptions.h"
#include <string>
#include "mpi_global.h"
#include <glog/logging.h>

using namespace std;


// constructor to be called when intentioned to send to separate files (for cnf_directory not NULL)
// if cnf_directory is NULL, then in-memory CNF storage is initiated
CnfHolder::CnfHolder(Dag* dag, char *cnf_directory, char* cnf_filename, int from_files) {
  this->dag = dag;
  if (cnf_directory == NULL) {
    this->from_files = from_files;
  } else {
    this->from_files = 1;
  }
  this->cnf_directory = cnf_directory;
  this->cnf_filename = cnf_filename;
  this->max_vc = 0;
  TEST_NOT_NULL(this->cnfs = (Cnf**)calloc(sizeof(Cnf*),dag->no_nodes))
  this->loaded_cnf = -1;
  this->master_cnf = NULL;
}


// constructor to be called when intentioned to send to separate files (for cnf_directory not NULL)
// if cnf_directory is NULL, then in-memory CNF storage is initiated
CnfHolder::CnfHolder(Dag* dag, char *cnf_directory, char* cnf_filename) {
  this->dag = dag;
  if (cnf_directory == NULL) {
    this->from_files = 0;
  } else {
    this->from_files = 1;
  }
  this->cnf_directory = cnf_directory;
  this->cnf_filename = cnf_filename;
  this->max_vc = 0;
  TEST_NOT_NULL(this->cnfs = (Cnf**)calloc(sizeof(Cnf*),dag->no_nodes))
  this->loaded_cnf = -1;
  this->master_cnf = NULL;
}

// CnfHolder defaults to in-memory CNF storage
CnfHolder::CnfHolder(Dag* dag, char* cnf_filename) : CnfHolder::CnfHolder(dag,NULL,cnf_filename) {}

// general destructor
CnfHolder::~CnfHolder() {
  for (int i=0; i<dag->no_nodes; i++) {
    if (cnfs[i] != NULL) {
      delete cnfs[i];
      cnfs[i] = NULL;
    }
  }
  delete this->master_cnf;
  free(this->cnfs);
}

// a function to get the 'filename' of a respective Dag node Cnf.
char* CnfHolder::get_Cnf_filename(int node) {
  str = cnf_directory;
  str += "/cnf_node";
  str += std::to_string(node);
  str += ".txt";
  return (char*)(str.c_str());
}


// accesses a CNF bassed on its node index
// if file_storage is used then load from the respective file
// otherwise if in-memory storage is used then load from memory
Cnf* CnfHolder::get_Cnf(int node) {
  if ((node<0) || (node>=dag->no_nodes))
    throw BadParameterException("CnfHolder called with node not in dag");
  if (from_files == 1) { // if from_files is true, then load the CNF from the respective file into cnfs[0]
    if (node == loaded_cnf)
      return cnfs[0];
    if (cnfs[0] != NULL)
      delete cnfs[0];
    // load in cnf from file
    cnfs[0] = new Cnf(get_Cnf_filename(node));
    loaded_cnf = node;
    return cnfs[0];
  } else if (from_files==2) { // dynamically create cnfs in memory
    if (node == loaded_cnf)
      return cnfs[0];
    if (cnfs[0] != NULL)
      delete cnfs[0];
    // load in cnf from file
    cnfs[0] = new Cnf(this->master_cnf, dag->clause_indices_for_each_node[node]);
    loaded_cnf = node;
    return cnfs[0];
  } else { // else directly pass the pointer for in-memory CNFs.
    if (cnfs[node]==NULL)
      throw BadParameterException("CnfHolder called prior to generate_decomposition");
    return cnfs[node];
  }
}


// int split_CNF(cnf_filename, indices)
//
// A big one-pass function to scan through the CNF file given by <cnf_filename> and create separate CNF files with clauses pointed to
// by the indices given in the <indices> vector.
// returns the maximum variable index found in the process of splitting the files up
// (NOTE: this function reminiscient of cnf.cpp's CNF() constructor)
//
// NOTE: potential bug, if header = 'p cnf 0 0'
int CnfHolder::split_CNF(char* cnf_filename, vector<RangeSet> &indices) {
  int nodes = indices.size();
  vector<std::deque<std::pair<int,int>>::iterator> node_index_pairs;
  vector<int> clause_counts;
  vector<int> variable_counts;
  vector<FILE*> file_pointers;
  clause_counts.resize(nodes);
  variable_counts.resize(nodes);
  file_pointers.resize(nodes);
  node_index_pairs.resize(nodes);
  
  // for each sub CNF (equal to the number of nodes) we are going to create we need to 
  // dump some witespace to be overwritten by the header, and setup initial variable values
  FILE *ifp;
  TEST_NOT_NULL(ifp = fopen(cnf_filename, "r"));
  for (int i=0; i<nodes; i++) {
    TEST_NOT_NULL(file_pointers[i] = fopen(get_Cnf_filename(i), "w"));
    fprintf(file_pointers[i],"                                             \n"); // dump a large amount of whitespace for overwriting with header info
    clause_counts[i]=0;
    variable_counts[i]=0;
    node_index_pairs[i] = indices[i].buffer.begin();
  }
  
  // create a buffer to read in lines
  char line[100000];
  size_t len = 100000;
  char c;
  
  // create a buffer to parse read integers from the read lines
  int max_clause_len = 1024, *literals;
  TEST_NOT_NULL(literals = (int *) malloc(max_clause_len * sizeof(int)))
  
  // search for and read the header
  int header_vc, header_cc;
  while((c=getc(ifp)) != EOF){ 
    if (isspace(c)) continue; else ungetc(c,ifp);
    fgets(line, len, ifp);
    if (c=='p'){
      if(sscanf(line, "p cnf %d %d", &header_vc, &header_cc) == 2)
        break;
      else
        throw ParsingException(" Invalid CNF file\n");
    }
  }

  int clause_index = 0;
  // for each line
  while((c=getc(ifp)) != EOF){
    if (isspace(c)) continue; else ungetc(c,ifp);
    // search for the first non-whitespace character
    if ((c=='-') || isdigit(c)) {
      int literal_input;
      int j=-1;
      int vc = 0;
      do { // scan the line into the literals buffer one character at a time, until the zero is scanned
        j++;
        int literal_input_count = fscanf(ifp, "%d", &literal_input);
        if (literal_input_count == 0)
          throw ParsingException("Invalid CNF file - cnf lines must involve digits and terminate with zero\n");
        if (j == max_clause_len) { // extend integer buffer if nessisary
          max_clause_len *= 2;
          TEST_NOT_NULL(literals = (int *) realloc(literals, max_clause_len * sizeof(int)))
        }
        // store read integer in integer buffer
        literals[j] = literal_input;
        if (abs(literal_input) > vc) // updating max variable 'vc'
          vc = abs(literal_input);
      } while (literal_input != 0);
      
      for (int i=0; i<nodes; i++) {
        // update the range pair [a,b] under consideration for each node such that the clause_index <= b
        while ((node_index_pairs[i] != indices[i].buffer.end()) && 
               (clause_index > (*(node_index_pairs[i])).second))
          node_index_pairs[i]++;
        // if not past the end of the respective indicies and for range pair [a,b] that clause_index >= a
        if ((node_index_pairs[i] != indices[i].buffer.end()) &&
            (clause_index >= (*(node_index_pairs[i])).first)) {
          // print the clause
          for (int* literal = literals; *literal; literal++)
            fprintf(file_pointers[i], "%i ",*literal);
          fprintf(file_pointers[i], "0\n");
          // update the variable count and clause count for the node CNF
          if (vc>variable_counts[i])
            variable_counts[i] = vc;
          clause_counts[i]++;
        }
      }
      clause_index++;
      // debug loading message (most relevent for super large CNF parsing)
      if ((clause_index % 10000)==0)
        VLOG(3) << "parsed " << clause_index << " of " << header_cc << " clauses. ie: " << clause_index*100.0/header_cc << " percent.";
    }
    // get a new line
    fgets(line, len, ifp);
  }
  free(literals);
  fclose(ifp);
  // update the headers of the split CNF files
  for (int i=0; i<nodes; i++) {
    rewind(file_pointers[i]);
    fprintf(file_pointers[i],"p cnf %i %i",variable_counts[i], clause_counts[i]);
    fclose(file_pointers[i]);
  }
  return header_vc;
}



// int pseudo_split_CNF(cnf_filename, indices)
//
// mimics the behavior of the split_CNF() function, except does not actually output the split files.
// This is a big one-pass function to scan through the CNF file given by <cnf_filename> and mimic the process of creating separate CNF files with clauses pointed to
// by the indices given in the <indices> vector.
// returns the maximum variable index found in the process of splitting the files up (which is the primary purpose of the function)
//
// NOTE: potential bug, if header = 'p cnf 0 0'
int CnfHolder::pseudo_split_CNF(char* cnf_filename) {
  FILE *ifp;
  TEST_NOT_NULL(ifp = fopen(cnf_filename, "r"));
  
  // create a buffer to read in lines
  char line[100000];
  size_t len = 100000;
  char c;
  
  // search for and read the header
  int header_vc, header_cc;
  while((c=getc(ifp)) != EOF){ 
    if (isspace(c)) continue; else ungetc(c,ifp);
    fgets(line, len, ifp);
    if (c=='p'){
      if(sscanf(line, "p cnf %d %d", &header_vc, &header_cc) == 2)
        break;
      else
        throw ParsingException(" Invalid CNF file\n");
    }
  }

  fclose(ifp);
  return header_vc;
}





/// LOAD in the filesystem module for removing/creating directories in generate_decomposition() function.
#if __cplusplus > 201703
#include<filesystem>
#elif __cplusplus >= 201103L
#include <experimental/filesystem> // gcc7 (Ubuntu 18.04)
#endif

// splits the DAG cnf into sub-CNFs, for in-memory storage this method needs to be called prior to access for every worker
// if file storage is used, then this method only needs to be called at-least once before access to create the respective files
void CnfHolder::generate_decomposition() {
  if (from_files == 1) {
    std::experimental::filesystem::remove_all(cnf_directory);
    if (!std::experimental::filesystem::create_directory(cnf_directory))
      ParsingException("failed to create directory for CNF decomposition");
    VLOG(3) << "Splitting CNF into files ";
    max_vc = split_CNF(cnf_filename, dag->clause_indices_for_each_node);
    VLOG(3) << "Done Splitting CNF into files ";
  } else if (from_files == 2) {
    this->master_cnf = new Cnf(cnf_filename);
    generate_pseudo_decomposition();
  } else {
    this->master_cnf = new Cnf(cnf_filename);
    for (int node=0; node<dag->no_nodes; node++) {
      VLOG(3) << "Loading CNF for node " << node << " of " << dag->no_nodes;
      Cnf* cnf = new Cnf(this->master_cnf, dag->clause_indices_for_each_node[node]);
      if (cnf->vc > max_vc) {
        max_vc = cnf->vc;
      }
      cnfs[node] = cnf;
    }
  }
}

// same as generate_decomposition, except only sets max_vc
void CnfHolder::generate_pseudo_decomposition() {
  max_vc = pseudo_split_CNF(cnf_filename);
}


// takes a message and gets the respective Cnf of the 'to' field, adds unitary clauses for the assignments of the messages and appends additional clauses
// note the returned Cnf must be deleted for memory safety
Cnf* CnfHolder::compile_Cnf_from_Message(Message* m) {
  if (m==NULL)
    throw ConsistencyException("cannot compile CNF from null message");
  Cnf* cnf = new Cnf(get_Cnf(m->to));
  // as well as loading a whole bunch of unitary clauses, from the message given to this node
  for (int i = 0; i < m->assignments.size(); i++)
    cnf->add_unitary_clause(m->assignments[i]);
  // and adding all the additional clauses into the cnf
  if (m->additional_clauses != NULL)
    cnf->join(m->additional_clauses);
  return cnf;
}
