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


#ifndef SATSOLVER_INTERFACE_H_
#define SATSOLVER_INTERFACE_H_

#include "exceptions.h"
#include "message.h"
#include "Dag.h"
#include <algorithm>
#include <vector>

//SatSolverInterface:
// a minimal virtual class of functions that will be the outward presenting face of any class that handles and processes any CDCL procedure
class SatSolverInterface {
  public:
  virtual int run()=0;
  virtual void load_into_message(Message* m, RangeSet &r)=0;
  virtual bool is_solver_unit_contradiction()=0;
  virtual bool reset_solver()=0;
  virtual void solver_add_conflict_clause(std::deque<int>)=0;
  virtual void load_into_deque(deque<int> &d, RangeSet &r)=0;
  virtual ~SatSolverInterface()=0;
};

//SatSolverInterface::~SatSolverInterface() {}

#endif // SATSOLVER_INTERFACE_H_
