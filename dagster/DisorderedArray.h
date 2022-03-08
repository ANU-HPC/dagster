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


#ifndef DISORDEREDARRAY_HH
#define DISORDEREDARRAY_HH

// A simple indexible vector type class, in which elements can be added or removed individually
// note that adding apprends to the end of the array, while removing reorderes indicies
template <typename T> 
class DisorderedArray {
private:
  int length_MAX;
  void setup(int length_MAX) {
    this->length_MAX = length_MAX;
    buffer = (T*)malloc(sizeof(T*)*length_MAX);
    length = 0;
  }
public:
  T* buffer;
  int length;
  DisorderedArray() {
    setup(8);
  }
  DisorderedArray(int length_MAX) {
    setup(length_MAX);
  }
  ~DisorderedArray() {
    free(buffer);
  }
  void add(T m) {
    if (length+1 > length_MAX) {
      length_MAX *= 2;
      buffer = (T*)realloc(buffer, sizeof(T*)*length_MAX);
    }
    buffer[length++] = m;
  }
  void remove(T m) {
    for (int i=0; i<length;)
      if (buffer[i] == m)
        buffer[i] = buffer[--length];
      else
        i++;
  }
  T& operator[] (int index) {
    return buffer[index];
  }
};

#endif

