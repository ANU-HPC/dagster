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


#ifndef _MASTER_ORGANISER_H
#define _MASTER_ORGANISER_H

#include <vector>
#include "message.h"
#include "DisorderedArray.h"

using namespace std;


// status of a given worker, as relevent to the Master
// what work it is currently operating on, what work it will (eventually) be reassigned to, and whether or not it is asking for a reassignment
class MasterUnit {
public:
  Message* assigned;  // the message that the worker is currently processing
  Message* to_be_assigned;  // the message that the worker will be given to process when it polls for a new assignment
  bool polled; // boolean flag indicating the worker wants a response
  bool needs_refresh; // a boolean flag indicating that the message that this one is working on has updated additional clauses

  MasterUnit();
};

// class which holds basic organisations and manipulations between workers and messages
class MasterOrganiser {
public:
  DisorderedArray<Message*> message_buffer;
  MasterUnit* workers;
  int num_workers;

  MasterOrganiser(int num_workers);
  ~MasterOrganiser();

  // add to the message buffer
  void add_message(Message* m);

  // remove from the message buffer, freeing all allocation to workers
  void remove_message(Message* m);

  // get information about what messages will have the max/min number of workers on them
  // after all allocations have been completed
  std::tuple<int,Message*,int,Message*> get_min_max_count();

  // for all workers, issue allocations so that they balance between the messages
  // NOTE: this method is not very efficient, but if we are talking small numbers of workers (ie. < 1000) then meh.
  void allocate_assignments();

  // for all workers working on a message m, set their needs_refresh to be true
  void refresh_except(Message* m, int worker);

  void debug_message(char* prepend);
  
  void dump_checkpoint(FILE* fp);
  void load_checkpoint(FILE* fp);
};

#endif

