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

#ifndef _SAT_SOLVER
#define _SAT_SOLVER
#include "CnfManager.h"

struct Luby{			// restart scheduler as proposed in 
	vector<unsigned> seq; 	// Optimal Speedup of Las Vegas Algorithms
	unsigned index; 	// Michael Luby et al, 1993
	unsigned k;
	Luby(): index(0), k(1) {}
	unsigned next(){
		if(++index == (unsigned) ((1 << k) - 1))
			seq.push_back(1 << (k++ - 1));
		else
			seq.push_back(seq[index - (1 << (k - 1))]);
		return seq.back();
	}
};

class SatSolver: public CnfManager{
	unsigned nVars;		// num of variables in varOrder
	Luby luby;		// restart scheduler
	unsigned lubyUnit;	// unit run length for Luby's
	unsigned nextDecay;	// next score decay point
	unsigned nextRestart;	// next restart point
	double startTime;
	double prevTime;

	int selectLiteral();
	bool verifySolution();
public:
	SatSolver(){};
	SatSolver(Cnf &cnf, double _start_time);
	bool run();
	void printStats();
	void printStatsInline();
	void printSolution(FILE *); 	
};
#endif
