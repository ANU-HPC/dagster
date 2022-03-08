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

try:
	#from pysat.formula import CNF
	import click
	from numpy.random import choice
	import numpy
	import random
	from itertools import product
	from tqdm import tqdm
except ImportError as e:
	print("Cannot import python libraries")
	raise

def generate_clause(variables, clause_size, weights):
	new_clause = choice(variables, clause_size, p=weights, replace=False).tolist()
	new_clause = [v*(random.randint(0,1)*2-1) for v in new_clause]
	return new_clause

@click.command()
@click.argument('size', type=click.INT)
@click.argument('overlap', type=click.INT)
@click.argument('n', type=click.INT)
@click.argument('clause_size', type=click.INT)
@click.argument('multiplier', type=click.INT)
@click.argument('cnf_file', type=click.STRING)
@click.argument('dag_file', type=click.STRING)
def gen_cnf(size, overlap, n, clause_size, multiplier, cnf_file, dag_file):
	#check input is sensible
	assert size>0
	assert overlap>0
	assert size>overlap
	assert n>0
	assert clause_size>1
	assert multiplier>0
	print("setting up variables")
	#setup variable partitions
	variables = [[j for j in range(1,size+1)] for i in range(n)]
	for i in range(n):
		variables[i] = [j+(size-overlap)*i for j in variables[i]]
	total_variables = list(set(sum(variables,[])))
	#for v1 in tqdm(variables):
	#	for vv in v1:
	#		if vv not in total_variables:
	#			total_variables.append(vv)
	print("generating random solution")
	total_variable_solution = [vv*(random.randint(0,1)*2-1) for vv in tqdm(total_variables)]
	total_variable_subsolution = [[v for v in total_variable_solution if abs(v) in vv] for vv in tqdm(variables)]
	total_variable_negated_subsolution = [[-v for v in vv] for vv in tqdm(total_variable_subsolution)]
	print("setting up weights")
	variable_counts = dict(zip(*numpy.unique(sum(variables,[]), return_counts=True)))
	variable_weights = [[(1.0+random.random())/(variable_counts[variables[i][j]]) for j in range(size)] for i in range(n)]
	variable_weights = [[j/sum(variable_weights[i]) for j in variable_weights[i]] for i in range(n)]
	clauses = [[] for i in range(n)]
	for new_clause_index in range(n):
		print("{} of {}".format(new_clause_index,n));
		for i in tqdm(range(multiplier)):
			for j,v in enumerate(total_variable_subsolution[new_clause_index]):
				restart = True
				while restart:
					clause = [v]
					new_variable_weights = variable_weights[new_clause_index][:]
					new_variable_weights[j] = 0
					sum_new_variable_weights = sum(new_variable_weights)
					new_variable_weights = [w*1.0/sum_new_variable_weights for w in new_variable_weights]
					clause = clause + generate_clause(total_variable_negated_subsolution[new_clause_index], clause_size-1, new_variable_weights)
					clause = sorted(clause)
					if clause not in clauses[new_clause_index]:
						clauses[new_clause_index].append(clause)
						restart = False
	vc = 0
	for c in clauses:
		for cc in c:
			for ccc in cc:
				if abs(ccc)>vc:
					vc = abs(ccc)
	with open(cnf_file,"w") as f:
		f.write("p cnf {} {}\n".format(vc,sum([len(cc) for cc in clauses])))
		for c in clauses:
			for cc in c:
				f.write(" ".join([str(ccc) for ccc in cc]))
				f.write(" 0\n")
	#cnf = CNF()
	#for i in range(n):
	#	for clause in clauses[i]:
	#		cnf.append(clause)
	#cnf.to_file(cnf_file)
	dag = open(dag_file,"w")
	dag.write("DAG-FILE\n")
	dag.write("NODES:{}\n".format(n))
	dag.write("GRAPH:\n")
	for i in range(1,n):
		dag.write("{}->{}:{}\n".format(i-1,i, ",".join([str((size-overlap)*i+j+1) for j in range(overlap)]) ))
	dag.write("CLAUSES:\n")
	for i in range(n):
		beginning = sum([len(c) for c in clauses[:i]])
		length = len(clauses[i])
		dag.write("{}:{}-{}\n".format(i, beginning,beginning+length-1 ))
	dag.write("REPORTING:\n")
	dag.write("{}-{}\n".format(variables[-1][0],variables[-1][-1]))
	dag.close()



if __name__ == '__main__':
	gen_cnf()

