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


#ifndef MPI_COMMS_INTERFACE_H_
#define MPI_COMMS_INTERFACE_H_

#include "Message.h"
#include <tuple>
#include "mpi.h"
#include "exceptions.h"
#include "Cnf.h"
#include "Dag.h"

class MPICommsInterface {
  private:
  MPI_Comm *communicator;
  int max_message_length;
  int *MPI_message_buffer;
  void reset_buffer(int size);
  
  public:
  int world_size;
  int world_rank;

  MPICommsInterface(MPI_Comm *communicator);
  ~MPICommsInterface();

  // sends an empty message with a specified tag to designation, can be recieved by recieve_message, which will give NULL message 
  void send_tag(int destination, int tag);
  // recieves a Message object from whoever sends it, return tupple is <sending_source, sending_tag, message_object>
  // NOTE: must delete message object when done.
  std::tuple<int,int,Message*> receive_message();
  // given a message object, send it to a destination with a specified tag
  void send_message(int destination, int tag, Message* m);
  // recieves a CNF object from whoever sends it, returning CNF_object
  // NOTE: must delete CNF object when done.
  Cnf* receive_Cnf(int source, int tag);
  // given a CNF object, send it to a destination with a specified tag
  void send_Cnf(int destination, int tag, Cnf* cnf);
  // recieves a Dag object from whoever sends it, returning dag_object
  // NOTE: must delete dag object when done.
  Dag* receive_Dag(int source, int tag);
  // given a dag object, send it to a destination with a specified tag
  void send_Dag(int destination, int tag, Dag* dag);

};


#endif

