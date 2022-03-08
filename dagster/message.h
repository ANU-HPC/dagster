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


#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "Cnf.h"

#include <iostream>
#include <iterator>
#include <vector>
#include <deque>
#include "utilities.h"
#include <ctime>

class Message {
public:
  std::vector<int> assignments; // a list of literal assignments for the message
  int to;                                     // the node destination of the message
  int from;                                   // the node origin of the message
  Cnf* additional_clauses;	// a Cnf structure (which may be NULL) holding any additional clauses
  Message(int new_to, int new_from);
  Message(Message *m);
  Message();
  Message(int* data);
  ~Message();
  
  clock_t time_start; // a temporary field (not communicated) indicating the time the message was drawn from solutions.

  int get_dehydrated_size();
  int dehydrate(int *data);      // turn the message into a newly created series of ints
  void print();            // output message contents to shell
  void set(Message *m);    // set this message to duplicate provided message
  void set_additional_clauses(int** clauses);

  void purge_variables(RangeSet &r);
  
  void dump_to_file(FILE* f); // for an open writable file stream, dump the contents of the message
  void read_from_file(FILE* f); // for an open readable file stream, read the contents of the message as produced by the dump_to_file() method
  
private:
  int hydrate(int* data); // turn a series of ints into a coherent message
};

std::ostream &operator<<(std::ostream &strm, const Message &msg);

#endif // MESSAGE_H_


