/*************************
Copyright 2026

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

#ifndef AFSAT_MAIN_HH
#define AFSAT_MAIN_HH

#include <mpi.h>
#include <string>

int afsat_main(
    MPI_Comm *communicator,
    int suggestionSize,
    const std::string &python_executable,
    const std::string &python_script,
    const std::string &python_activate,
    const std::string &visible_gpus,
    int gpus_per_helper);

#endif
