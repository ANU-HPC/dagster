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


#ifndef UTILITIES_HH
#define UTILITIES_HH

#include <iostream>
#include <iterator>
#include <deque>
#include <set>
#include <vector>

#include <ctime>
#include <unistd.h>
#include <sys/time.h>
#include <sys/times.h>

// UTILITIES: collection of misc small helper functions, used variously

using namespace std;

int genRandomSeed();
int compareAbsInts(const void *a, const void *b);
bool compareAbs(int a, int b);
bool isSubset(int *, int, int *, int);
bool isClauseOverlap(int *inClauseArray1, int inClauseLength1, int *inClauseArray2, int inClauseLength2);
bool litInClause(int, int *, int);
void resolve_sorted_vectors(vector<int> &result, vector<vector<int>> &inputs);
int** copy_2D_int_array(int** a);
int** copy_2D_int_array(int** a, int outer_size);
int** copy_2D_int_array(int** a, int* a_size, int outer_size);
int** copy_2D_int_array_abs(int** a);
int* copy_1D_int_array(int* a);
int* copy_1D_int_array(int* a, int outer_size);


// set to stream formatting
template<typename T>
std::ostream& operator<<(std::ostream& strm, const std::set<T> &v) {
  strm<<"{ ";
  std::copy(v.begin(), v.end(), std::ostream_iterator<int>(strm, " "));
  strm<<"} ";
  return strm;
}

// vector to stream formatting
template <typename T>
std::ostream &operator<<(std::ostream &strm, const std::vector<T> &v) {
  std::copy(v.begin(), v.end(), std::ostream_iterator<int>(strm, " "));
  return strm;
}

// deaque to stream formatting
template <typename T>
std::ostream &operator<<(std::ostream &strm, const std::deque<T> &v) {
  std::copy(v.begin(), v.end(), std::ostream_iterator<int>(strm, " "));
  return strm;
}

// the "double stringize trick"
#define STR(s) #s
#define XSTR(s) STR(s)

// macro for flipping variable values
#define FLIP(A,B) { \
  auto temp = B; \
  B = A; \
  A = temp; \
} \

#endif

