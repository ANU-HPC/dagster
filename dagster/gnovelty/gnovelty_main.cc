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


#include <cassert>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <glog/logging.h>
#include <mpi.h>

#include "../CnfHolder.h"
#include "../Dag.h"
#include "../mpi_global.h"
#include "../utilities.h"
#include "gnovelty.hh"

using namespace std;

extern CnfHolder *cnf_holder;
const int CHECK_PREFIX_STEPS = 80;
extern int world_rank;

int gnovelty_main(MPI_Comm *communicator, int suggestionSize, const string &advise_scheme, int dynamic_local_search) {
  int walkProb = 1;
  int adaptFlag = 1;
  VLOG(3) <<"advise_scheme="<<advise_scheme<<endl;
  bool using__ghost_suggestions = (strcmp(advise_scheme.c_str(), "ghosts") == 0);
  srandom(genRandomSeed());
  VLOG(3) <<"using__ghost_suggestions="<<using__ghost_suggestions<<endl;
  int phase;

  while (true) {
    // Get a new CNF file from SatHandler
    VLOG(3) << " gnovelty " << world_rank << " about to recieve CNF";
    int fname_length;
    MPI_Status mpi_status;
    MPI_Recv(&fname_length, 1 /*Number of ints*/, MPI_INT, 0 /*CDCL process ID*/, CNF_FILENAME_LENGTH_TAG, *communicator, &mpi_status);

    if (fname_length == -1)
      break;
    assert(fname_length > 0);
    int *buffer;
    TEST_NOT_NULL(buffer = (int *)calloc(sizeof(int), fname_length))
    MPI_Recv(buffer, fname_length, MPI_INT, 0, CNF_FILENAME_TAG, *communicator, &mpi_status);
    phase = buffer[fname_length - 1];

    Message *m = new Message(buffer);
    cnf_holder->get_Cnf(m->to)->compute_variable_neighborhoods(); // compute variable neighborhoods on the root CNF
    if (m->additional_clauses!=NULL)
      m->additional_clauses->compute_variable_neighborhoods__DEPRECATED_2(); // and compute variable neighborhoods on the additional clauses, such that compile_cnf_from_message joins the neighborhood info
    Cnf *cnf = cnf_holder->compile_Cnf_from_Message(m);

    Gnovelty *gnovelty;
    if (dynamic_local_search)
      gnovelty = new Gnovelty_updateClauseWeights_Linear(cnf, 0, suggestionSize, 30);
    else
      gnovelty = new Gnovelty_updateClauseWeights_NULL(cnf, 0, suggestionSize);
    free(buffer); // The CNF instances has now been initialised for the purpose of search.

    // We are using an MPI window to keep a record of the "suggestions"
    // that we are sending back to the CDCL search. After this call, the
    // only variable to keep an eye on is \local{suggestions}.
    int *suggestions;
    MPI_Win window;
    VLOG(3) << " gnovelty " << world_rank << " about to win_allocate";
    auto result = MPI_Win_allocate(suggestionSize * sizeof(int), sizeof(int), MPI_INFO_NULL, *communicator, &suggestions, &window);
    if (result != MPI_SUCCESS)
      cerr << "UNRECOVERABLE ERROR :: cannot allocate window from sls";
    VLOG(3) << " gnovelty " << world_rank << " success win_allocate";
    for (int i = 0; i < suggestionSize; i++) // Initially we have no heuristic information to share with the CDCL procedure.
      suggestions[i] = 0;

    // The gnovelty receives a prefix, which is a set of assignments
    // that it cannot change. So, the local search that is undertaken
    // is "local", in the sense that we are trying to find a
    // specifiable solution that is consistent with this prefix.
    int *prefix;
    prefix = new int[cnf->vc];

    // Ask for the prefix. This is a list of non-zero signed integers, each has absolute value indicating the variable. So, it's a list of literals.
    MPI_Request prefixRequest;
    MPI_Recv_init(prefix, cnf->vc, MPI_INT, 0, PREFIX_TAG, *communicator, &prefixRequest);
    MPI_Start(&prefixRequest);

    // The _main_ loop. Iteratively flip the assignment of one
    // variable until in aggregate you end up with a satisfying
    // assignment, or not.
    // -------------------------------------------------------------
    int steps = 0;
    while (true) {
      int found_solution = 0;
      while ((++steps % CHECK_PREFIX_STEPS) != 0 && !found_solution) {
        found_solution = gnovelty->step_newStyle(walkProb, adaptFlag);
      }
      VLOG(5) << " gnovelty " << world_rank << " done stepping " << found_solution;
      if (using__ghost_suggestions)
        gnovelty->loadSuggestion_ghost(suggestions, suggestionSize);
      else
        gnovelty->loadSuggestion(suggestions, suggestionSize);

      // Communicate with CDCL procedure
      bool received_prefix = false;
      int incoming = 0;
      int prefix_length;
      MPI_Status prefixStatus;
      MPI_Test(&prefixRequest, &incoming, &prefixStatus); // test for a message telling us we need to change the "prefix". That is, change the set of fixed assignments that are locked in.
      // receive all incoming prefix messages and use only the most recent
      int dup = 0;
      while (incoming) {
        dup++;
        received_prefix = true;
        MPI_Get_count(&prefixStatus, MPI_INT, &prefix_length);
        if (prefix_length == 0)
          break;
        MPI_Start(&prefixRequest);
        MPI_Test(&prefixRequest, &incoming, &prefixStatus);
      }
      VLOG(5) << " gnovelty " << world_rank << " recieved prefix " << received_prefix;
      if (received_prefix) { // New advice wrt assignment constraint has been communicated. We have a new, possibly different, prefix to think about.
        VLOG(3) << "received " << dup << " prefix messages, latest prefix_length = " << prefix_length;
        if (prefix_length == 0) // prefix length zero indicates reset to receive new CNF
          break;
      }
      if (found_solution) {
        // need to send current solution before loading new prefix
        //int *solution = gnovelty->processSolution(); // add it as a nogood
        int *solution;
        if (cnf_holder->dag->forward_connections[m->to].size() == 0) { // if message is to terminal node then reporting variables is what we care about
          solution = gnovelty->processSolution(cnf_holder->dag->reporting); // add it as a nogood
        } else {
          solution = gnovelty->processSolution(cnf_holder->dag->amalgamated_forward_connection_literals[m->to]); // add it as a nogood
        }
        solution = (int *)realloc(solution, sizeof(int) * (cnf->vc + 2));
        solution[cnf->vc] = 0;
        solution[cnf->vc+1] = phase;
        MPI_Send(solution, cnf->vc + 2, MPI_INT, 0, SLS_SOLUTION_TAG, *communicator);
        if (VLOG_IS_ON(3)) {
          ostringstream os;
          os << "gnovelty found solution:";
          for (int i = 0; solution[i]!=0; i++) {
            os << " " << solution[i];
          }
          VLOG(3) << os.str();
        }
        free(solution);
      }
      if (received_prefix) {
        if (gnovelty->loadPrefix(prefix, prefix_length)==1) {
          std::stringstream ss;
          for(int i = 0; i < prefix_length && prefix[i] != 0; i++) {
            ss << prefix[i] << " ";
          }
          VLOG(3) << "WARNING: received BAD prefix messages for message: " << *m << ", prefix is " << ss.str();
        }
        steps = 0;
      }
    }
    MPI_Request_free(&prefixRequest);
    delete[] prefix;

    MPI_Win_free(&window);
    delete gnovelty;
    delete cnf;
    delete m;
  }
  return 0;
}

