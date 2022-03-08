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

#ifndef STATE_HH
#define STATE_HH

#include <stdint.h>
#include <assert.h>
#include "../mpi_global.h"

class State {
public:	
  /*Initially nothing is true (all bits in \member{data} are
   * false). \argument{size} is the number of propositions that
   * this state is required to keep the truth value of.*/
  State(unsigned int size) {
    numPropositions = size;
    TEST_NOT_NULL(data = (uint8_t*)calloc(sizeof(uint8_t),size))
  }

  //standard destructor
  ~State() {
    free(data);
  }

  // resize function
  int resize(int size) {
    TEST_NOT_NULL(data = (uint8_t*)realloc(data,sizeof(uint8_t)*size))
    for (int i=numPropositions; i<size; i++)
      data[i] = 0;
    numPropositions = size;
    return (data==NULL);
  }

  //Change individual (at index \argument{uing}) bits of the state representation (\member{data}).*/
  inline void flipOn(unsigned int index) {
    //assert(index < numPropositions);
    data[index] = 1;
  }

  inline void flipOff(unsigned int index) {
    //assert(index < numPropositions);
    data[index] = 0;
  }

  inline void flip(unsigned int index) {
    //assert(index < numPropositions);
    data[index] = data[index] ^ 1;
  }

  inline bool isTrue(unsigned int index) {
    //assert(index < numPropositions);
    return data[index];
  }

  //Number of propositions in the state description.
  inline unsigned int getNumPropositions() {return numPropositions;};

  void randomise() {
    for (int i = 0; i < numPropositions; i++) {
      if (random()%2) {
        flipOn(i);
      } else {
        flipOff(i);
      }
    }
  }

  void print() {
    for (int i=0; i<numPropositions; i++)
      printf("%i ",data[i]);
    printf("\n");
  }

private:
    unsigned int numPropositions;
    //binary vector, each bit represents the truth value of a proposition.
    uint8_t* data;
};

#endif
