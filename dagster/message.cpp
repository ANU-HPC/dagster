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


#include "message.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <glog/logging.h>
#include "utilities.h"

using namespace std;

// shorthand constructor, creating an empty message, with a to and from field
Message::Message(int new_to, int new_from) {
  to = new_to;
  from = new_from;
  assignments.clear();
  additional_clauses = NULL;
}

// create a duplicate message from another message
Message::Message(Message *m) {
  additional_clauses = NULL;
  set(m);
}

// generate a blank message
Message::Message() {
  to = 0;
  from =0;
  assignments.clear();
  additional_clauses = NULL;
}

// generate a new message from denydrated message data
Message::Message(int* data) {
  additional_clauses = NULL;
  hydrate(data);
}

// standard destructor
Message::~Message() {
  assignments.clear();
  if (additional_clauses != NULL)
    delete additional_clauses;
}

// returns the size of the dehydrated message
int Message::get_dehydrated_size() {
  if (additional_clauses == NULL)
    return assignments.size() + 5;
  else
    return assignments.size() + 5 + additional_clauses->get_dehydrated_size();
}

// dehydrates the message into a data buffer and returns the size of the dehydrated message
int Message::dehydrate(int *data) {
  int upto = 1;
  data[upto++] = to;
  data[upto++] = from;
  data[upto++] = (additional_clauses != NULL);
  data[upto++] = assignments.size();
  for (int i = 0; i < assignments.size(); i++)
    data[upto++] = assignments[i];
  int size = upto;
  if (additional_clauses != NULL)
    size += additional_clauses->dehydrate(&(data[upto]));
  data[0] = size;
  return size;
}

// from a dehydrated message, turn into a full message with all fields populated
int Message::hydrate(int *data) {
  to = data[1];
  from = data[2];
  assignments.resize(data[4]);
  for (int i = 0; i < data[4]; i++)
    assignments[i] = data[5 + i];
  if (additional_clauses != NULL)
    delete additional_clauses;
  additional_clauses = NULL;
  if (data[3])
    additional_clauses = new Cnf(&(data[5 + data[4]]));
  return data[0]; // message size
}

// set this message to be a duplicate of another message
void Message::set(Message *m) {
  to = m->to;
  from = m->from;
  assignments.resize(m->assignments.size());
  for (int i = 0; i < assignments.size(); i++)
    assignments[i] = m->assignments[i];
  if (additional_clauses != NULL)
    delete additional_clauses;
  if (m->additional_clauses == NULL)
    additional_clauses=NULL;
  else
    additional_clauses = new Cnf(m->additional_clauses);
}

void Message::set_additional_clauses(int** clauses) {
  if (additional_clauses != NULL) {
    delete additional_clauses;
    additional_clauses = NULL;
  }
  if (clauses != NULL) {
    additional_clauses = new Cnf(clauses);
  } else {
    additional_clauses = NULL;
  }
}

void Message::print() {
  printf("messgage about node %i assignments:",to);
  for (const auto &assignment : assignments)
    printf("%i ",assignment);
  printf("\n");
  if (additional_clauses!=NULL) {
    printf("additional clauses %i\n",additional_clauses->cc);
  }
}


void Message::purge_variables(RangeSet &r) {
  int assignments_size = assignments.size();
  for (int i = 0; i < assignments_size; i++) {
    if (!(r.find(abs(assignments[i])))) {
      assignments.erase(assignments.begin() + i);
      i--;
      assignments_size--;
    }
  }
}


// print a summary of the message to the io stream
std::ostream &operator<<(std::ostream &strm, const Message &msg) {
  strm << "message about node " << msg.to;
  strm << " assignments:";
  for (const auto &assignment : msg.assignments)
    strm << " " << assignment;
  if (msg.additional_clauses!=NULL)
    strm << " additional_clauses: " << msg.additional_clauses->cc;
  return strm;
}



void Message::dump_to_file(FILE* f) {
	fprintf(f,"%i %i [",this->from,this->to);
	for (auto it = assignments.begin(); it!= assignments.end(); it++) {
		fprintf(f,"%i ",*it);
	}
	fprintf(f,"]");
}
void Message::read_from_file(FILE* f) {
	fscanf(f,"%i %i [",&(this->from),&(this->to));
	this->assignments.clear();
	while (true) {
		int ll = 0;
		int l = fscanf(f,"%i ",&ll);
		if (l!=1) {
			break;
		}
		this->assignments.push_back(ll);
	}
	fgetc(f);
}




