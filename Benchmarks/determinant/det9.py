from tqdm import tqdm

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
	cnf.append(" ".join([str(s) for s in clause])+" 0")
def add_x_clause(clause):
	cnf.append("x"+" ".join([str(s) for s in clause])+" 0")
def add_comment(comment):
	cnf.append("c "+str(comment))


n = 3
bit_resolution = 5


# setup the bit array
for x in range(n):
	for y in range(n):
		CNF_variables["m_{}_{}".format(x,y)]

# setup the inverse array
for x in range(n):
	for y in range(n):
		for b in range(bit_resolution):
			CNF_variables["i_{}_{}_{}".format(x,y,b)]


# function for creating a half-adder circuit
def HalfAdder(var1,var2,output,carry):
	var1 = CNF_variables[var1]
	var2 = CNF_variables[var2]
	carry = CNF_variables[carry]
	output = CNF_variables[output]
	
	# carry == var1 and var 2
	add_clause([-var1,-var2,carry])
	add_clause([var1,-carry])
	add_clause([var2,-carry])
	
	# output == var1 xor var 2
	add_x_clause([var1,var2,-output])

# function for creating a full-adder circuit
def FullAdder(var1,var2,var3,output,carry):
	var1 = CNF_variables[var1]
	var2 = CNF_variables[var2]
	var3 = CNF_variables[var3]
	carry = CNF_variables[carry]
	output = CNF_variables[output]
	
	# carry == sum(var1,var2,var3)>1  - ie, if any two are true/false, then the carry is true/false
	add_clause([var1,var2,-carry])
	add_clause([var1,var3,-carry])
	add_clause([var2,var3,-carry])
	add_clause([-var1,-var2,carry])
	add_clause([-var1,-var3,carry])
	add_clause([-var2,-var3,carry])
	
	# output == var1 xor var2 xor var3
	add_x_clause([var1,var2,var3,-output])

# function for creating a full-adder circuit, discarding the carry
def TerminatingAdder(var1,var2,var3,output):
	var1 = CNF_variables[var1]
	var2 = CNF_variables[var2]
	var3 = CNF_variables[var3]
	output = CNF_variables[output]
	
	# output == var1 xor var2 xor var3
	add_x_clause([var1,var2,var3,-output])



# ADDER CIRCUIT

#  C A B | O C
#  0 0 0 \ 0 0
#  0 0 1 | 1 0
#  0 1 0 \ 1 0
#  0 1 1 | 0 1
#  1 0 0 \ 1 0
#  1 0 1 | 0 1
#  1 1 0 \ 0 1
#  1 1 1 | 1 1

# O circuit
# CA   -- -+ ++ +-
# B  - 0  1  0  1    
# B  + 1  0  1  0  
# O = C xor B xor A

# C circuit
# CA   -- -+ ++ +-
# B  - 0  0  1  0    
# B  + 0  1  1  1 

# C =  


# function for adding two integers
def Add(v1,v2,v_output):
	v1_ = ["{}_{}".format(v1,i) for i in range(bit_resolution)]
	v2_ = ["{}_{}".format(v2,i) for i in range(bit_resolution)]
	v_output_ = ["{}_{}".format(v_output,i) for i in range(bit_resolution)]

	i = 0
	carry = "carry_{}_{}_{}".format(v1,v2,i)
	HalfAdder(v1_[i],v2_[i],v_output_[i],carry)  #half adder for the first bit pairs
	old_carry = carry
	i += 1
	while i<bit_resolution-1:
		carry = "carry_{}_{}_{}".format(v1,v2,i)
		FullAdder(v1_[i],v2_[i],old_carry,v_output_[i],carry) #full adder for the middle bit pairs, translating the carry
		old_carry = carry
		i += 1
	TerminatingAdder(v1_[i],v2_[i],old_carry,v_output_[i]) #terminating adder for the last bits, discarding the carry

# function for gating an integer
def Gated(v1,gate,output):
	v1_ = [CNF_variables["{}_{}".format(v1,i)] for i in range(bit_resolution)]
	v_output_ = [CNF_variables["{}_{}".format(output,i)] for i in range(bit_resolution)]
	gate_ = CNF_variables[gate]
	
	for i in range(bit_resolution):
		add_clause([-gate_,-v1_[i],v_output_[i]]) # if gate is true, then input <==> output
		add_clause([-gate_,v1_[i],-v_output_[i]])
		add_clause([gate_,-v_output_[i]]) # if gate is false, then output bit is false

