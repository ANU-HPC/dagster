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



#ifndef UTILITIES_CC
#define UTILITIES_CC

#include <ctime>
#include <unistd.h>
#include <sys/time.h>
#include <sys/times.h>
#include <stdlib.h>
#include "utilities.h"
#include "exceptions.h"
#include <limits.h>

// set pseudo-random number, to be used as a seed for default random number generator
int genRandomSeed() {
  struct timeval tv;
  struct timezone tzp;
  gettimeofday(&tv, &tzp);
  return ((( tv.tv_sec & 0x000007FF ) * 1000000) + tv.tv_usec);
}

// return difference of absolute value of integers at pointers (used in sort algorithm) 
int compareAbsInts(const void *a, const void *b) {
    int *va = (int *) a;
    int *vb = (int *) b;
    return ( abs(*va) - abs(*vb) );
}

// is abs of one integer greater than another
bool compareAbs(int a, int b) {
    return ( abs(a) < abs(b) );
}

// given a literal integer, and an int* array, is lit in array
bool litInClause(int lit, int *inClauseArray, int inClauseLength) {
  for (int i = 0; i < inClauseLength; i++)
    if (inClauseArray[i] == lit)
      return true;
  return false;
}

// is one int* array a subset of another int* array
bool isSubset(int *inClauseArray1, int inClauseLength1, int *inClauseArray2, int inClauseLength2) {
  for (int i = 0; i < inClauseLength1; i++) {
    bool found = litInClause(inClauseArray1[i],inClauseArray2,inClauseLength2);
    if (!found)
      return false;
  }
  return true;
}

// is there any overlap between two int* arrays
bool isClauseOverlap(int *inClauseArray1, int inClauseLength1, int *inClauseArray2, int inClauseLength2) {
  for (int i = 0; i < inClauseLength1; i++) {
    bool found = litInClause(inClauseArray1[i],inClauseArray2,inClauseLength2);
    if (found)
      return true;
  }
  return false;
}

// take a series of sorted input vectors (by compareAbs), and resolve them together
void resolve_sorted_vectors(vector<int> &result, vector<vector<int>> &inputs) {
  int input_size = inputs.size();
  vector<int> upto;
  upto.clear();
  upto.resize(input_size,0);
  result.clear();
  if (input_size == 0)
    return;
  int least = -1;
  int least_value = INT_MAX;
  int last_number_added = INT_MAX;
  while (true) {
    for (int i=0; i<input_size; i++) {
      if (upto[i]<inputs[i].size()) {
        if (compareAbs( inputs[i][upto[i]],least_value)) {
          least = i;
          least_value = inputs[i][upto[i]];
        }
      }
    }
    if (least!=-1) {
      if (least_value == -last_number_added)
        throw ConsistencyException(" trying to resolve together inconsistent messages\n");
      if (least_value != last_number_added) {
        result.push_back(least_value);
        last_number_added = least_value;
      }
      upto[least] += 1;
      least = -1;
      least_value = INT_MAX;
    } else
      break;
  }
}

int** copy_2D_int_array(int** a) {
	int** b;
	int outer_size = 0;
	while (a[outer_size]!=NULL) outer_size++;
	b = (int**)calloc(sizeof(int*),outer_size+1);
	if (b==NULL) return NULL;
	for (int i=0; i<outer_size; i++) {
		int inner_size = 0;
		while (a[i][inner_size]!=0) inner_size++;
		b[i] = (int*)calloc(sizeof(int),inner_size+1);
		if (b[i]==NULL) {
			for (int j=0; j<i; j++) {
				free(b[j]);
			}
			free(b);
			return NULL;
		}
		for (int j=0; j<inner_size; j++) {
			b[i][j] = a[i][j];
		}
	}
	return b;
}


int** copy_2D_int_array_abs(int** a) {
	int** b;
	int outer_size = 0;
	while (a[outer_size]!=NULL) outer_size++;
	b = (int**)calloc(sizeof(int*),outer_size+1);
	if (b==NULL) return NULL;
	for (int i=0; i<outer_size; i++) {
		int inner_size = 0;
		while (a[i][inner_size]!=0) inner_size++;
		b[i] = (int*)calloc(sizeof(int),inner_size+1);
		if (b[i]==NULL) {
			for (int j=0; j<i; j++) {
				free(b[j]);
			}
			free(b);
			return NULL;
		}
		for (int j=0; j<inner_size; j++) {
			b[i][j] = abs(a[i][j]);
		}
	}
	return b;
}



int** copy_2D_int_array(int** a, int outer_size) {
	int** b;
	b = (int**)calloc(sizeof(int*),outer_size+1);
	if (b==NULL) return NULL;
	for (int i=0; i<outer_size; i++) {
		if (a[i]==NULL) continue;
		int inner_size = 0;
		while (a[i][inner_size]!=0) inner_size++;
		b[i] = (int*)calloc(sizeof(int),inner_size+1);
		if (b[i]==NULL) {
			for (int j=0; j<i; j++) {
				if (b[j]!=NULL)
					free(b[j]);
			}
			free(b);
			return NULL;
		}
		for (int j=0; j<inner_size; j++) {
			b[i][j] = a[i][j];
		}
	}
	return b;
}


int** copy_2D_int_array(int** a, int* a_size, int outer_size) {
	int** b;
	b = (int**)calloc(sizeof(int*),outer_size+1);
	if (b==NULL) return NULL;
	for (int i=0; i<outer_size; i++) {
		if (a[i]==NULL) continue;
		int inner_size = a_size[i];
		b[i] = (int*)calloc(sizeof(int),inner_size+1);
		if (b[i]==NULL) {
			for (int j=0; j<i; j++) {
				if (b[j]!=NULL)
					free(b[j]);
			}
			free(b);
			return NULL;
		}
		for (int j=0; j<inner_size; j++) {
			b[i][j] = a[i][j];
		}
	}
	return b;
}


int* copy_1D_int_array(int* a) {
	int* b;
	int outer_size = 0;
	while (a[outer_size]!=0) outer_size++;
	b = (int*)calloc(sizeof(int),outer_size+1);
	if (b==NULL) return NULL;
	for (int j=0; j<outer_size; j++) {
		b[j] = a[j];
	}
	return b;
}

int* copy_1D_int_array(int* a, int outer_size) {
	int* b = (int*)calloc(sizeof(int),outer_size+1);
	if (b==NULL) return NULL;
	for (int j=0; j<outer_size; j++) {
		b[j] = a[j];
	}
	return b;
}

#endif

