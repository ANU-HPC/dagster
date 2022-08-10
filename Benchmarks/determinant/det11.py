# beginning to setup CNF
from collections import defaultdict
from tqdm import tqdm
import os

class Problem(object):
	variable_index = None
	CNF_variables = None
	cnf = None
	n = None
	bit_resolution = None
	interesting_integers = None
	interesting_variables = None
	interesting_matricies = None
	
	def new_variable_index(self):
		self.variable_index += 1
		return self.variable_index
	
	def __init__(self,n,bit_resolution):
		self.variable_index = 0
		self.CNF_variables = defaultdict(self.new_variable_index)
		self.cnf = []
		self.n = n
		self.bit_resolution = bit_resolution
		self.interesting_integers = []
		self.interesting_variables = []
		self.interesting_matricies = []

	def add_clause(self,clause):
		self.cnf.append(" ".join([str(s) for s in clause])+" 0")
	def add_x_clause(self,clause):
		self.cnf.append("x"+" ".join([str(s) for s in clause])+" 0")
	def add_a_x_clause(self,clause):
		for i in range(2**(len(clause)-1)):
			c = ""
			mod = 1
			for j in range(len(clause)-1):
				polarity = 1 if i&(1<<j) else -1
				mod *= polarity
				c = c + str(polarity*clause[j]) + " "
			c = c + "{} 0".format(mod*clause[-1])
			self.cnf.append(c)
	def add_comment(self,comment):
		self.cnf.append("c "+str(comment))

	# function for creating a half-adder circuit
	def HalfAdder(self,var1,var2,output,carry):
		var1 = self.CNF_variables[var1]
		var2 = self.CNF_variables[var2]
		carry = self.CNF_variables[carry]
		output = self.CNF_variables[output]
		
		# carry == var1 and var 2
		self.add_clause([-var1,-var2,carry])
		self.add_clause([var1,-carry])
		self.add_clause([var2,-carry])
		
		# output == var1 xor var 2
		self.add_a_x_clause([var1,var2,-output])

	# function for creating a full-adder circuit
	def FullAdder(self,var1,var2,var3,output,carry):
		var1 = self.CNF_variables[var1]
		var2 = self.CNF_variables[var2]
		var3 = self.CNF_variables[var3]
		carry = self.CNF_variables[carry]
		output = self.CNF_variables[output]
		
		# carry == sum(var1,var2,var3)>1  - ie, if any two are true/false, then the carry is true/false
		self.add_clause([var1,var2,-carry])
		self.add_clause([var1,var3,-carry])
		self.add_clause([var2,var3,-carry])
		self.add_clause([-var1,-var2,carry])
		self.add_clause([-var1,-var3,carry])
		self.add_clause([-var2,-var3,carry])
		
		# output == var1 xor var2 xor var3
		self.add_a_x_clause([var1,var2,var3,-output])

	# function for creating a full-adder circuit, discarding the carry
	def TerminatingAdder(self,var1,var2,var3,output):
		var1 = self.CNF_variables[var1]
		var2 = self.CNF_variables[var2]
		var3 = self.CNF_variables[var3]
		output = self.CNF_variables[output]
		
		# output == var1 xor var2 xor var3
		self.add_a_x_clause([var1,var2,var3,-output])


	# function for adding two integers
	def Add(self,v1,v2,v_output):
		v1_ = ["{}_{}".format(v1,i) for i in range(self.bit_resolution)]
		v2_ = ["{}_{}".format(v2,i) for i in range(self.bit_resolution)]
		v_output_ = ["{}_{}".format(v_output,i) for i in range(self.bit_resolution)]

		i = 0
		carry = "carry_{}_{}_{}".format(v1,v2,i)
		self.HalfAdder(v1_[i],v2_[i],v_output_[i],carry)  #half adder for the first bit pairs
		old_carry = carry
		i += 1
		while i<self.bit_resolution-1:
			carry = "carry_{}_{}_{}".format(v1,v2,i)
			self.FullAdder(v1_[i],v2_[i],old_carry,v_output_[i],carry) #full adder for the middle bit pairs, translating the carry
			old_carry = carry
			i += 1
		self.TerminatingAdder(v1_[i],v2_[i],old_carry,v_output_[i]) #terminating adder for the last bits, discarding the carry

	# function for gating an integer
	def Gated(self,v1,gate,output):
		v1_ = [self.CNF_variables["{}_{}".format(v1,i)] for i in range(self.bit_resolution)]
		v_output_ = [self.CNF_variables["{}_{}".format(output,i)] for i in range(self.bit_resolution)]
		gate_ = self.CNF_variables[gate]
		
		for i in range(self.bit_resolution):
			self.add_clause([-gate_,-v1_[i],v_output_[i]]) # if gate is true, then input <==> output
			self.add_clause([-gate_,v1_[i],-v_output_[i]])
			self.add_clause([gate_,-v_output_[i]]) # if gate is false, then output bit is false

	# function if integer from array of bits v1_ is greater than that of v2_ (assuming v1,v2 are positive)
	def Compare(self,v1_,v2_,vv1,vv2,g,l):
		length = len(v1_)
		assert len(v1_)==len(v2_)
		g = self.CNF_variables[g]
		l = self.CNF_variables[l]
		for i in range(length-1,-1,-1):
			v1i = self.CNF_variables["{}_{}".format(vv1,i)] # mark at this depth
			v2i = self.CNF_variables["{}_{}".format(vv2,i)]
			if i==length-1:
				self.add_clause([-v1_[i], v2_[i], v1i ]) # if difference at this depth, then mark at this depth appropriately
				self.add_clause([v1_[i],-v1i])	#ie. mark if and only if one bit is greater than the other
				self.add_clause([-v2_[i],-v1i])
				self.add_clause([v1_[i], -v2_[i], v2i ])
				self.add_clause([-v1_[i],-v2i])
				self.add_clause([v2_[i],-v2i])
			else:
				self.add_clause([-prev_v1i, v1i ]) # if marked previous depth, then mark at this depth
				self.add_clause([-prev_v2i, v2i ])
				self.add_clause([prev_v1i, -v1_[i], v2_[i], v1i ]) # if not marked at previous depth and if difference at this depth, then mark at this depth appropriately
				self.add_clause([prev_v1i, v1_[i], -v1i ]) # ie. mark if not otherwise previously marked and iff one bit is greater than another
				self.add_clause([prev_v1i, -v2_[i], -v1i ])
				self.add_clause([prev_v2i, v1_[i], -v2_[i], v2i ])
				self.add_clause([prev_v2i, -v1_[i], -v2i ])
				self.add_clause([prev_v2i, v2_[i], -v2i ])
			self.add_clause([-v1i, v2i, g]) # if difference in marks then set appropriate g/l flag
			self.add_clause([v1i, -v2i, l])
			prev_v1i = v1i
			prev_v2i = v2i
		self.add_clause([v1i,v2i,-g]) # if no flags set, then g&l flags are both false
		self.add_clause([v1i,v2i,-l])


	# hard assert the value of an item
	def AssertValue(self,v1,v):
		for i in range(self.bit_resolution):
			pos = 2*(v&1)-1
			self.add_clause([pos*self.CNF_variables["{}_{}".format(v1,i)]])
			v = v>>1

	# assert that an integer is zero
	def AssertIsZero(self,v1):
		v1_ = [self.CNF_variables["{}_{}".format(v1,i)] for i in range(self.bit_resolution)]
		for v in v1_:
			self.add_clause([-v])

	# assert that an integer is not zero
	def AssertIsNotZero(self,v1):
		v1_ = [self.CNF_variables["{}_{}".format(v1,i)] for i in range(self.bit_resolution)]
		self.add_clause(v1_)

	# assert that an integer is >0
	def AssertIsPositive(self,v1):
		v1_ = [self.CNF_variables["{}_{}".format(v1,i)] for i in range(self.bit_resolution)]
		self.add_clause(v1_)
		self.add_clause([-v1_[-1]])

	# assert that two integers are equal
	def AssertIsEqual(self,v1,v2):
		v1_ = [self.CNF_variables["{}_{}".format(v1,i)] for i in range(self.bit_resolution)]
		v2_ = [self.CNF_variables["{}_{}".format(v2,i)] for i in range(self.bit_resolution)]
		for i in range(self.bit_resolution):
			self.add_clause([-v1_[i],v2_[i]])
			self.add_clause([v1_[i],-v2_[i]])
			
	# function for if an integer is equal to +1
	def isOne(self,v1,t):
		t = self.CNF_variables[t]
		v1_ = [(-1 if i==0 else 1) * self.CNF_variables["{}_{}".format(v1,i)] for i in range(self.bit_resolution)]
		self.add_clause(v1_ +[t]) #if all variables align, then t is true
		for vv in v1_:
			self.add_clause([-vv,-t]) # if any variable does not align then t is untrue
	
	# function for if an integer is equal to -1
	def isNegOne(self,v1,t):
		t = self.CNF_variables[t]
		v1_ = [-1 * self.CNF_variables["{}_{}".format(v1,i)] for i in range(self.bit_resolution)]
		self.add_clause(v1_ +[t]) #if all variables align, then t is true
		for vv in v1_:
			self.add_clause([-vv,-t]) # if any variable does not align then t is untrue
	
	# function for if an integer is equal to 0
	def isZero(self,v1,t):
		t = self.CNF_variables[t]
		v1_ = [self.CNF_variables["{}_{}".format(v1,i)] for i in range(self.bit_resolution)]
		self.add_clause(v1_ +[t]) #if all variables align, then t is true
		for vv in v1_:
			self.add_clause([-vv,-t]) # if any variable does not align then t is untrue

	# function for if an integer has absolute value of 1
	def isAbsOne(self,v1,t):
		t_one = "{}_+1".format(t)
		t_minus_one = "{}_-1".format(t)
		self.isOne(v1,t_one)
		self.isNegOne(v1,t_minus_one)
		t = self.CNF_variables[t]
		t_one = self.CNF_variables[t_one]
		t_minus_one = self.CNF_variables[t_minus_one]
		# t is true iff t_one or t_minus_one - ie. t== not t_one and not t_minus_one
		self.add_clause([t_one,t_minus_one,-t])
		self.add_clause([-t_one,t])
		self.add_clause([-t_minus_one,t])
		
	# function if an integer is positive (ie >0)
	def isPositive(self,v1,t):
		is_zero = "{}_=0".format(t)
		self.isZero(v1,is_zero)
		is_zero = self.CNF_variables[is_zero]
		t = self.CNF_variables[t]
		is_negative = self.CNF_variables["{}_{}".format(v1,self.bit_resolution-1)]
		# t is true iff not isZero and not negative, ie. 
		self.add_clause([t, is_zero, is_negative])
		self.add_clause([-t,-is_zero])
		self.add_clause([-t,-is_negative])

	# conditions
	#   - the binary matrix multiplied by its integer inverse must equal the identity multiplied by some arbitrary positive integer
	#   - that the inverse matrix must have a variable with a absolute value of 1 in it  (prevents scaling problems)
	#   - that elementwise, if the matrix element is 0 then the inverse element is negative (or zero), if the matrix element is 1 then the inverse element is positive (or zero)
	#   - the binary matrix must be symmetric
	#   - the binary matrix must be symmetry broken wrt vertex swaps

	def write_matrix_identity_constraint(self):
		# the matrix multiplied by its inverse must be identity
		print("writing matrix identity constraints")
