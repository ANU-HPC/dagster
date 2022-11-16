#!/usr/bin/python3

# SUDOKU CNF generator
# with custom DAG file output
# modify the clauses in the dag and what nodes they occur on by the comment line that each set of clauses occurs under
#
# original: -- see https://users.aalto.fi/~tjunttil/2020-DP-AUT/notes-sat/solving.html

import sys
from collections import defaultdict
D = 3	# Subgrid dimension
N = D*D  # Grid dimension


# given an array of integers, return a formatted a string in 'printer' format
# eg. [1,2,3,5,6,7,9] -> '1-3,5-7,9'
def contract_sequence(seq):
	seq = sorted(seq)
	s = []
	for ss in seq:
		if len(s)!=0 and s[-1][0]+s[-1][1]==ss:
			continue
		elif len(s)==0 or s[-1][0]+s[-1][1]!=ss-1:
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

# break a string into chunks of size n
def chunks(l, n):
	n = max(1, n)
	return [l[i:i+n] for i in range(0, len(l), n)]

# A helper: get the Dimacs CNF variable number for the variable v_{r,c,v}
# encoding the fact that the cell at (r,c) has the value v
def var(r, c, v):
	assert(1 <= r and r <= N and 1 <= c and c <= N and 1 <= v and v <= N)
	return (r-1)*N*N+(c-1)*N+(v-1)+1



