
from sympy import *
from collections import defaultdict


variable_index = 0
def a():
	global variable_index
	variable_index += 1
	return variable_index
CNF_variables = defaultdict(a)

cnf = []
def add_clause(clause):
	#index = len(cnf)
	cnf.append(clause)
	#for c in clause:
	#	CNF_variable_indices[abs(c)].append(index)

n = 5
a = [[0 for i in range(n)] for j in range(n)]
for i in range(n):
	for j in range(n):
		if i!=j:
			a[i][j] = CNF_variables["a{}_{}".format(max(i,j),min(i,j))]

def determinant(M):
	# Base case of recursive function: 1x1 matrix
	if len(M) == 1:
		return M[0][0]

	total = 0
	for column, element in enumerate(M[0]):
		# Exclude first row and current column.
		K = [x[:column] + x[column + 1 :] for x in M[1:]]
		s = 1 if column % 2 == 0 else -1
		total += s * element * determinant(K)
	return total
