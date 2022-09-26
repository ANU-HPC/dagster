# beginning to setup CNF
from collections import defaultdict
from tqdm import tqdm
import os
import click


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


subblock_size = 3#5


class Problem(object):
	CNF_variables = None
	cnf = None
	n = None
	bit_resolution = None
	interesting_integers = None
	interesting_variables = None
	interesting_matricies = None
	node_parts = None
	
	variable_index = None
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
		self.node_parts = defaultdict(list)
		self.node_variables = defaultdict(set)

	def add_clause(self,clause, nodes):
		for n in nodes:
			self.node_parts[n].append(len(self.cnf))
			for s in clause:
				self.node_variables[n].add(abs(s))
		self.cnf.append(" ".join([str(s) for s in clause])+" 0")
	#def add_x_clause(self,clause):
	#	self.cnf.append("x"+" ".join([str(s) for s in clause])+" 0")
	def add_a_x_clause(self,clause, nodes):
		for i in range(2**(len(clause)-1)):
			c = ""
			mod = 1
			for j in range(len(clause)-1):
				polarity = 1 if i&(1<<j) else -1
				mod *= polarity
				c = c + str(polarity*clause[j]) + " "
			c = c + "{} 0".format(mod*clause[-1])
			for n in nodes:
				self.node_parts[n].append(len(self.cnf))
				for s in clause:
					self.node_variables[n].add(abs(s))
			self.cnf.append(c)

	# function for creating a half-adder circuit
	def HalfAdder(self,var1,var2,output,carry,nodes):
		var1 = self.CNF_variables[var1]
		var2 = self.CNF_variables[var2]
		carry = self.CNF_variables[carry]
		output = self.CNF_variables[output]
		
		# carry == var1 and var 2
		self.add_clause([-var1,-var2,carry],nodes)
		self.add_clause([var1,-carry],nodes)
		self.add_clause([var2,-carry],nodes)
		
		# output == var1 xor var 2
		self.add_a_x_clause([var1,var2,-output],nodes)

	# function for creating a full-adder circuit
	def FullAdder(self,var1,var2,var3,output,carry,nodes):
		var1 = self.CNF_variables[var1]
		var2 = self.CNF_variables[var2]
		var3 = self.CNF_variables[var3]
		carry = self.CNF_variables[carry]
		output = self.CNF_variables[output]
		
		# carry == sum(var1,var2,var3)>1  - ie, if any two are true/false, then the carry is true/false
		self.add_clause([var1,var2,-carry],nodes)
		self.add_clause([var1,var3,-carry],nodes)
		self.add_clause([var2,var3,-carry],nodes)
		self.add_clause([-var1,-var2,carry],nodes)
		self.add_clause([-var1,-var3,carry],nodes)
		self.add_clause([-var2,-var3,carry],nodes)
		
		# output == var1 xor var2 xor var3
		self.add_a_x_clause([var1,var2,var3,-output],nodes)

	# function for creating a full-adder circuit, discarding the carry
	def TerminatingAdder(self,var1,var2,var3,output,nodes):
		var1 = self.CNF_variables[var1]
		var2 = self.CNF_variables[var2]
		var3 = self.CNF_variables[var3]
		output = self.CNF_variables[output]
		
		# output == var1 xor var2 xor var3
		self.add_a_x_clause([var1,var2,var3,-output],nodes)
		
		# no overflow conditions
		self.add_clause([-var1,-var2,output],nodes) # add no overflow constraint, negative + negative, -> negative result
		self.add_clause([var1,var2,-output],nodes) # add no overflow constraint, positive + positive, -> positive result

	# function for adding two integers
	def Add(self,v1,v2,v_output,nodes):
		v1_ = ["{}_{}".format(v1,i) for i in range(self.bit_resolution)]
		v2_ = ["{}_{}".format(v2,i) for i in range(self.bit_resolution)]
		v_output_ = ["{}_{}".format(v_output,i) for i in range(self.bit_resolution)]

		i = 0
		carry = "carry_{}_{}_{}".format(v1,v2,i)
		self.HalfAdder(v1_[i],v2_[i],v_output_[i],carry,nodes)  #half adder for the first bit pairs
		old_carry = carry
		i += 1
		while i<self.bit_resolution-1:
			carry = "carry_{}_{}_{}".format(v1,v2,i)
			self.FullAdder(v1_[i],v2_[i],old_carry,v_output_[i],carry,nodes) #full adder for the middle bit pairs, translating the carry
			old_carry = carry
			i += 1
		self.TerminatingAdder(v1_[i],v2_[i],old_carry,v_output_[i],nodes) #terminating adder for the last bits, discarding the carry

	# function for gating an integer
	def Gated(self,v1,gate,output,nodes):
		v1_ = [self.CNF_variables["{}_{}".format(v1,i)] for i in range(self.bit_resolution)]
		v_output_ = [self.CNF_variables["{}_{}".format(output,i)] for i in range(self.bit_resolution)]
		gate_ = self.CNF_variables[gate]
		
		for i in range(self.bit_resolution):
			self.add_clause([-gate_,-v1_[i],v_output_[i]],nodes) # if gate is true, then input <==> output
			self.add_clause([-gate_,v1_[i],-v_output_[i]],nodes)
			self.add_clause([gate_,-v_output_[i]],nodes) # if gate is false, then output bit is false

	# function if integer from array of bits v1_ is greater than that of v2_ (assuming v1,v2 are positive)
	def Compare(self,v1_,v2_,vv1,vv2,g,l,nodes):
		length = len(v1_)
		assert len(v1_)==len(v2_)
		g = self.CNF_variables[g]
		l = self.CNF_variables[l]
		for i in range(length-1,-1,-1):
			v1i = self.CNF_variables["{}_{}".format(vv1,i)] # mark at this depth
			v2i = self.CNF_variables["{}_{}".format(vv2,i)]
			if i==length-1:
				self.add_clause([-v1_[i], v2_[i], v1i ],nodes) # if difference at this depth, then mark at this depth appropriately
				self.add_clause([v1_[i],-v1i],nodes)	#ie. mark if and only if one bit is greater than the other
				self.add_clause([-v2_[i],-v1i],nodes)
				self.add_clause([v1_[i], -v2_[i], v2i ],nodes)
				self.add_clause([-v1_[i],-v2i],nodes)
				self.add_clause([v2_[i],-v2i],nodes)
			else:
				self.add_clause([-prev_v1i, v1i ],nodes) # if marked previous depth, then mark at this depth
				self.add_clause([-prev_v2i, v2i ],nodes)
				self.add_clause([prev_v1i, -v1_[i], v2_[i], v1i ],nodes) # if not marked at previous depth and if difference at this depth, then mark at this depth appropriately
				self.add_clause([prev_v1i, v1_[i], -v1i ],nodes) # ie. mark if not otherwise previously marked and iff one bit is greater than another
				self.add_clause([prev_v1i, -v2_[i], -v1i ],nodes)
				self.add_clause([prev_v2i, v1_[i], -v2_[i], v2i ],nodes)
				self.add_clause([prev_v2i, -v1_[i], -v2i ],nodes)
				self.add_clause([prev_v2i, v2_[i], -v2i ],nodes)
			self.add_clause([-v1i, v2i, g],nodes) # if difference in marks then set appropriate g/l flag
			self.add_clause([v1i, -v2i, l],nodes)
			prev_v1i = v1i
			prev_v2i = v2i
		self.add_clause([v1i,v2i,-g],nodes) # if no flags set, then g&l flags are both false
		self.add_clause([v1i,v2i,-l],nodes)


	# hard assert the value of an item
	def AssertValue(self,v1,v,nodes):
		for i in range(self.bit_resolution):
			pos = 2*(v&1)-1
			self.add_clause([pos*self.CNF_variables["{}_{}".format(v1,i)]],nodes)
			v = v>>1

	# assert that an integer is zero
	def AssertIsZero(self,v1,nodes):
		v1_ = [self.CNF_variables["{}_{}".format(v1,i)] for i in range(self.bit_resolution)]
		for v in v1_:
			self.add_clause([-v],nodes)

	# assert that an integer is not zero
	def AssertIsNotZero(self,v1,nodes):
		v1_ = [self.CNF_variables["{}_{}".format(v1,i)] for i in range(self.bit_resolution)]
		self.add_clause(v1_,nodes)

	# assert that an integer is >0
	def AssertIsPositive(self,v1,nodes):
		v1_ = [self.CNF_variables["{}_{}".format(v1,i)] for i in range(self.bit_resolution)]
		self.add_clause(v1_,nodes)
		self.add_clause([-v1_[-1]],nodes)

	# assert that two integers are equal
	def AssertIsEqual(self,v1,v2,nodes):
		v1_ = [self.CNF_variables["{}_{}".format(v1,i)] for i in range(self.bit_resolution)]
		v2_ = [self.CNF_variables["{}_{}".format(v2,i)] for i in range(self.bit_resolution)]
		for i in range(self.bit_resolution):
			self.add_clause([-v1_[i],v2_[i]],nodes)
			self.add_clause([v1_[i],-v2_[i]],nodes)
			
	# function for if an integer is equal to +1
	def isOne(self,v1,t,nodes):
		t = self.CNF_variables[t]
		v1_ = [(-1 if i==0 else 1) * self.CNF_variables["{}_{}".format(v1,i)] for i in range(self.bit_resolution)]
		self.add_clause(v1_ +[t],nodes) #if all variables align, then t is true
		for vv in v1_:
			self.add_clause([-vv,-t],nodes) # if any variable does not align then t is untrue
	
	# function for if an integer is equal to -1
	def isNegOne(self,v1,t,nodes):
		t = self.CNF_variables[t]
		v1_ = [-1 * self.CNF_variables["{}_{}".format(v1,i)] for i in range(self.bit_resolution)]
		self.add_clause(v1_ +[t],nodes) #if all variables align, then t is true
		for vv in v1_:
			self.add_clause([-vv,-t],nodes) # if any variable does not align then t is untrue
	
	# function for if an integer is equal to 0
	def isZero(self,v1,t,nodes):
		t = self.CNF_variables[t]
		v1_ = [self.CNF_variables["{}_{}".format(v1,i)] for i in range(self.bit_resolution)]
		self.add_clause(v1_ +[t],nodes) #if all variables align, then t is true
		for vv in v1_:
			self.add_clause([-vv,-t],nodes) # if any variable does not align then t is untrue

	# function for if an integer has absolute value of 1
	def isAbsOne(self,v1,t,nodes):
		t_one = "{}_+1".format(t)
		t_minus_one = "{}_-1".format(t)
		self.isOne(v1,t_one)
		self.isNegOne(v1,t_minus_one)
		t = self.CNF_variables[t]
		t_one = self.CNF_variables[t_one]
		t_minus_one = self.CNF_variables[t_minus_one]
		# t is true iff t_one or t_minus_one - ie. t== not t_one and not t_minus_one
		self.add_clause([t_one,t_minus_one,-t],nodes)
		self.add_clause([-t_one,t],nodes)
		self.add_clause([-t_minus_one,t],nodes)

	# function if an integer is positive (ie >0)
	def isPositive(self,v1,t,nodes):
		is_zero = "{}_=0".format(t)
		self.isZero(v1,is_zero,nodes)
		is_zero = self.CNF_variables[is_zero]
		t = self.CNF_variables[t]
		is_negative = self.CNF_variables["{}_{}".format(v1,self.bit_resolution-1)]
		# t is true iff not isZero and not negative, ie. 
		self.add_clause([t, is_zero, is_negative],nodes)
		self.add_clause([-t,-is_zero],nodes)
		self.add_clause([-t,-is_negative],nodes)

	# conditions
	#   - the binary matrix multiplied by its integer inverse must equal the identity multiplied by some arbitrary positive integer
	#   - that elementwise, if the matrix element is 0 then the inverse element is negative (or zero), if the matrix element is 1 then the inverse element is positive (or zero)
	#   - the binary matrix must be symmetric
	#   - the binary matrix must be symmetry broken wrt vertex swaps

	def write_matrix_identity_constraint(self):
		# the matrix multiplied by its inverse must be identity
		print("writing matrix identity constraints")
		for col in tqdm(range(self.n)): # iterating over columns of the inverse matrix
			for row in range(self.n): # iterating over rows of the matrix
				#nodes = list(range(2))
				#nodes = [1] + ([0] if ((row < 3) and (col < 3)) else [])
				nodes = [1]
				for k in range(self.n): # k is iterator
					# gate the inverse element by the corresponding matrix element
					self.Gated("i_{}_{}".format(k,col),"m_{}_{}".format(row,k),"gi_{}_{}_{}".format(k,row,col), nodes )
				# perform the sum along the index
				self.AssertIsEqual("gi_{}_{}_{}".format(0,row,col),"gis_{}_{}_{}".format(0,row,col), nodes )
				for k in range(1,self.n):
					self.Add("gis_{}_{}_{}".format(k-1,row,col),"gi_{}_{}_{}".format(k,row,col),"gis_{}_{}_{}".format(k,row,col), nodes )
				# assert the result of the sum is zero if off-axis
				if col!=row:
					self.AssertIsZero("gis_{}_{}_{}".format(self.n-1,row,col), nodes)
				elif col==0: #else assert positive for first diagonal element
					self.AssertIsPositive("gis_{}_{}_{}".format(self.n-1,row,col), nodes)
				else: #else assert all diagonal elements are equal
					self.AssertIsEqual("gis_{}_{}_{}".format(self.n-1,row,col),"gis_{}_{}_{}".format(self.n-1,row-1,col-1), nodes)

	# that elementwise, if the matrix element is 0 then the inverse element is negative (or zero), if the matrix element is 1 then the inverse element is positive (or zero)
	# ie. if inverse element is positive, then matrix element is 1, if inverse element is negative, then matrix element is 0
	def write_determinant_non_increasing_constraint(self):
		print("writing determinant non-decreasing constraint for elementwise change")
		for row in tqdm(range(self.n)):
			for col in range(self.n):
				#nodes = list(range(row,self.n))
				#nodes = list(range(row*row,self.n-1)) +[self.n-1]
				nodes = [1]
				v1 = "m_{}_{}".format(row,col)
				v2 = "i_{}_{}".format(row,col)
				v2_end = "{}_{}".format(v2,self.bit_resolution-1)
				v2_pos = "i_{}_{}_pos".format(row,col)
				self.isPositive(v2,v2_pos,nodes)
				self.add_clause([-self.CNF_variables[v2_pos],  self.CNF_variables[v1] ],nodes)
				self.add_clause([-self.CNF_variables[v2_end], -self.CNF_variables[v1] ],nodes)

	# that the matrix is an binary adjacency matrix with no self loops
	# ie, matrix is symmetric, with zero diagonal
	def write_matrix_symmetric_constraint(self):
		print("writing symmetry constraints")
		for col in tqdm(range(self.n)):
			for row in range(self.n):
				nodes = [1] + ([0] if ((row < subblock_size) and (col < subblock_size)) else [])
				if row>col:
					# symmetric equality constraint
					v1 = "m_{}_{}".format(row,col)
					v2 = "m_{}_{}".format(col,row)
					self.add_clause([-self.CNF_variables[v1],self.CNF_variables[v2]],nodes)
					self.add_clause([self.CNF_variables[v1],-self.CNF_variables[v2]],nodes)
				if row==col:
					# diagonal is false
					self.add_clause([-self.CNF_variables["m_{}_{}".format(row,col)] ],nodes)
	
	# that the inverse matrix is an is symmetric, redundant constraint
	def write_inverse_matrix_symmetric_constraint(self):
		print("writing inverse symmetry constraints")
		for col in tqdm(range(self.n)):
			for row in range(self.n):
				nodes = [1]
				if row>col:
					# symmetric equality constraint
					for k in range(self.bit_resolution):
						v1 = "i_{}_{}_{}".format(row,col,k)
						v2 = "i_{}_{}_{}".format(col,row,k)
						self.add_clause([-self.CNF_variables[v1],self.CNF_variables[v2]],nodes)
						self.add_clause([self.CNF_variables[v1],-self.CNF_variables[v2]],nodes)

	# the binary matrix must be symmetry broken wrt vertex swaps
	def write_matrix_vertex_symmetry_breaking(self):
		print("writing vertex symmetry constrints")
		nodes = [1]
		for row1 in tqdm(range(self.n-1)):
			for row2 in range(row1+1,self.n):
				v1_ = [self.CNF_variables["m_{}_{}".format(row1,col)] for col in range(self.n) if col!=row1 and col!=row2]
				v2_ = [self.CNF_variables["m_{}_{}".format(row2,col)] for col in range(self.n) if col!=row1 and col!=row2]
				#v1_.reverse()
				#v2_.reverse()
				g = "comp_row_read_{}_{}_g".format(row1,row2)
				l = "comp_row_read_{}_{}_l".format(row1,row2)
				self.Compare(v1_,v2_,g,l,g,l,nodes)
				self.add_clause([-self.CNF_variables[g]],nodes)

	# the binary matrix must have unique columns (if it were otherwise the determinant would be zero, which would mean that there could be no inverse)
	def write_matrix_no_columns_equal(self):
		print("writing matrix column pairwise unique")
		nodes = [1]
		for row1 in tqdm(range(self.n-1)):
			for row2 in range(row1+1,self.n):
				v1_ = [self.CNF_variables["m_{}_{}".format(row1,col)] for col in range(self.n)]
				v2_ = [self.CNF_variables["m_{}_{}".format(row2,col)] for col in range(self.n)]
				g = "comp_row_{}_{}_g".format(row1,row2)
				l = "comp_row_{}_{}_l".format(row1,row2)
				self.Compare(v1_,v2_,g,l,g,l,nodes)
				self.add_clause([self.CNF_variables[g], self.CNF_variables[l]],nodes)

	# the binary matrix must have no zero columns (if it were otherwise the determinant would be zero, which would mean that there could be no inverse)
	def write_matrix_no_zero_columns(self):
		print("writing matrix no non-zero columns")
		nodes = [1]
		for col in tqdm(range(self.n)):
			is_zeros = []
			for row in range(self.n):
				is_zeros.append("m_{}_{}".format(row,col))
			self.add_clause([-self.CNF_variables[t] for t in is_zeros],nodes)

	# that the binary matrix is not a block diagonal matrix, ie. there is no block of zeros off the diagonal
	def write_matrix_is_not_block_diagonal(self):
		print("writing matrix is not block diagonal constraint")
		nodes = [1]
		for k in tqdm(range(self.n-1)):
			zero_block = []
			for col in range(k+1,self.n):
				for row in range(0,k+1):
					zero_block.append(self.CNF_variables["m_{}_{}".format(row,col)])
			self.add_clause(zero_block,nodes)
		
	def write_all_constraints(self):
		# setup the bit array
		for x in range(self.n):
			for y in range(self.n):
				v = "m_{}_{}".format(x,y)
				self.CNF_variables[v]
				#self.interesting_variables.append(v)
		self.interesting_matricies.append("m")
		# setup the inverse array
		for x in range(self.n):
			for y in range(self.n):
				v = "i_{}_{}".format(x,y)
				#self.interesting_integers.append(v)
				for b in range(self.bit_resolution):
					self.CNF_variables["{}_{}".format(v,b)]
		self.interesting_matricies.append("i")
		
		self.write_matrix_identity_constraint()
		self.write_determinant_non_increasing_constraint()
		self.write_matrix_symmetric_constraint()
		self.write_inverse_matrix_symmetric_constraint()
		self.write_matrix_vertex_symmetry_breaking()
		self.write_matrix_no_columns_equal()
		self.write_matrix_no_zero_columns()
		self.write_matrix_is_not_block_diagonal()
		
		#A = [-1,2,-3,-4,-5,-6,7,-8,-9,-10,-11,-12,-13,-14,-15,-16,-17,18,-19,-20,-21,-22,23,24,-25,-26,-27,28,-29,30,-31,-32,33,34,35,-36]
		#for a in A:
		#	self.add_clause([a])

	def output_CNF_MAP(self,cnf_name,map_name):
		print("outputting CNF and MAP")
		with open(cnf_name, "w") as f:
			matrix_variables = []
			for x in range(self.n):
				for y in range(self.n):
					v = "m_{}_{}".format(x,y)
					matrix_variables.append(str(self.CNF_variables[v]))
			f.write("c ind {} 0\n".format(" ".join(matrix_variables) ))
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


