from sympy import *
n = 6
desired_determinant = 7

# setup the symbol matrix
zero = symbols("x") * 0
a = [[zero for i in range(n)] for j in range(n)]
for i in range(n):
	for j in range(n):
		if i!=j:
			a[i][j] = symbols("a{}_{}".format(max(i,j),min(i,j)))

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
		
		#do some sympy simplifing because all terms are binary, so we can get rid of powers
		total = total.expand()
		total_terms = list(Add.make_args(total))
		for i in range(len(total_terms)):
			power_dict = total_terms[i].as_powers_dict()
			m = 1
			for k in power_dict.keys():
				m = m * k
			total_terms[i] = m
		total = 0
		for t in total_terms:
			total += t
	return total

# calculate the determinant
determinant_terms = Add.make_args(determinant(a))





# beginning to setup CNF
from collections import defaultdict
variable_index = 0
def gen_variable():
	global variable_index
	variable_index += 1
	return variable_index
CNF_variables = defaultdict(gen_variable)

cnf = []
def add_clause(clause):
	cnf.append(clause)

big_determinant_term = {}

#for all the terms in the determinant
for term in determinant_terms:
	# split the multiplicative term in the determinant into its coefficient and symbols
	terms = term.as_powers_dict().keys()
	coefficient = 1
	terms_list = []
	for k in terms:
		if k.is_number:
			coefficient = int(k)
		else:
			terms_list.append(k)
	
	term_string = "*".join([str(t) for t in terms_list])
	term_val = CNF_variables[term_string]
	# if AND variable is true clause then all other variables are true (eg. -3A +a+b+c >= 0)
	cnf_clause = {}
	cnf_clause[term_val] = -len(terms_list)
	for t in terms_list:
		cnf_clause[CNF_variables[str(t)]] = 1
	cnf_clause['val'] = 0
	add_clause(cnf_clause)
	# if all other variables are true, then AND variable is true (eg. -A -a-b-c >= -1)
	cnf_clause = {}
	cnf_clause[term_val] = 1
	for t in terms_list:
		cnf_clause[CNF_variables[str(t)]] = -1
	cnf_clause['val'] = -len(terms_list)+1
	add_clause(cnf_clause)
	# add the AND clause with its coefficient to the big_determinant term
	big_determinant_term[term_val] = coefficient

# add the major constraint that the determinant must be greater than that desired
big_determinant_term['val'] = desired_determinant
add_clause(big_determinant_term)

# output the pseudo-boolean file
print("outputting OPB")
with open("zog.opb","w") as f:
	f.write("* #variable= {} #constraint= {}\n".format(len(CNF_variables.keys()), len(cnf)))
	for c in cnf:
		keys = [cc for cc in c.keys() if isinstance(cc,int)]
		for k in keys:
			f.write("{}{} x{} ".format('+' if c[k]>0 else '',int(c[k]),k))
		f.write(">= {} ;\n".format(int(c["val"])))

# solve using sat4j
import os
os.system("~/Downloads/sat4j-2_3_6/dist/CUSTOM/sat4j-pb.jar CompetMinPBResLongWLMixedConstraintsObjectiveExpSimp ./zog.opb > delete_me.txt")

# load the results and format into processed_data
with open("delete_me.txt","r") as f:
	data = [d[2:].strip() for d in f.readlines() if len(d)>0 and d[0]=='v']
assert(len(data)==1)
data = "".join([dd for dd in data[0] if dd!='x'])
data = data.strip().split(" ")
data = [int(d) for d in data]
processed_data = {}
for d in data:
	if d>0:
		processed_data[d] = True
	else:
		processed_data[-d] = False

# reformat the matrix
for aa in a:
	for aaa in aa:
		var = CNF_variables[str(aaa)]
		if var in processed_data.keys():
			if processed_data[var]:
				print("1",end=' ')
			else:
				print("0",end=' ')
		else:
			print("0",end=' ')
	print("")
	