# function if integer from array of bits v1_ is greater than that of v2_ (assuming v1,v2 are positive)
def Compare(v1_,v2_,vv1,vv2,g,l):
	length = len(v1_)
	assert len(v1_)==len(v2_)
	g = CNF_variables[g]
	l = CNF_variables[l]
	for i in range(length-1,-1,-1):
		v1i = CNF_variables["{}_{}".format(vv1,i)] # mark at this depth
		v2i = CNF_variables["{}_{}".format(vv2,i)]
		if i==length-1:
			add_clause([-v1_[i], v2_[i], v1i ]) # if difference at this depth, then mark at this depth appropriately
			add_clause([v1_[i], -v2_[i], v2i ])
			add_clause([v1_[i], v2_[i], -v1i ]) # if no difference at this depth, then dont set mark at this depth appropriately
			add_clause([-v1_[i], -v2_[i], -v2i ])
		else:
			add_clause([-prev_v1i, v1i ]) # if marked previous depth, then mark at this depth
			add_clause([-prev_v2i, v2i ])
			add_clause([prev_v1i, -v1_[i], v2_[i], v1i ]) # if not marked at previous depth and if difference at this depth, then mark at this depth appropriately
			add_clause([prev_v2i, v1_[i], -v2_[i], v2i ])
			add_clause([prev_v1i, v1_[i], v2_[i], -v1i ]) # if not marked at previous depth and if no difference at this depth, then dont set mark at this depth appropriately
			add_clause([prev_v2i, -v1_[i], -v2_[i], -v2i ])
		add_clause([-v1i, v2i, g]) # if difference in marks then set appropriate g/l flag
		add_clause([v1i, -v2i, l])
		prev_v1i = v1i
		prev_v2i = v2i
	add_clause([-v1i,-v2i,-g]) # if no flags set, then g&l flags are both false
	add_clause([-v1i,-v2i,-l])


# assert that an integer is zero
def AssertIsZero(v1):
	v1_ = [CNF_variables["{}_{}".format(v1,i)] for i in range(bit_resolution)]
	for v in v1_:
		add_clause([-v])

# assert that an integer is not zero
def AssertIsNotZero(v1):
	v1_ = [CNF_variables["{}_{}".format(v1,i)] for i in range(bit_resolution)]
	add_clause(v1_)

# assert that an integer is >0
def AssertIsPositive(v1):
	v1_ = [CNF_variables["{}_{}".format(v1,i)] for i in range(bit_resolution)]
	add_clause(v1_)
	add_clause([-v1_[-1]])

# assert that two integers are equal
def AssertIsEqual(v1,v2):
	v1_ = [CNF_variables["{}_{}".format(v1,i)] for i in range(bit_resolution)]
	v2_ = [CNF_variables["{}_{}".format(v2,i)] for i in range(bit_resolution)]
	for i in range(bit_resolution):
		add_clause([-v1_[i],v2_[i]])
		add_clause([v1_[i],-v2_[i]])	

# function for if an integer has absolute value of 1
def isAbsOne(v1,t):
	v1_ = [CNF_variables["{}_{}".format(v1,i)] for i in range(bit_resolution)]
	t = CNF_variables[t]
	add_clause([-t,v1_[0]]) # t => first bit is 1 
	for i in range(2,bit_resolution): #for the remaining bits, t => they must all be the same
		add_clause([-t,-v1_[i-1],v1_[i]])
		add_clause([-t,v1_[i-1],-v1_[i]]) #ie. if t is true, then bidirectional implication

# function if an integer is positive (ie >0)
def isPositive(v1,t):
	v1_ = [CNF_variables["{}_{}".format(v1,i)] for i in range(bit_resolution)]
	t = CNF_variables[t]
	add_clause([-t,v1_[-1]]) #if t, then first bit must be zero 
	add_clause([-t]+v1_) #if t, then one other but must be positive




# conditions
#   - the binary matrix multiplied by its integer inverse must equal the identity multiplied by some arbitrary positive integer
#   - that the inverse matrix must have a variable with a absolute value of 1 in it  (prevents scaling problems)
#   - that elementwise, if the matrix element is 0 then the inverse element is negative (or zero), if the matrix element is 1 then the inverse element is positive (or zero)
#   - the binary matrix must be symmetric
#   - the binary matrix must be symmetry broken wrt vertex swaps


