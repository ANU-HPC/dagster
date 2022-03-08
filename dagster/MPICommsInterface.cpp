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


#include "MPICommsInterface.h"
#include "message.h"
#include <tuple>
#include "mpi_global.h"


// standard constructor, sets communicator and defines world_rank and world_size by it
// also allocates dynamic message_buffer or otherwise throws exception
MPICommsInterface::MPICommsInterface(MPI_Comm *communicator) {
  this->communicator = communicator;
  this->max_message_length = 1024;
  TEST_NOT_NULL(this->MPI_message_buffer = (int *)malloc(sizeof(int) * this->max_message_length))
  MPI_Comm_size(*communicator, &(this->world_size));
  MPI_Comm_rank(*communicator, &(this->world_rank));
};

// standard destructor, the mpi_message buffer needs to be freed
MPICommsInterface::~MPICommsInterface() {
  free(this->MPI_message_buffer);
}

// resize the buffer (if nessisary) to have atleast message_length length
void MPICommsInterface::reset_buffer(int message_length) {
  if (message_length > this->max_message_length) {
    while (message_length > this->max_message_length) this->max_message_length *= 2;
    TEST_NOT_NULL(this->MPI_message_buffer = (int*)realloc(this->MPI_message_buffer,sizeof(int)*(this->max_message_length)))
  }
}

// sends an empty message to destination with a tag.
void MPICommsInterface::send_tag(int destination, int tag) {
  MPI_Send(NULL, 0, MPI_INT, destination, tag, *(this->communicator));
}

// recieves a message from any source with any tag,
// reallocates the size of the message_buffer as appropriate and parses the incomming stream as a Message object which it returns
// return tuple is <message_source, message_tag, Message_object>
// NOTE: message object must be deleted when done.
std::tuple<int,int,Message*> MPICommsInterface::receive_message() {
  MPI_Status status;
  int message_length;
  MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, *(this->communicator), &status);
  MPI_Get_count(&status, MPI_INT, &message_length);
  reset_buffer(message_length);
  MPI_Recv(this->MPI_message_buffer, message_length, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, *(this->communicator), &status);
  Message *m = NULL;
  if (message_length > 0)
    m = new Message(this->MPI_message_buffer);
  return {status.MPI_SOURCE, status.MPI_TAG, m};
}

// given a message object, send it to destination with a tag.
// resulting message to be picked up with receive_message() function.
void MPICommsInterface::send_message(int destination, int tag, Message* m) {
  if (m==NULL) {
    send_tag(destination,tag);
    return;
  }
  int dehydrated_size = m->get_dehydrated_size();
  reset_buffer(dehydrated_size);
  m->dehydrate(this->MPI_message_buffer);
  MPI_Send(this->MPI_message_buffer, dehydrated_size, MPI_INT, destination, tag, *(this->communicator));
}


// recieves a Cnf from any source with any tag,
// reallocates the size of the message_buffer as appropriate and parses the incomming stream as a Cnf object which it returns
// return tuple is <message_source, message_tag, Cnf_object>
// NOTE: Cnf object must be deleted when done.
// NOTE: can recieve null message, returning CNF NULL.
Cnf* MPICommsInterface::receive_Cnf(int source, int tag) {
  MPI_Status status;
  int message_length;
  MPI_Probe(source, tag, *(this->communicator), &status);
  MPI_Get_count(&status, MPI_INT, &message_length);
  reset_buffer(message_length);
  MPI_Recv(this->MPI_message_buffer, message_length, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, *(this->communicator), &status);
  Cnf *cnf = NULL;
  if (message_length > 0)
    cnf = new Cnf(this->MPI_message_buffer);
  return cnf;
}

// given a message object, send it to destination with a tag.
// resulting message to be picked up with receive_message() function.
void MPICommsInterface::send_Cnf(int destination, int tag, Cnf* cnf) {
  if (cnf==NULL) {
    send_tag(destination,tag);
    return;
  }
  int dehydrated_size = cnf->get_dehydrated_size();
  reset_buffer(dehydrated_size);
  cnf->dehydrate(this->MPI_message_buffer);
  MPI_Send(this->MPI_message_buffer, dehydrated_size, MPI_INT, destination, tag, *(this->communicator));
}


// recieves a Dag from any source with any tag,
// reallocates the size of the message_buffer as appropriate and parses the incomming stream as a Dag object which it returns
// return tuple is <message_source, message_tag, Dag_object>
// NOTE: Dag object must be deleted when done.
// NOTE: can recieve null message, returning DAG NULL.
Dag* MPICommsInterface::receive_Dag(int source, int tag) {
  MPI_Status status;
  int message_length;
  MPI_Probe(source, tag, *(this->communicator), &status);
  MPI_Get_count(&status, MPI_INT, &message_length);
  reset_buffer(message_length);
  MPI_Recv(this->MPI_message_buffer, message_length, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, *(this->communicator), &status);
  Dag *dag = NULL;
  if (message_length > 0)
    dag = new Dag(this->MPI_message_buffer);
  return dag;
}

// given a message object, send it to destination with a tag.
// resulting message to be picked up with receive_message() function.
void MPICommsInterface::send_Dag(int destination, int tag, Dag* dag) {
  if (dag==NULL) {
    send_tag(destination,tag);
    return;
  }
  int dehydrated_size = dag->get_dehydrated_size();
  reset_buffer(dehydrated_size);
  dag->dehydrate(this->MPI_message_buffer);
  MPI_Send(this->MPI_message_buffer, dehydrated_size, MPI_INT, destination, tag, *(this->communicator));
}