@click.command()
@click.argument('n', type=click.INT)
@click.argument('bit_resolution', type=click.INT, default=10)
@click.argument('cnf_file', type=click.types.Path(), default="cnf.txt")
@click.argument('map_file', type=click.types.Path(), default="map.txt")
@click.argument('dag_file', type=click.types.Path(), default="dag.txt")
def determinant_problem(n, bit_resolution, cnf_file, map_file, dag_file):
	problem = Problem(n,bit_resolution)
	problem.write_all_constraints()
	problem.output_CNF_MAP(cnf_file,map_file)
	
	global subblock_size
	subblock_size = n-2

	'''os.system("cryptominisat5 {} >zog.txt".format(cnf_file))
	with open("zog.txt",'r') as f:
		solution = [l[1:] for l in f.readlines() if l[0]=='v']
	assert len(solution)>0, "no SOLUTION"
	solution = [[s for s in sol.strip().split(" ") if len(s)>0] for sol in solution]
	solution = sum(solution,[])
	solution = [int(s) for s in solution]
	problem.interpret_solution(solution)'''
	
	matrix_keys = []
	for x in range(n):
		for y in range(n):
			matrix_keys.append(problem.CNF_variables["m_{}_{}".format(x,y)])
	max_node_key = max(problem.node_parts.keys())

	with open(dag_file,"w") as f:
		f.write("DAG-FILE\nNODES:{}\nGRAPH:\n".format(max_node_key+1))
		for k in range(1,max_node_key+1):
			#f.write("{}->{}:{}\n".format(k-1,k,contract_sequence(problem.node_variables[k-1] ) ))
			f.write("{}->{}:{}\n".format(k-1,k,contract_sequence(matrix_keys) ))
		f.write("CLAUSES:\n")
		for k in sorted(problem.node_parts.keys()):
			f.write("{}:{}\n".format(k,contract_sequence(problem.node_parts[k] ) ))
		f.write("REPORTING:\n")
		f.write(contract_sequence(matrix_keys))
		f.write("\n")
		f.close()

if __name__ == '__main__':
	determinant_problem()