##		self.add_comment("MATRIX IDENTITY CONSTRAINTS")
		for col in tqdm(range(self.n)): # iterating over columns of the inverse matrix
			for row in range(self.n): # iterating over rows of the matrix
##				self.add_comment("ROW: {}, COLUMN: {}".format(row,col))
##				self.add_comment("Creating GATED ROW")
				for k in range(self.n): # k is iterator
					# gate the inverse element by the corresponding matrix element
					self.Gated("i_{}_{}".format(k,col),"m_{}_{}".format(row,k),"gi_{}_{}_{}".format(row,col,k))
				# perform the sum along the index
				self.AssertIsEqual("gi_{}_{}_{}".format(row,col,0),"gis_{}_{}_{}".format(row,col,0))
				for k in range(1,self.n):
					self.Add("gis_{}_{}_{}".format(row,col,k-1),"gi_{}_{}_{}".format(row,col,k),"gis_{}_{}_{}".format(row,col,k))
				# assert the result of the sum is zero if off-axis
				if col!=row:
					self.AssertIsZero("gis_{}_{}_{}".format(row,col,self.n-1))
				elif col==0: #else assert positive for first diagonal element
					self.AssertIsPositive("gis_{}_{}_{}".format(row,col,self.n-1))
				else: #else assert all diagonal elements are equal
					self.AssertIsEqual("gis_{}_{}_{}".format(row,col,self.n-1),"gis_{}_{}_{}".format(row-1,col-1,self.n-1))

	def write_identity_must_have_one_constraint(self):
		# that the inverse matrix must have an element with absolute value of 1
		is_abs_ones = []
		print("writing normaliseation of inverse matrix constraints")
		for col in tqdm(range(self.n)):
			for row in range(self.n):
				t = "abs1_{}_{}".format(row,col)
				is_abs_ones.append(t)
				self.isAbsOne("i_{}_{}".format(row,col),t)
		self.add_clause([self.CNF_variables[t] for t in is_abs_ones])

	# that elementwise, if the matrix element is 0 then the inverse element is negative (or zero), if the matrix element is 1 then the inverse element is positive (or zero)
	# ie. if inverse element is positive, then matrix element is 1, if inverse element is negative, then matrix element is 0
	def write_determinant_non_increasing_constraint(self):
		print("writing determinant non-decreasing constraint for elementwise change")
		for col in tqdm(range(self.n)):
			for row in range(self.n):
				v1 = "m_{}_{}".format(row,col)
				v2 = "i_{}_{}".format(row,col)
				v2_end = "{}_{}".format(v2,self.bit_resolution-1)
				v2_pos = "i_{}_{}_pos".format(row,col)
				self.isPositive(v2,v2_pos)
				self.add_clause([-self.CNF_variables[v2_pos],  self.CNF_variables[v1] ])
				self.add_clause([-self.CNF_variables[v2_end], -self.CNF_variables[v1] ])

	# that the matrix is an binary adjacency matrix with no self loops
	# ie, matrix is symmetric, with zero diagonal
	def write_matrix_symmetric_constraint(self):
		print("writing symmetry constraints")
		for col in tqdm(range(self.n)):
			for row in range(self.n):
				if row>col:
					# symmetric equality constraint
					v1 = "m_{}_{}".format(row,col)
					v2 = "m_{}_{}".format(col,row)
					self.add_clause([-self.CNF_variables[v1],self.CNF_variables[v2]])
					self.add_clause([self.CNF_variables[v1],-self.CNF_variables[v2]])
				if row==col:
					# diagonal is false
					self.add_clause([-self.CNF_variables["m_{}_{}".format(row,col)] ])
					
	# that the inverse matrix is an is symmetric, redundant constraint
	def write_inverse_matrix_symmetric_constraint(self):
		print("writing inverse symmetry constraints")
		for col in tqdm(range(self.n)):
			for row in range(self.n):
				if row>col:
					# symmetric equality constraint
					for k in range(self.bit_resolution):
						v1 = "i_{}_{}_{}".format(row,col,k)
						v2 = "i_{}_{}_{}".format(col,row,k)
						self.add_clause([-self.CNF_variables[v1],self.CNF_variables[v2]])
						self.add_clause([self.CNF_variables[v1],-self.CNF_variables[v2]])

	# the binary matrix must be symmetry broken wrt vertex swaps
	def write_matrix_vertex_symmetry_breaking(self):
		print("writing vertex symmetry constrints")
		for row in tqdm(range(self.n-1)):
			v1_ = [self.CNF_variables["m_{}_{}".format(row,col)] for col in range(self.n) if col!=row and col!=row+1]
			v2_ = [self.CNF_variables["m_{}_{}".format(row+1,col)] for col in range(self.n) if col!=row and col!=row+1]
			#v1_.reverse()
			#v2_.reverse()
			g = "comp_row_{}_{}_g".format(row,row+1)
			l = "comp_row_{}_{}_l".format(row,row+1)
			self.Compare(v1_,v2_,g,l,g,l)
			self.add_clause([-self.CNF_variables[g]])

	def write_all_constraints(self):
		# setup the bit array
		for x in range(n):
			for y in range(n):
				v = "m_{}_{}".format(x,y)
				self.CNF_variables[v]
				#self.interesting_variables.append(v)
		self.interesting_matricies.append("m")
		# setup the inverse array
		for x in range(n):
			for y in range(n):
				v = "i_{}_{}".format(x,y)
				#self.interesting_integers.append(v)
				for b in range(bit_resolution):
					self.CNF_variables["{}_{}".format(v,b)]
		self.interesting_matricies.append("i")
		
		self.write_matrix_identity_constraint()
		self.write_identity_must_have_one_constraint()
		self.write_determinant_non_increasing_constraint()
		self.write_matrix_symmetric_constraint()
		self.write_inverse_matrix_symmetric_constraint()
		self.write_matrix_vertex_symmetry_breaking()
		
	def output_CNF_MAP(self,cnf_name,map_name):
		print("outputting CNF and MAP")
		with open(cnf_name, "w") as f:
			f.write("p cnf {} {}\n".format(len(self.CNF_variables.keys()), len(self.cnf)))
			for c in tqdm(self.cnf):
				f.write("{}\n".format(c))
		with open(map_name,"w") as f:
			for k,v in tqdm(self.CNF_variables.items()):
				f.write("{} {}\n".format(k,v))

	def interpret_solution(self,solution):
		solution = {abs(s): 1 if s>0 else 0 for s in solution}
		for integer in self.interesting_integers:
			val = 0
			val_string = ''
			for k in list(range(self.bit_resolution))[::-1]:
				key = "{}_{}".format(integer,k)
				val = (val<<1) + solution[self.CNF_variables[key]]
				val_string += str(solution[self.CNF_variables[key]])
			print("{}: {} - {}".format(integer,val,val_string))
		for variable in self.interesting_variables:
			print("{}: {}".format(variable,solution[self.CNF_variables[variable]]))
		for matrix in self.interesting_matricies:
			if "{}_{}_{}".format(matrix,0,0) in self.CNF_variables.keys():
				print(matrix)
				for row in range(self.n):
					for col in range(self.n):
						key = "{}_{}_{}".format(matrix,row,col)
						print(solution[self.CNF_variables[key]], end=" ")
					print("")
			elif "{}_{}_{}_{}".format(matrix,0,0,0) in self.CNF_variables.keys():
				print(matrix)
				for row in range(self.n):
					for col in range(self.n):
						integer = "{}_{}_{}".format(matrix,row,col)
						val = 0
						for k in list(range(self.bit_resolution))[::-1]:
							key = "{}_{}".format(integer,k)
							val = (val<<1) + solution[self.CNF_variables[key]]
						# convert value from 2's complement
						if (val & (1 << (self.bit_resolution - 1))) != 0:
							val = val - (1 << self.bit_resolution)
						print("{}".format(val), end=" ")
					print("")

n = 7
bit_resolution = 10
problem = Problem(n,bit_resolution)
problem.write_all_constraints()
problem.output_CNF_MAP("cnf.txt","map.txt")


os.system("cryptominisat5 cnf.txt >zog.txt")
with open("zog.txt",'r') as f:
	solution = [l[1:] for l in f.readlines() if l[0]=='v']
assert len(solution)>0, "no SOLUTION"
solution = [[s for s in sol.strip().split(" ") if len(s)>0] for sol in solution]
solution = sum(solution,[])
solution = [int(s) for s in solution]
#print(solution)
problem.interpret_solution(solution)


'''def contract_sequence(seq):
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


matrix_keys = []
for x in range(n):
	for y in range(n):
		matrix_keys.append(problem.CNF_variables["m_{}_{}".format(x,y)])

with open("dag.txt","w") as f:
	f.write("DAG-FILE\nNODES:1\nGRAPH:\nCLAUSES:\n")
	f.write("0:0-{}\n".format(len(problem.cnf)-1))
	f.write("REPORTING:\n")
	f.write(contract_sequence(matrix_keys))
	f.write("\n")
	f.close()'''