if __name__ == '__main__':
	# Read the clues from the file given as the first argument
	assert len(sys.argv)>=3, "Command line: python gen.py <FILE_NAME> <SUDOKU_INDEX>"
	file_name = sys.argv[1]
	sudoku_index = int(sys.argv[2])
	clues = []
	digits = {'0':0,'1':1,'2':2,'3':3,'4':4,'5':5,'6':6,'7':7,'8':8,'9':9}
	with open(file_name, "r") as f:
		line = f.readlines()[sudoku_index].strip().replace("0",".")
		assert len(line) == N*N, "Badly dimensioned sudoku"
		for l in line:
			assert(l in digits.keys() or l=='.')
		clues = chunks(line,N)
	assert(len(clues) == N)

	# Build the clauses in a list
	cls = defaultdict(list)  # The clauses: a list of integer lists
	for r in range(1,N+1): # r runs over 1,...,N
		for c in range(1, N+1):
			s = "c cell at r={},c={} has exactly one value".format(r,c)
			# The cell at (r,c) has at least one value
			cls[s].append([var(r,c,v) for v in range(1,N+1)])
			# The cell at (r,c) has at most one value
			for v in range(1, N+1):
				for w in range(v+1,N+1):
					cls[s].append([-var(r,c,v), -var(r,c,w)])
					
	# Each row has all values
	for r in range(1, N+1):
		s = "c each row={} has all values".format(r)
		for v in range(1, N+1):
			cls[s].append([var(r,c,v) for c in range(1,N+1)])
	# Each column has all values
	for c in range(1, N+1):
		s = "c each column={} has all values".format(c)
		for v in range(1, N+1):
			cls[s].append([var(r,c,v) for r in range(1,N+1)])
	# Each subgrid has all values
	for sr in range(0,D):
		for sc in range(0,D):
			s = "c each subgrid={},{} has all values".format(sr,sc)
			for v in range(1, N+1):
				cls[s].append([var(sr*D+rd,sc*D+cd,v) for rd in range(1,D+1) for cd in range(1,D+1)])
	
	# Redundant constraints
	for v in range(1, N+1):
		for c in range(1, N+1):
			s = "c column {} has only one value {}".format(c,v)
			for r in range(1, N+1):
				for rr in range(r+1, N+1):
					cls[s].append([-var(r,c,v),-var(rr,c,v)]) # each value only appears once on any row
		for r in range(1, N+1):
			s = "c row {} has only one value {}".format(r,v)
			for c in range(1,N+1):
				for cc in range(c+1, N+1):
					cls[s].append([-var(r,c,v),-var(r,cc,v)]) # each value only appears once on any column
		for sr in range(0,D):
			for sc in range(0,D):
				s = "c subgrid {}_{} has only one value {}".format(sr,sc,v)
				for rd in range(1,D+1):
					for cd in range(1,D+1):
						for rrd in range(1,D+1):
							for ccd in range(1,D+1):
								if (ccd>cd) or (ccd==cd and rrd>rd):
									cls[s].append([-var(sr*D+rd,sc*D+cd,v),-var(sr*D+rrd,sc*D+ccd,v)]) # each value only appears in a box once


	# The clues must be respected
	s = "c hard number constraints"
	for r in range(1, N+1):
		for c in range(1, N+1):
			if clues[r-1][c-1] in digits.keys():
				cls[s].append([var(r,c,digits[clues[r-1][c-1]])])

	with open("sudoku_cnf_{}.txt".format(sudoku_index),"w") as f:
		# Output the DIMACS CNF representation
		# Print the header line
		f.write("p cnf %d %d\n" % (N*N*N, sum([len(c) for c in cls.values()])))
		# print the map
		f.write("c +--------+\nc | SUDOKU |\nc +--------+\n")
		f.write("c \n")
		for i,cc in enumerate([clues[0:3],clues[3:6],clues[6:9]]):
			for c in cc:
				f.write("c {}|{}|{}\n".format(c[0:3],c[3:6],c[6:9]))
			if i<2: f.write("c ---+---+---\n")
		# print the mapping
		f.write("c \n");
		for r in range(1,N+1):
			for c in range(1, N+1):
				for v in range(1, N+1):
					f.write("c MAPPING -- r={} c={} v={}: -- {}\n".format(r,c,v,var(r,c,v)))
		# Print the clauses
		clause_index = 0
		cls_indices = defaultdict(list)
		cls_variables = defaultdict(list)
		for k in sorted(cls.keys()):
			f.write("{}\n".format(k))
			for c in cls[k]:
				f.write(" ".join([str(l) for l in c])+" 0\n")
				cls_indices[k].append(clause_index)
				clause_index += 1
				for l in c:
					if abs(l) not in cls_variables[k]:
						cls_variables[k].append(abs(l))
	
	'''# subgrid 1,1 filled as many ways as possible, then solve whole problem
	node_clauses = [(["c cell at r={},c={} has exactly one value".format(r,c) for r in range(1,4) for c in range(1,4)] +
		["c column {} has only one value {}".format(c,v) for c in range(1,4) for v in range(1,N+1)] +
		["c row {} has only one value {}".format(r,v) for r in range(1,4) for v in range(1,N+1)] +
		["c subgrid {}_{} has only one value {}".format(0,0,v) for v in range(1,N+1)] +
		["c hard number constraints"]),
		cls.keys()]
	node_variables = [[var(r, c, v) for r in range(1,4) for c in range(1,4) for v in range(1,N+1)],
		[var(r, c, v) for r in range(1,N+1) for c in range(1,N+1) for v in range(1,N+1)]]'''
		
		
	# subgrid 2,2 filled as many ways as possible, then solve row 2, then whole problem
	node_clauses = [(["c cell at r={},c={} has exactly one value".format(r,c) for r in range(4,7) for c in range(4,7)] +
		["c column {} has only one value {}".format(c,v) for c in range(4,7) for v in range(1,N+1)] +
		["c row {} has only one value {}".format(r,v) for r in range(4,7) for v in range(1,N+1)] +
		["c subgrid {}_{} has only one value {}".format(1,1,v) for v in range(1,N+1)] +
		["c hard number constraints"]),
		
		(["c cell at r={},c={} has exactly one value".format(r,c) for r in range(4,7) for c in range(1,N+1)] +
		["c column {} has only one value {}".format(c,v) for c in range(1,N+1) for v in range(1,N+1)] +
		["c row {} has only one value {}".format(r,v) for r in range(4,7) for v in range(1,N+1)] +
		["c subgrid {}_{} has only one value {}".format(r,1,v) for r in [0,1,2] for v in range(1,N+1)] +
		["c hard number constraints"]),
		
		cls.keys()]
	node_variables = [[var(r, c, v) for r in range(4,7) for c in range(4,7) for v in range(1,N+1)],
		[var(r, c, v) for r in range(4,7) for c in range(1,N+1) for v in range(1,N+1)],
		[var(r, c, v) for r in range(1,N+1) for c in range(1,N+1) for v in range(1,N+1)]]
	
	
	
	# output the DAG
	with open("sudoku_dag_{}.txt".format(sudoku_index),"w") as f:
		f.write("DAG-FILE\nNODES:{}\nGRAPH:\n".format(len(node_clauses)))
		for i in range(len(node_clauses)-1):
			f.write("{}->{}:{}\n".format(i,i+1,contract_sequence( node_variables[i] )))
		f.write("CLAUSES:\n")
		for i in range(len(node_clauses)):
			f.write("{}:{}\n".format(i, contract_sequence( sum([cls_indices[k] for k in node_clauses[i]],[]) )))
		f.write("REPORTING:\n")
		f.write(contract_sequence( node_variables[-1] ))
		f.write("\n")
		f.close()