# the matrix multiplied by its inverse must be identity
print("writing matrix identity constraints")
add_comment("MATRIX IDENTITY CONSTRAINTS")
for col in tqdm(range(n)): # iterating over columns of the inverse matrix
	for row in range(n): # iterating over rows of the matrix
		add_comment("ROW: {}, COLUMN: {}".format(row,col))
		add_comment("Creating GATED ROW")
		for k in range(n): # k is iterator
			# gate the inverse element by the corresponding matrix element
			Gated("i_{}_{}".format(k,col),"m_{}_{}".format(row,k),"gi_{}_{}_{}".format(row,col,k))
		# perform the sum along the index
		AssertIsEqual("gi_{}_{}_{}".format(row,col,0),"gis_{}_{}_{}".format(row,col,0))
		for k in range(1,n):
			Add("gis_{}_{}_{}".format(row,col,k-1),"gi_{}_{}_{}".format(row,col,k-1),"gis_{}_{}_{}".format(row,col,k))
		# assert the result of the sum is zero if off-axis
		if col!=row:
			AssertIsZero("gis_{}_{}_{}".format(row,col,n-1))
		elif col==0: #else assert positive for first diagonal element
			AssertIsPositive("gis_{}_{}_{}".format(row,col,n-1))
		else: #else assert all diagonal elements are equal
			AssertIsEqual("gis_{}_{}_{}".format(row,col,n-1),"gis_{}_{}_{}".format(row-1,col-1,n-1))

# that the inverse matrix must have an element with absolute value of 1
is_abs_ones = []
print("writing normaliseation of inverse matrix constraints")
for col in tqdm(range(n)):
	for row in range(n):
		t = "abs1_{}_{}".format(row,col)
		is_abs_ones.append(t)
		isAbsOne("i_{}_{}".format(row,col),t)
add_clause([CNF_variables[t] for t in is_abs_ones])

# that elementwise, if the matrix element is 0 then the inverse element is negative (or zero), if the matrix element is 1 then the inverse element is positive (or zero)
# ie. if inverse element is positive, then matrix element is 1, if inverse element is negative, then matrix element is 0
print("writing determinant non-decreasing constraint for elementwise change")
for col in tqdm(range(n)):
	for row in range(n):
		v1 = "m_{}_{}".format(row,col)
		v2 = "i_{}_{}".format(row,col)
		v2_end = "{}_{}".format(v2,bit_resolution-1)
		v2_pos = "i_{}_{}_pos".format(row,col)
		isPositive(v2,v2_pos)
		add_clause([-CNF_variables[v2_pos],  CNF_variables[v1] ])
		add_clause([-CNF_variables[v2_end], -CNF_variables[v1] ])

# that the matrix is an binary adjacency matrix with no self loops
# ie, matrix is symmetric, with zero diagonal
print("writing symmetry constraints")
for col in tqdm(range(n)):
	for row in range(n):
		if row>col:
			# symmetric equality constraint
			v1 = "m_{}_{}".format(row,col)
			v2 = "m_{}_{}".format(col,row)
			add_clause([-CNF_variables[v1],CNF_variables[v2]])
			add_clause([CNF_variables[v1],-CNF_variables[v2]])
		if row==col:
			# diagonal is false
			add_clause([-CNF_variables["m_{}_{}".format(row,col)] ])

# the binary matrix must be symmetry broken wrt vertex swaps
print("writing vertex symmetry constrints")
for row in tqdm(range(n-1)):
	v1_ = [CNF_variables["m_{}_{}".format(row,col)] for col in range(n) if col!=row and col!=row+1]
	v2_ = [CNF_variables["m_{}_{}".format(row+1,col)] for col in range(n) if col!=row and col!=row+1]
	v1_.reverse()
	v2_.reverse()
	g = "comp_row_{}_{}_g".format(row,row+1)
	l = "comp_row_{}_{}_l".format(row,row+1)
	Compare(v1_,v2_,g,l,g,l)
	add_clause([-CNF_variables[g]])


print("outputting CNF and MAP")
with open("cnf.txt", "w") as f:
	f.write("p cnf {} {}\n".format(len(CNF_variables.keys()), len(cnf)))
	for c in tqdm(cnf):
		f.write("{}\n".format(c))

with open("map.map","w") as f:
	for k,v in tqdm(CNF_variables.items()):
		f.write("{} {}\n".format(k,v))



