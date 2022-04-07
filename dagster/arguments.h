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


#ifndef _ARGUMENTS_H_
#define _ARGUMENTS_H_

#include <string>

struct Arguments {
  // Filename for the file containing the DIMACS CNF problem formula.
  char *cnf_filename;
  
  // Filename describing how to decompose the proof search when solving the problem at \member{cnf_filename}
  char *dag_filename;
  
  // Filename for where the solutions get outputted to
  const char *output_filename;

  // the directory name where CNF partials will be stored, NULL if no directory, and storage is in memory
  char *cnf_directory;
  
  // Filename describing describing what checkpoint to load initially
  char *checkpoint_filename;
  
  // integer of seconds between every checkpoint, 0=checkpoint_generation_disabled
  int checkpoint_frequency;

  // Scheme the (dynamic) local search uses to communicate variable and value choices to the CDCL search.
  std::string advise_scheme;

  // Scheme that we use to combile BDDs into CNF.
  //
  // CONTEXT: When a message is transmitted on an arc, then we do not
  // want to generate or transmit that message a second time. We use a
  // BDD to represent the set of messages that have been transmitted
  // on an DAG arc. When a complete search is executed for the problem
  // posed at a DAG node, we add a constraint "do not generate a
  // model that would transmit a message that we have seen before".
  std::string BDD_compilation_scheme;

  // Should the local search heuristic guide use clause weights?
  int dynamic_local_search;

  // Should the CDCL searches be using local searches as a heuristic guide? 0=no, 1=yes
  int mode;

  // If the CDCL searches are using a local search guide, how many decisions should they make between communications with the local search?
  int decision_interval;

  // If the CDCL searches are using a local search guide, how many assignments should they ask for, when they ask for advise?
  int suggestion_size;
  
  // When the CDCL searches are able to appeal to a local search for
  // variable-selection and assignment advise, they usually have
  // access to a portfolio of variable-selection and value-selection
  // approaches. Here, we specify the order in which to seek advise.
  std::string heuristic_rotation_scheme;

  // If this number is greater than 0, then the CDCL searches will
  // have \member{novelty_number} local searches processes working on
  // providing heuristic information.
  int novelty_number;

  // If this number is one then the master will use BDDmaster
  // otherwise use SettableMaster
  int master_sub_mode;

  // flag whether Dagster should exit after finding the first solution to a terminal node
  int ENUMERATE_SOLUTIONS;
  
  // flag whether dagster should generate depth first on the dag of breadth first
  bool BREADTH_FIRST_NODE_ALLOCATIONS; 
  
  
  // the number of solutions a CDCL will discover before asking master for a possible reassignment
  int sat_solution_interrupt;
  // the number of decisions that the CDCL will make before asking master for a possible reassignment
  int sat_reporting_time;
  // the number decsions that the CDCL will make before checking for a solution from gnovelties
  int gnovelty_solution_checking_time;
  
  // flag to set trimming of variables everytime tinisat has a solution pass through.
  int solution_trimming;
  
  // flag to set if tinisat CDCL should do restarts or not
  int tinisat_restarting;

  //opportunity modulo in the geometric restarting scheme (0 is default, = off)
  int opportunity_modulo;
  //discount factor in the geometric restarting scheme (default is 0.95, only applied if opportunity_modulo is nonzero)
  double discount_factor;


  Arguments();
  
  void load(int argc, char **argv);
};

#endif
