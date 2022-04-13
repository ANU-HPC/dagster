/*************************
Copyright 2007 Jinbo Huang

This file is part of Tinisat.

Tinisat is free software; you can redistribute it 
and/or modify it under the terms of the GNU General 
Public License as published by the Free Software 
Foundation; either version 2 of the License, or
(at your option) any later version.

Tinisat is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR 
A PARTICULAR PURPOSE. See the GNU General Public 
License for more details.

You should have received a copy of the GNU General
Public License along with Tinisat; if not, write to
the Free Software Foundation, Inc., 51 Franklin St, 
Fifth Floor, Boston, MA  02110-1301  USA
*************************/

#ifndef _CNF
#define _CNF

struct Cnf{
	unsigned vc;	// var count
	unsigned cc;	// clause count
	int **clauses;	// 2-dim. array with entries same as in cnf file
	unsigned lc;	// literal count
	unsigned *cl;	// clause length
	Cnf(char *fname);
	~Cnf();
};
#endif
