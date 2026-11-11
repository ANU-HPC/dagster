/**********************************************************/
/* A gradient based Novelty+ for guiding a Conflict       */
/* Driven Clause Learning procedure. This module is       */
/* based on gNovelty+, version 1.0                        */
/*                                                        */
/*   Authors in chronological order                       */
/*                                                        */
/*      1. Charles Gretton (charles.gretton@anu.edu.au)   */ 
/*            Australian National University              */
/*                                                        */
/*      2. Josh Milthorpe (josh.milthorpe@anu.edu.au)     */ 
/*            Australian National University              */
/*                                                        */
/*      3. Tate Kennington (tatekennington@gmail.com)     */ 
/*            University of Dunedin, New Zealand          */
/*            ANU Summer Scholarship 2018/19              */
/*                                                        */
/*      4. Mark Burgess  (markburgess1989@gmail.com)      */ 
/*            Australian National University              */
/*            Research Assistant 2019/20                  */
/*                                                        */
/*                                                        */
/**********************************************************/
/* Based on -- gNovelty+, version 1.0                     */
/*                                                        */
/*      A greedy gradient based Novelty+                  */
/*                                                        */
/*      1. Duc Nghia Pham (duc-nghia.pham@nicta.com.au)   */ 
/*            SAFE Program,  National ICT Australia Ltd.  */
/*            IIIS, Griffith University, Australia        */
/*                                                        */
/*      2. Charles Gretton (charles.gretton@gmail.com)    */ 
/*            University of Birmingham                    */
/*            IIIS, Griffith University, Australia        */
/*                                                        */
/*                                                        */
/**********************************************************/
/* Part of gNovelty+ is based on UBCSAT version 1.0       */
/* written by:                                            */
/* Dave A.D. Tompkins & Holger H. Hoos                    */
/*   Univeristy of British Columbia                       */
/* February 2004                                          */
/**********************************************************/

/*************************
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


#ifndef GNOVELTY_MAIN_HH
#define GNOVELTY_MAIN_HH

#include <mpi.h>

int gnovelty_main(MPI_Comm* communicator,
	int _suggestionSize,
	const string& advise_scheme,
	int dynamic_local_search = 0);

#endif
