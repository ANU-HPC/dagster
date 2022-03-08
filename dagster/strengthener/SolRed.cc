//
//     MiniRed/GlucoRed
//
//     Siert Wieringa
//     siert.wieringa@aalto.fi
// (c) Aalto University 2012/2013
//
//
#include"SolRed.h"

#ifdef MINIRED
using namespace MiniRed;
#elif defined GLUCORED
using namespace GlucoRed;
#endif

SolRed::SolRed()
    : reducer_backtracks             (0)
    , reducer_backtracks_tozero      (0)
    , reducer_backtrack_levels       (0)
    , reducer_backtrack_level_before (0)

    , reducer_in                     (0)
    , reducer_in_lits                (0)
    , reducer_out                    (0)
    , reducer_out_lits               (0)
    , reducer_notout_lits            (0)
    , workset_in                     (0)
    , workset_in_lits                (0)
    , workset_deleted                (0)
    , workset_deleted_lits           (0)

    , nhead                          (0)
    , offset                         (0)
    , reducer                        () // Instance of Reducer which forms the reducer
{
}

SolRed::~SolRed() {
  // TODO Review what is actually needed here
}


// Overloaded 'solve_' function, will handle solver and reducer
lbool SolRed::solve_() {
  return l_False;
}

// Give clause 'c' to the reducer, 'metric' represents some quality measure of the clause for sorting work set
bool SolRed::submitToReducer(const vec<Lit>& c, int metric) {
  return 0;
}



void SolRed::runReducingLoop(MPI_Comm communicator, int phase, MPI_Request *kill_req) {

  // Working variables for MPI
  // MPI constants TODO change in end product
  MPI_Status tinisatToReducerClauseStatus;
  MPI_Status tinisatToReducerRequestStatus;
  const int tinisatRank = 0;
  const int reducerRank = 1;
  const int tinisatToReducerClauseTag = 0;
  const int reducerToTinisatClauseTag = 1;

  // Set up MPI
  MpiBuffer* mpiBuffer = new MpiBuffer(&(communicator), phase, tinisatRank, tinisatToReducerClauseTag, reducerToTinisatClauseTag);

  vec<Lit> *lits = NULL;
  int* arrayClause;
  vec<Lit>* vectorClause;
  ClauseWithPos* clauseWithPos;

  // Set up reducer
  if (okay()) {
    copyProblem(reducer, 0);
    int completed = 0;
    while (!completed) {
      // Get new clause
      clauseWithPos = mpiBuffer->getClause();
      if (clauseWithPos != NULL) {
        // convert from external to internal representations of literals
        lits = new vec<Lit>;
        while (clauseWithPos->clause.size() != 0) {
          lits->push(getLitFromInteger(clauseWithPos->clause.back())); // TODO reverses order, bad?
          clauseWithPos->clause.pop_back();
        }
        // size and statistics
        const int sz = lits->size();
        reducer_in++;
        reducer_in_lits += sz;
        // try reduce
        if (reducer.reduce(*lits)) {
          reducer_out++;
          reducer_out_lits += lits->size();
          // Was able to reduce, add to be sent back
          int outArrayClauseWithPosLength = lits->size() + 2;
          int *outArrayClauseWithPos = new int[outArrayClauseWithPosLength];
          // populate array to send back
          for (int i = 0; i < lits->size(); i++)
            outArrayClauseWithPos[i] = getIntegerFromLit((*lits)[i]);
          outArrayClauseWithPos[lits->size()] = clauseWithPos->litPool;
          outArrayClauseWithPos[lits->size() + 1] = clauseWithPos->litPoolPos;
          mpiBuffer->pushClause(outArrayClauseWithPos, outArrayClauseWithPosLength);
          delete[] outArrayClauseWithPos;
        } else
          reducer_notout_lits += sz;
        delete lits;
        delete clauseWithPos;
        // Reducer derived unsat, stop everything
        if (!reducer.okay()) {
          //fprintf(stderr, "REDUCER DERIVED UNSAT\n"); // TODO deal with this situation accordingly
          MPI_Wait(kill_req, MPI_STATUS_IGNORE);
          break;
        }
      }
      MPI_Test(kill_req, &completed, MPI_STATUS_IGNORE);
    }
  } else {
    MPI_Wait(kill_req, MPI_STATUS_IGNORE);
  }
  delete mpiBuffer;
}
