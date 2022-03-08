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

#include "ReversableIntegerMap.h"
#include <assert.h>
#include "../exceptions.h"
#include "stdio.h"
#include "../mpi_global.h"

ReversableIntegerMap::ReversableIntegerMap(unsigned int size) {
  this->size = size;
  TEST_NOT_NULL(map = (unsigned int*)malloc(sizeof(unsigned int)*size))
  TEST_NOT_NULL(reverse_map = (unsigned int*)malloc(sizeof(unsigned int)*size))
  reset();
}

ReversableIntegerMap::ReversableIntegerMap(unsigned int* map, int map_size) {
  this->size = 0;
  for (int i=0; i<map_size; i++)
    if (map[i] > this->size)
      this->size = map[i]+1;
  TEST_NOT_NULL(this->map = (unsigned int*)malloc(sizeof(unsigned int)*this->size))
  TEST_NOT_NULL(this->reverse_map = (unsigned int*)malloc(sizeof(unsigned int)*this->size))
  reset();
  for (int i=0; i<map_size; i++)
    append(map[i]);
}

ReversableIntegerMap::ReversableIntegerMap(ReversableIntegerMap* m) : ReversableIntegerMap::ReversableIntegerMap(m->map, m->map_size) {}

ReversableIntegerMap::~ReversableIntegerMap() {
  free(map);
  free(reverse_map);
}

void ReversableIntegerMap::append(unsigned int c) {
  if (c+1>size)
    increase_size(c+1-size);
  if (reverse_map[c] == -1) {
    map[map_size] = c;
    reverse_map[c] = map_size;
    map_size++;
  }
}

void ReversableIntegerMap::remove(unsigned int c) {
  assert(c<size);
  if (reverse_map[c] != -1) {
    map_size--;
    map[reverse_map[c]] = map[map_size];
    reverse_map[map[map_size]] = reverse_map[c];
    reverse_map[c] = -1;
  }
}

void ReversableIntegerMap::resize(unsigned int new_size) {
  if (new_size>size)
    increase_size(new_size-size);
}

void ReversableIntegerMap::increase_size(unsigned int additional_size) {
  if (additional_size<=0) return;
  TEST_NOT_NULL(map = (unsigned int*)realloc(map, sizeof(unsigned int)*(size+additional_size)))
  TEST_NOT_NULL(reverse_map = (unsigned int*)realloc(reverse_map, sizeof(unsigned int)*(size+additional_size)))
  for (int i=size; i<size+additional_size; i++) {
    reverse_map[i] = -1;
  }
  this->size = size+additional_size;
}

void ReversableIntegerMap::reset() {
  for (int i=0; i<size; i++)
    reverse_map[i] = -1;
  map_size = 0;
}

void ReversableIntegerMap::print() {
  for (int i=0; i<size; i++)
    printf("%i ",map[i]);
  printf("\n");
}

inline int ReversableIntegerMap::get_forward_map(int v) {
  if ((v<0)||(v>=map_size))
    return -1;
  return map[v];
}

inline int ReversableIntegerMap::get_reverse_map(int v) {
  if ((v<0)||(v>size))
    return -1;
  return reverse_map[v];
}





