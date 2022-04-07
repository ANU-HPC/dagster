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


'''
	WITNESS_VIEW:
    ---------

	print onto the screen the regions of the resolved problem, inputs the output from the SAT solver and the key file from the generator

	the usage of the program is, calling: 

		python witness.py <sat_input> <key_input>

	where <cnf_input> is solved problem from the SAT solver
	where <key_input> is the identifier of what the litterals mean from the witness generator
'''

import sys
import json
assert len(sys.argv)==3, "You must supply a SAT solver output file and key file"

#load the file
with open(sys.argv[1], "r") as f:
	lines = [l.strip() for l in f.readlines()]

#check the SAT solver actually solved the problem and extract resolved true litterals
assert lines[0]=="SAT", "SAT SOLVER FAILED, OR BAD INPUT FILE"
lines = lines[1].split(" ")
lines = [int(i) for i in lines if int(i)>0]

#load the key file
with open(sys.argv[2], "r") as f:
	keys = json.load(f)
n = keys[0]
keys = {i:keys[i] for i in range(len(keys))}

#iterate over the relevent coordinates, finding assigned litterals and printing them
regions = [tuple(keys[l]) for l in lines]
s = ""
for y in range(1,n-1):
	for x in range(1,n-1):
		if (x,y,True) in regions:
			s = s+'#'
		else:
			s = s+'-'
	s = s+'\n'
print s
