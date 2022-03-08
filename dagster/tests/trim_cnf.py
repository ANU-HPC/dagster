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
#
# given a DAG that does not index all the clauses of the CNF, trim the CNF to contain only the clauses indexed in the dag
# and update the dag to the new indexing.
#

def contract_sequence(seq):
	seq = sorted(seq)
	s = []
	for ss in seq:
		if len(s)==0 or s[-1][0]+s[-1][1]!=ss-1:
			s.append([ss,0])
		else:
			s[-1][1] += 1
	st = []
	for ss in s:
		if ss[1]==0:
			st.append("{}".format(ss[0]))
		else:
			st.append("{}-{}".format(ss[0],ss[0]+ss[1]))
	return ",".join(st)

assert len(sys.argv)==5, "need to supply <CNF>, <DAG>, <NEW_CNF>, <NEW_DAG>"

with open(sys.argv[2],"r") as f:
	dag_lines = [a.strip() for a in f.readlines()]
for i in range(len(dag_lines)):
	if dag_lines[i] == "CLAUSES:":
		clause_line = i
	if dag_lines[i] == "REPORTING:":
		reporting_line = i
clause_dictionary = {}
clause_indices = set()
for i in range(clause_line+1,reporting_line):
	node,clauses = dag_lines[i].split(":")
	clauses = clauses.split(",")
	int_clauses = []
	for c in clauses:
		if "-" in c:
			a,b = c.split('-')
			a = int(a)
			b = int(b)
			for j in range(a,b+1):
				int_clauses.append(j)
		else:
			int_clauses.append(int(c))
	int_clauses = set(int_clauses)
	clause_dictionary[int(node)] = int_clauses
	clause_indices.update(int_clauses)

with open(sys.argv[1],"r") as f:
	cnf_lines = [a.strip() for a in f.readlines() if (('c' not in a) and ('p' not in a))]
new_cnf_lines = []
new_cnf_line_len = 0
cnf_line_mapping = []
for i,line in enumerate(cnf_lines):
	if i in clause_indices:
		new_cnf_lines.append(line)
		cnf_line_mapping.append(new_cnf_line_len)
		new_cnf_line_len += 1
	else:
		cnf_line_mapping.append(-1)

max_var = 0
for l in new_cnf_lines:
	for ll in l.split(" "):
		v = abs(int(ll))
		if v>max_var:
			max_var = v

with open(sys.argv[3],"w") as f:
	f.write("p cnf {} {}\n".format(max_var,len(new_cnf_lines)))
	for l in new_cnf_lines:
		f.write("{}\n".format(l))

k = sorted(clause_dictionary.keys())
for e,i in enumerate(range(clause_line+1,reporting_line)):
	kk = k[e]
	dag_lines[i] = "{}:{}".format(kk,contract_sequence([cnf_line_mapping[a] for a in clause_dictionary[kk]] ))

with open(sys.argv[4],"w") as f:
	for d in dag_lines:
		f.write("{}\n".format(d))
		
print("DONE")
