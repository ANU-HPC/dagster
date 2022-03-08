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

#ifndef RIM_HH
#define RIM_HH

// an easy lookup list of integers. map stores sequential list of positive integers upto a specified number
// and reverse_map stores the position on the list that any specific integer occurs on.
class ReversableIntegerMap {
public:
  int size;
  unsigned int* map;
  unsigned int* reverse_map;
  unsigned int map_size;

  ReversableIntegerMap(unsigned int size);
  ReversableIntegerMap(unsigned int* map, int map_size);
  ReversableIntegerMap(ReversableIntegerMap *m);
  ~ReversableIntegerMap();

  void append(unsigned int c);
  void remove(unsigned int c);
  void resize(unsigned int new_size);
  void increase_size(unsigned int additional_size);
  inline int get_forward_map(int v);
  inline int get_reverse_map(int v);
  void reset();

  void print();
};

#endif
