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

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <cerrno>
#include <time.h>

#include "../../utilities.cpp"
#include "../../Cnf.cpp"
#include "../ReversableIntegerMap.h"
#include "../ReversableIntegerMap.cc"
#include "../gnovelty.cc"

using namespace std;
clock_t tStart;
#define CLOCK_CHECKS 1000

void run(void* filename, int seconds, int search_mode) {
  srandom(genRandomSeed());
  Cnf* cnf = new Cnf((const char*)filename);
  Gnovelty *gnovelty = new Gnovelty_updateClauseWeights_NULL(cnf, 0, 5, search_mode);
  int ticks = 0;
  while (true) {
    while (gnovelty->step(1,true) == 0) {
      if (ticks >= CLOCK_CHECKS) { 
        ticks = 0;
        if ((double)(clock() - tStart)/CLOCKS_PER_SEC > seconds) {
          printf("\nTime taken: %.5fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
          printf("flipped %li\n",gnovelty->flip);
          delete gnovelty;
          delete cnf;
          return;
        }
      }
      ticks++;
    }
    int* solution = gnovelty->processSolution();
    for (int i=0; solution[i]!=0; i++)
      printf("%i ",solution[i]);
    printf("\n");
    free(solution);
  }
}


int main(int argc,char *argv[]) {
  if (argc != 4) {
    printf("must pass two parameters <filename> <seconds> <search_mode>\n");
    return 1;
  }
  int seconds = atoi(argv[2]);
  if (seconds <= 0) {
    printf("must pass >0 seconds\n");
    return 1;
  }
  int search_mode = atoi(argv[3]);
  if ((search_mode < 0) || (search_mode > 2)) {
    printf("search_mode must be an integer 0,1,2\n");
    return 1;
  }
  printf("----------------------------------\n");
  printf("--- Gnovelty solution searcher ---\n");
  printf("----------------------------------\n");
  printf("--- running for %i seconds\n",seconds);
  printf("--- running search mode %i",search_mode);
  if (search_mode == 0)
    printf(" (non-overlapping solutions specifying all variable values)\n");
  if (search_mode == 1)
    printf(" (non-overlapping solutions not necessarily specifying all variables)\n");
  if (search_mode == 2)
    printf(" (overlapping solutions not necessarily specifying all variables)\n");
  printf("---\n");
  tStart = clock();
  run(argv[1], seconds, search_mode);
  return 0;
}




