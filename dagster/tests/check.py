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
try:
	from pysat.solvers import Minisat22
	from pysat.formula import CNF
except ImportError as e:
	print("Cannot import python libraries, needs PySAT library with Minisat import")
	raise

# parse inputs
assert len(sys.argv)==3 or len(sys.argv)==4, "need to supply <CNF>, <SOLUTION_FILE> [CHECK_MODE]"
cnf_filename = sys.argv[1]
outputs_filename = sys.argv[2]
if len(sys.argv)==4:
	check_only_some = (sys.argv[3].lower()=="some")
else:
	check_only_some = False

# load the CNF and extract all cnflines
with open(cnf_filename,"r") as f:
	cnflines = f.readlines()
cnflines = [c for c in cnflines if (('p' not in c) and ('c' not in c))]
cnflines = [c.strip() for c in cnflines]
cnflines = [[int(cc) for cc in c.split(" ") if cc!="0" and cc!=""] for c in cnflines]

# load the solution lines
with open(outputs_filename,"r") as f:
	sollines = f.readlines()
sollines = [c.strip() for c in sollines]
sollines = [[int(cc) for cc in c.split(" ") if cc!="0" and cc!=""] for c in sollines]

if not check_only_some:
	# verify that there is no extra solutions, running minisat on the CNF + negation of all solution lines
	m = Minisat22(bootstrap_with= cnflines + [[-ss for ss in s] for s in sollines])
	assert m.solve()==False, "missing solutions"

# for each solution line, ensure that it satisfies all CNF clauses
for sol in sollines:
	m = Minisat22(bootstrap_with= cnflines + [[s] for s in sol])
	assert m.solve(), "solution {} did not satisfy all CNF clauses".format(sol)

print("CHECK PASSED")
