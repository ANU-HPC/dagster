#!/usr/bin/python3

#SUDOKU CNF generator
# -- see https://users.aalto.fi/~tjunttil/2020-DP-AUT/notes-sat/solving.html

import sys
D = 3	# Subgrid dimension
N = D*D  # Grid dimension


def chunks(l, n):
	n = max(1, n)
	return [l[i:i+n] for i in range(0, len(l), n)]

if __name__ == '__main__':
	# Read the clues from the file given as the first argument
	assert len(sys.argv)>=3
	file_name = sys.argv[1]
	sudoku_index = int(sys.argv[2])
	if len(sys.argv)==4:
		easy = (sys.argv[3]==1)
	else:
		easy = True
	clues = []
	digits = {'0':0,'1':1,'2':2,'3':3,'4':4,'5':5,'6':6,'7':7,'8':8,'9':9}
	with open(file_name, "r") as f:
		line = f.readlines()[sudoku_index].strip().replace("0",".")
		assert len(line) == N*N, "Badly dimensioned sudoku"
		for l in line:
			assert(l in digits.keys() or l=='.')
		clues = chunks(line,N)
	assert(len(clues) == N)

	# A helper: get the Dimacs CNF variable number for the variable v_{r,c,v}
	# encoding the fact that the cell at (r,c) has the value v
	def var(r, c, v):
		assert(1 <= r and r <= N and 1 <= c and c <= N and 1 <= v and v <= N)
		return (r-1)*N*N+(c-1)*N+(v-1)+1

	# Build the clauses in a list
	cls = []  # The clauses: a list of integer lists
	for r in range(1,N+1): # r runs over 1,...,N
		for c in range(1, N+1):
			cls.append(["c cell at r={},c={} has exactly one value".format(r,c)])
			# The cell at (r,c) has at least one value
			cls.append([var(r,c,v) for v in range(1,N+1)])
			# The cell at (r,c) has at most one value
			for v in range(1, N+1):
				for w in range(v+1,N+1):
					cls.append([-var(r,c,v), -var(r,c,w)])
	for v in range(1, N+1):
		cls.append(["c each column has a value {}".format(v)])
		# Each row has the value v
		for r in range(1, N+1): cls.append([var(r,c,v) for c in range(1,N+1)])
		cls.append(["c each row has a value {}".format(v)])
		# Each column has the value v
		for c in range(1, N+1): cls.append([var(r,c,v) for r in range(1,N+1)])
		cls.append(["c each subgrid has a value {}".format(v)])
		# Each subgrid has the value v
		for sr in range(0,D):
			for sc in range(0,D):
				cls.append([var(sr*D+rd,sc*D+cd,v)
							for rd in range(1,D+1) for cd in range(1,D+1)])

	if not easy:
		for v in range(1, N+1):
			for c in range(1, N+1):
				cls.append(["c column {} has only one value {}".format(c,v)])
				for r in range(1, N+1):
					for rr in range(r+1, N+1):
						cls.append([-var(r,c,v),-var(rr,c,v)]) # each value only appears once on any row
			for r in range(1, N+1):
				cls.append(["c row {} has only one value {}".format(r,v)])
				for c in range(1,N+1):
					for cc in range(c+1, N+1):
						cls.append([-var(r,c,v),-var(r,cc,v)]) # each value only appears once on any column
			for sr in range(0,D):
				for sc in range(0,D):
					cls.append(["c subgrid {}_{} has only one value {}".format(sr,sc,v)])
					for rd in range(1,D+1):
						for cd in range(1,D+1):
							for rrd in range(1,D+1):
								for ccd in range(1,D+1):
									if (ccd>cd) or (ccd==cd and rrd>rd):
										cls.append([-var(sr*D+rd,sc*D+cd,v),-var(sr*D+rrd,sc*D+ccd,v)]) # each value only appears in a box once

	# The clues must be respected
	cls.append(["c hard number constraints"])
	for r in range(1, N+1):
		for c in range(1, N+1):
			if clues[r-1][c-1] in digits.keys():
				cls.append([var(r,c,digits[clues[r-1][c-1]])])

	with open("sudoku_cnf_{}.txt".format(sudoku_index),"w") as f:
		# Output the DIMACS CNF representation
		# Print the header line
		f.write("p cnf %d %d\n" % (N*N*N, len([c for c in cls if not isinstance(c[0],str)])))
		# print the map
		f.write("c +--------+\nc | SUDOKU |\nc +--------+\n")
		f.write("c \n");
		for i,cc in enumerate([clues[0:3],clues[3:6],clues[6:9]]):
			for c in cc:
				f.write("c {}|{}|{}\n".format(c[0:3],c[3:6],c[6:9]))
			if i<2: f.write("c ---+---+---\n");
		# print the mapping
		f.write("c \n");
		for r in range(1,N+1):
			for c in range(1, N+1):
				for v in range(1, N+1):
					f.write("c MAPPING -- r={} c={} v={}: -- {}\n".format(r,c,v,var(r,c,v)))
		# Print the clauses
		for c in cls:
			f.write(" ".join([str(l) for l in c])+" 0\n")



