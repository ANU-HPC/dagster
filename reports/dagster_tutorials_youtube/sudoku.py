D = 3 # Subgrid dimension
N = D*D # Grid dimension
var = lambda r, c, v: (r-1)*N*N+(c-1)*N+(v-1)+1
cls = []  # The clauses

for r in range(1,N+1):
	for c in range(1, N+1):
		# The cell at (r,c) has at least one value
		cls.append([var(r,c,v) for v in range(1,N+1)])
		# The cell at (r,c) has at most one value
		for v in range(1, N+1):
			for w in range(v+1,N+1):
				cls.append([-var(r,c,v), -var(r,c,w)])

for v in range(1, N+1): # each value v
	# Each row has the value v
	for r in range(1, N+1):
		cls.append([var(r,c,v) for c in range(1,N+1)])
	# Each column has the value v
	for c in range(1, N+1):
		cls.append([var(r,c,v) for r in range(1,N+1)])
	# Each subgrid has the value v
	for sr in range(0,D):
		for sc in range(0,D):
			cls.append([var(sr*D+rd,sc*D+cd,v)
			  for rd in range(1,D+1) for cd in range(1,D+1)])

# TODO: Add clues constraints
cls.append([var(1,1,5)])
# .......

# Output the DIMACS CNF representation
print("p cnf %d %d" % (N*N*N, len(cls)))
for c in cls:
	print(" ".join([str(l) for l in c])+" 0")
