/*************************
Copyright 2021 Marshall Cliffton

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

#include "MpiBuffer.h"
#include <glog/logging.h>
#include "../mpi_global.h"

// TODO rewrite getClause in a cleaner way, possibly with recursive calls

MpiBuffer::MpiBuffer(MPI_Comm *communicator, int phase, int partnerRank, int inwardClauseTag, int outwardClauseTag)
    : communicator(communicator),
      phase(phase),
      sizeIn(MAX_BUFFER_SIZE),
      sizeOut(MAX_BUFFER_SIZE),
      partnerRank(partnerRank),
      inwardClauseTag(inwardClauseTag),
      outwardClauseTag(outwardClauseTag) {

  inwardClauseRequests[0] = MPI_REQUEST_NULL;
  inwardClauseRequests[1] = MPI_REQUEST_NULL;
  outwardClauseRequests[0] = MPI_REQUEST_NULL;
  outwardClauseRequests[1] = MPI_REQUEST_NULL;

  MPI_Comm_size(*communicator, &numProcesses);
  
  // Double buffers information and marker for which one to use
  bufferIn = 0;
  bufferOut = 0;
  
  TEST_NOT_NULL(buffer = (int*) malloc((sizeIn + PADDING + sizeOut + PADDING) * 2*sizeof(int)))
  buffersIn[0] = &buffer[0] + PADDING;
  buffersIn[1] = buffersIn[0] + sizeIn + PADDING;
  buffersOut[0] = buffersIn[1] + sizeIn + PADDING;
  buffersOut[1] = buffersOut[0] + sizeOut + PADDING;

  // Set up first in and out buffers
  buffersOutNextData[0]   = buffersOutNextData[1] = 0; // where next to put data in the buffer (equivalent to start buffer?)
  buffersOutCycledData[0] = buffersOutCycledData[1] = false;  // has the buffer cycled?

  // setting up the points of the start and end of the buffer to be the first two possitions in the array.
  buffersIn[0][-2] = buffersIn[0][-1] = 0; // start?/end? of cyclic buffer
  buffersIn[1][-2] = buffersIn[1][-1] = 0;

  MPI_Irecv(buffersIn[bufferIn] - PADDING, sizeIn + PADDING, MPI_INT, partnerRank, inwardClauseTag, *communicator, &inwardClauseRequests[bufferIn]);
  bufferIn = 1; // first testAndSwitchBufferIn will set this back to 0
}

inline void cancelRequest(MPI_Request *req) {
  if (*req != MPI_REQUEST_NULL) MPI_Cancel(req);
}

MpiBuffer::~MpiBuffer() {
  free(buffer);

  cancelRequest(&inwardClauseRequests[0]);
  cancelRequest(&inwardClauseRequests[1]);
  cancelRequest(&outwardClauseRequests[0]);
  cancelRequest(&outwardClauseRequests[1]);
}

// Functions for accessing/modifying current and end positions in buffers
inline int MpiBuffer::buffersInNext()     {return buffersIn[bufferIn][-2];}
inline int MpiBuffer::buffersInEnd()      {return buffersIn[bufferIn][-1];}
inline int MpiBuffer::buffersOutNext()    {return buffersOutNextData[bufferOut];}
inline bool MpiBuffer::buffersOutCycled() {return buffersOutCycledData[bufferOut];}


//NOTE: 'next' is the position that I do stuff to next, in the read buffer it is the beginning of the clauses, in the output buffer it is where am i next writing to.

// adding to the buffer beginning with a cycle
inline void MpiBuffer::increaseBuffersInNext(int amount) {
  buffersIn[bufferIn][-2] = (buffersIn[bufferIn][-2]+amount)%sizeIn;
}

// adding to the buffer out with a cycle and a flag
inline void MpiBuffer::increaseBuffersOutNext(int amount) {
  buffersOutNextData[bufferOut] += amount;
  if (buffersOutNextData[bufferOut] >= sizeOut) {
    buffersOutNextData[bufferOut] = buffersOutNextData[bufferOut] % sizeOut;
    buffersOutCycledData[bufferOut] = true;
  }
  assert(buffersOutNextData[bufferOut] < sizeOut);
}

// Get and push public commands
ClauseWithPos* MpiBuffer::getClause() {
  if (buffersInNext() == buffersInEnd()) {
    // no more clauses in bufferIn
    testAndSwitchBufferIn();
////TODO: 
//  if testANDSwitch swaps to another buffer, then it should check to see if there is any clauses to be gotten from that other buffer.
  } else {
    // On a readable buffer with remaining clauses
    ClauseWithPos *clauseWithPos = new ClauseWithPos; // TODO not getting deallocated

    // the beginnig of a clause to be read should not be a zero.
    if (!buffersIn[bufferIn][buffersInNext()]) {
      VLOG(1) << "ERROR, buffersIn at buffersInNext is 0. buffersInNext: " << buffersInNext() << "\n";
      printBuffer(buffersIn[bufferIn], sizeIn);
    }
    assert(buffersIn[bufferIn][buffersInNext()]); // Next should point to a lit

    int lit;

    // loading a clause from the buffer into ClauseWIthPos class
    while ((lit = buffersIn[bufferIn][buffersInNext()])) {
      if (buffersInNext() == buffersInEnd()) {
// TODO: check that this does happen??? and check when it happens
        // Got to end without finding the 0 to complete clause (the next call will recognize that this is the end then go call another buffer)
        delete clauseWithPos;
        return NULL;
// this is discarding 'garbage'. potentially error has occured
      }
      clauseWithPos->clause.push_back(lit);
      increaseBuffersInNext(1);
    }

    // on 0 so move forward one UNLESS AT END
    if (buffersInNext() != buffersInEnd()) {
      increaseBuffersInNext(1);
    }

// I refuse to handle binary and unit clauses (preempting tinisat behavior)
    if (!(clauseWithPos->clause.size() >= 3)) {
      VLOG(1) << "ERROR, clauseWithPos -> clause.size(): " << clauseWithPos->clause.size() << " ,buffersInNext: " << buffersInNext() << "\n";
      printBuffer(buffersIn[bufferIn], sizeIn);
    }
    assert(clauseWithPos->clause.size() >= 3);

// litpoolstorage offset, is adding a costant positive to the positions so that they aren't zero and screw up the reading process
    clauseWithPos->litPoolPos = clauseWithPos->clause.back() - litPoolStorageOffset;
    clauseWithPos->clause.pop_back();
    clauseWithPos->litPool = clauseWithPos->clause.back() - litPoolStorageOffset;
    clauseWithPos->clause.pop_back();

    return clauseWithPos;
  }
  return NULL;
}

// split the clause into two parts and past it into the buffer in two std::copy commands
void MpiBuffer::pushClauseNoAutoSend(int* inClauseWithPosArray, int inClauseWithPosLength) {
  int reqdBufferSize = inClauseWithPosLength + 1;
  if (reqdBufferSize > sizeOut) return; // Too big, can't handle it

  const int endRemainingCapacity = (sizeOut - buffersOutNext());
  const int firstPassSize = std::min(endRemainingCapacity, inClauseWithPosLength);
  const int secondPassSize = inClauseWithPosLength - firstPassSize;

  std::copy(inClauseWithPosArray, inClauseWithPosArray + firstPassSize, buffersOut[bufferOut] + buffersOutNext());
  increaseBuffersOutNext(firstPassSize);

  if (secondPassSize != 0) {
    assert(buffersOutNext() == 0);

    std::copy(inClauseWithPosArray + firstPassSize, inClauseWithPosArray + inClauseWithPosLength, buffersOut[bufferOut]);
    increaseBuffersOutNext(secondPassSize);
  }

  // add a constant to positions in the litpool, as they may be 0
  buffersOut[bufferOut][(buffersOutNext() - 1 + sizeOut) % sizeOut] += litPoolStorageOffset;
  buffersOut[bufferOut][(buffersOutNext() - 2 + sizeOut) % sizeOut] += litPoolStorageOffset;
  
  // add zero
  buffersOut[bufferOut][buffersOutNext()] = 0;
  increaseBuffersOutNext(1);
}
  
void MpiBuffer::pushClause(int* inClauseWithPosArray, int inClauseWithPosLength) {
  testAndSwitchBufferOut();  

  // Actually push the clause now
  pushClauseNoAutoSend(inClauseWithPosArray, inClauseWithPosLength);
}

inline bool MpiBuffer::testAndSwitchBufferIn() {
  int completedFlag;
  MPI_Test(&inwardClauseRequests[(bufferIn+1)%2], &completedFlag, MPI_STATUS_IGNORE);
  if (completedFlag) {
    MPI_Irecv(buffersIn[bufferIn]-PADDING, sizeIn + PADDING, MPI_INT, partnerRank, inwardClauseTag, *communicator, &inwardClauseRequests[bufferIn]);
    bufferIn = (bufferIn + 1) % 2;
    int phaseReceived = buffersIn[bufferIn][-PADDING];
    if (phaseReceived != phase) {
      VLOG(4) << "Received message from phase " << phaseReceived << " when my phase is " << phase << std::endl;
      // Received message from a different phase.
      // Ignore received clauses by setting beginning=end=0
      buffersIn[bufferIn][-2] = 0;
      buffersIn[bufferIn][-1] = 0;
    }
  }
  return completedFlag;
}

bool MpiBuffer::readyToSend() {
  return testAndSwitchBufferOut();
}

// if there is an alternate buffer free, then we switch to it and we send the current one.
inline bool MpiBuffer::testAndSwitchBufferOut() {
  // Check if Out request for other Out buffer has been successful
  int completedFlag;
  MPI_Test(&outwardClauseRequests[(bufferOut+1)%2], &completedFlag, MPI_STATUS_IGNORE);
  if (completedFlag) {
    sendBufferOut();
    bufferOut = (bufferOut + 1) % 2;
    // clear buffer out
    buffersOutNextData[bufferOut] = 0;
    buffersOutCycledData[bufferOut] = false;
  }
  return completedFlag;
}

bool MpiBuffer::getWillThisClauseEverFit(int clauseLength) {
  return clauseLength < sizeIn; // less than as an x-1 sized clause needs the padding of a 0, and will take x space
}

int MpiBuffer::getRemainingOutSpace() {
  if (buffersOutCycledData[bufferOut]) return 0;
  return sizeOut - buffersOutNext();
}

inline int MpiBuffer::getClauseLength(int* inClauseArray) {
  int i = 0;
  while(inClauseArray[i]) i++;
  return i;
}

inline void MpiBuffer::sendBufferOut() {
  // If buffer has not been touched don't send
  if (buffersOutNext() == 0 && buffersOutCycled() == false) return;
  
  // Work out beginning and end of buffer to send
  int end = buffersOutNext() - 1;
  if (end == -1) end += sizeOut;
  int i = buffersOutNext();
  int beginning;
  if (buffersOutCycled()) {
    while(buffersOut[bufferOut][i]) i = (i + 1) % sizeOut; // get i to the 0 at the end of the (probably) half overridden clause
    beginning = i+1;
  } else {
    beginning = 0;
  }

  assert(!buffersOut[bufferOut][end]);
  
  // put beginnning and end at the end of the big buffer to send all as one
  buffersOut[bufferOut][-3] = phase;
  buffersOut[bufferOut][-2] = beginning;
  buffersOut[bufferOut][-1] = end;

  int sendLength;
  if (end > beginning) sendLength = end + PADDING + 1;
  else                 sendLength = sizeOut + PADDING + 1;
  if (sendLength > MAX_BUFFER_SIZE) VLOG(0) << "sendLength = " << sendLength;
 
  MPI_Isend(buffersOut[bufferOut]-PADDING, sendLength, MPI_INT, partnerRank, outwardClauseTag, *communicator, &outwardClauseRequests[bufferOut]);
}

// Printing
void MpiBuffer::printBuffer(int *buffer, int size) {
  VLOG(1) << "###########\n"
      << "\t0\t1\t2\t3\t4\t5\t6\t7\t8\t9\n";
  for (int i = 0; i < size; i++) {
    if (i % 10 == 0)
      VLOG(1) << i << "\t";
    VLOG(1) << buffer[i] << "\t";
    if (i % 10 == 9)
      VLOG(1) << "\n";
  }
  VLOG(1) << "\n###########" << std::endl;
}

void MpiBuffer::printClause(int *inClauseArray, int inClauseLength = -1) {
  if (inClauseLength != -1) {
    for (int i = 0; i < inClauseLength; i++) {
      VLOG(1) << inClauseArray[i] << " ";
    }
    VLOG(1) << "0\n";
  } else {
    for (int i = 0; inClauseArray[i]; i++) {
      VLOG(1) << inClauseArray[i] << " ";
    }
    VLOG(1) << "0\n";
  }
}
