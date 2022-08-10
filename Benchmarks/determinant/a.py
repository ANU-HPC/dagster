from sympy import *
n = 6
a = Matrix([[0 for i in range(n)] for j in range(n)])
for i in range(n):
	for j in range(n):
		if i!=j:
			a[i,j] = symbols("a{}_{}".format(max(i,j),min(i,j)))
determinant = a.det()
determinant_terms = Add.make_args(determinant)


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


# beginning to setup CNF
from collections import defaultdict
variable_index = 0
def a():
	global variable_index
	variable_index += 1
	return variable_index
CNF_variables = defaultdict(a)

cnf = []
def add_clause(clause):
	cnf.append(clause)


for term in determinant_terms:
	power_dictionary = term.as_powers_dict()
	
	coefficient = 1
	terms_dictionary
	for k in power_dictionary.keys():
		if k.is_number():
			coefficient = int(k)
		else:
			terms_dictionary[k] = power_dictionary[k]
	


print("outputting OPB")
with open("zog.opb","w") as f:
	f.write("* #variable= {} #constraint= {}\n".format(len(CNF_variables.keys()), len(cnf)))
	for c in tqdm(cnf):
		keys = [cc for cc in c.keys() if isinstance(cc,int)]
		for k in keys:
			f.write("{}{} x{} ".format('+' if c[k]>0 else '',int(c[k]),k))
		if "val" not in c.keys():
			print(c)
			print(c.keys())
		f.write(">= {} ;\n".format(int(c["val"])))




