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


#ifndef _CNF_HOLDER
#define _CNF_HOLDER

#include "Cnf.h"
#include "Dag.h"
#include "Message.h"

// A class that holds accessors to Cnfs for each node
// initialised by a referenced Dag.
// if a cnf_directory is specified (ie not NULL) then the parts of the CNF are broken down into separate files
// which are repeatedly loaded
// otherwise all CnFs are held in immediate memory
class CnfHolder {
public:
  CnfHolder(Dag* dag, char *cnf_directory, char *cnf_filename, int from_files); // constructor to be called when intentioned to send to separate files
  CnfHolder(Dag* dag, char *cnf_directory, char *cnf_filename); // constructor to be called when intentioned to send to separate files
  CnfHolder(Dag* dag, char* cnf_filename);  // constructor to be called when in-memory CNFs are desired
  void generate_decomposition(); // generates a decomposition, for file CNF storage, this only needs to be called once, otherwise once for each worker.
  void generate_pseudo_decomposition();
  ~CnfHolder();  // destructor
  Cnf* get_Cnf(int node); // general accessor, returns a reference to respective node CNF, CNF does not need to be freed.
  char* get_Cnf_filename(int node);
  Cnf* compile_Cnf_from_Message(Message* m); // takes a message and gets the respective Cnf of the 'to' field, adds unitary clauses for the assignments of the messages and appends additional clauses
  int max_vc; // the maximum variable count from any of the cnfs
  int split_CNF(char* cnf_filename, vector<RangeSet> &vec_indices);
  int pseudo_split_CNF(char* cnf_filename);
  Dag* dag;
private:
  int from_files;
  char* cnf_directory;
  char* cnf_filename;
  Cnf** cnfs;
  int loaded_cnf;
  std::string str;
  Cnf* master_cnf;
};
#endif
