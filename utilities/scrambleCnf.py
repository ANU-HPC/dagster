#*************************
#Copyright 2021 Mark Burgess
#
#This file is part of Dagster.
#
#Dagster is free software; you can redistribute it 
#and/or modify it under the terms of the GNU General 
#Public License as published by the Free Software 
#Foundation; either version 2 of the License, or
#(at your option) any later version.
#
#Dagster is distributed in the hope that it will be
#useful, but WITHOUT ANY WARRANTY; without even the
#implied warranty of MERCHANTABILITY or FITNESS FOR 
#A PARTICULAR PURPOSE. See the GNU General Public 
#License for more details.
#
#You should have received a copy of the GNU General 
#Public License along with Dagster.
#If not, see <http://www.gnu.org/licenses/>.
#*************************

import sys
import random

with open(sys.argv[1], 'r') as inFile:
	allLines = inFile.readlines()
clauses = []
for line in allLines:
	if line[0] == 'c':
		print(line.rstrip())
	if line[0] == 'p':
		variables = int(line.split(" ")[2])
		print(line.rstrip())
	else:
		clauses.append(line)
	random.shuffle(clauses)
var_map = list(range(1,variables+1))
random.shuffle(var_map)
var_map = [0] + var_map

for clause in clauses:
	lits = [int(a) for a in clause.rstrip().split(" ")[:-1]]
	random.shuffle(lits)
	for lit in lits:
		lit_pos = 1 if lit>0 else -1
		print(str(var_map[abs(lit)]*lit_pos) + " ", end='')
	print("0")
