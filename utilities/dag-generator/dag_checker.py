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
	import click
	import networkx as nx
	from networkx.algorithms.dag import topological_sort
	import sys
except ImportError:
	print("Welcome to Dag_check script")
	print("you need to install working python libraries: networkx,click\n")
	raise
try:
	from utils import RepresentsInt
except ImportError:
	print("Welcome to Dagify script")
	print("cannot find utils.py\n")
	raise



@click.command()
@click.argument('cnf', type=click.File('r'))
@click.argument('dag', type=click.File('r'))
def dag_check(cnf, dag):
	"""from a <CNF> file check a <DAG> do some sanity checks to make sure it is appropriately formed to model the cnf"""
	max_variable = 0
	print("parsing {} and {}".format(cnf.name,dag.name))

	# parse the cnf file, and absolute value all the literals
	#print("parsing CNF file")
	cnf_lines = [a.strip().split(' ') for a in cnf.readlines()]
	cnf.close()
	cnf = []
	for c in cnf_lines:
		int_sequence = []
		break_out = False
		for s in c:
			ss = RepresentsInt(s)
			if ss is None:
				break_out = True
				break
			if ss != 0:
				int_sequence.append(abs(ss))
				max_variable = max(max_variable,ss)
		if break_out == True:
			continue
		cnf.append(int_sequence)
	original_cnf_length = len(cnf)

	#tag each clause with an index
	#cnf = list(enumerate(cnf))


	dg = nx.DiGraph()
	total_clauses = set()
	total_litterals = set()

	# parse dag file into a networkx digraph
	#print ("parsing DAG file")
	dag_lines = [a.strip() for a in dag.readlines()]
	if dag_lines[0] != "DAG-FILE":
		print("dag file has inappropraite header")
		sys.exit(1)
	if dag_lines[1][0:5] != "NODES":
		print("dag file has inappropriate NODES header")
		sys.exit(1)
	nodes = int(dag_lines[1][6:])
	if dag_lines[2] != "GRAPH:":
		print("dag file has inappropriate GRAPH header")
		sys.exit(1)
	index = 3
	while (dag_lines[index]!="CLAUSES:"):
		line = dag_lines[index]
		line_parts = line.split(":")
		if len(line_parts)!=2:
			print("badly formatted dag graph")
			sys.exit(1)
		to_from = line_parts[0].split("->")
		to_from = [int(a) for a in to_from]
		litterals = line_parts[1].split(',')
		litterals = [l.split('-') for l in litterals]
		new_litterals = []
		for l in litterals:
			if len(l)==2:
				for i in range(int(l[0]),int(l[1])+1):
					new_litterals.append(i)
			else:
				new_litterals.append(int(l[0]))
		dg.add_node(to_from[0])
		dg.add_node(to_from[1])
		dg.add_edge(to_from[0],to_from[1])
		dg[to_from[0]][to_from[1]]['litterals'] = new_litterals
		total_litterals.update(set(new_litterals))
		index += 1
	if dag_lines[index] != "CLAUSES:":
		print("badly formatted dag graph")
		sys.exit(1)
	index += 1
	while (index != len(dag_lines)) and (dag_lines[index]!="REPORTING:"):
		line = dag_lines[index]
		line_parts = line.split(":")
		if len(line_parts)!=2:
			print("badly formatted clause graph")
			sys.exit(1)
		node = int(line_parts[0])
		clauses = line_parts[1].split(',')
		clauses = [l.split('-') for l in clauses]
		new_clauses = []
		for l in clauses:
			if len(l)==2:
				for i in range(int(l[0]),int(l[1])+1):
					new_clauses.append(i)
			else:
				new_clauses.append(int(l[0]))
		dg.nodes[node]['clauses'] = new_clauses
		total_clauses.update(set(new_clauses))
		index += 1


	# check all litterals and clauses are mentioned
	if original_cnf_length != len(total_clauses):
		print("cnf length mismatch")
		sys.exit(1)
	for i in total_clauses:
		if not (i>=0 and i<original_cnf_length):
			print("cnf length mismatch")
			sys.exit(1)
	if not (len(total_litterals) == max([max(c) for c in cnf])):
		print("cnf litteral mismatch")
		sys.exit(1)
	for i in total_litterals:
		if not (i>0 and i<=len(total_litterals)):
			print("cnf litteral mismatch")
			sys.exit(1)
	
	# for each node compile a list of all the litterals it will be feeding forward
	order = list(topological_sort(dg))
	for parent in order:
		parent_neighborhood = set()
		for grand_parent in dg.predecessors(parent):
			parent_neighborhood.update(dg[grand_parent][parent]['litterals'])
		dg.nodes[parent]['inherited_variables'] = parent_neighborhood
	for parent in order:
		neighborhood = set()
		for i in dg.nodes[parent]["clauses"]:
			neighborhood.update(set(cnf[i]))
		neighborhood.update(dg.nodes[parent]['inherited_variables'])
		dg.nodes[parent]['neighborhood'] = neighborhood
	
	#check that each child will pass on more variables than the parent
	for parent in order:
		for child in dg.successors(parent):
			if not dg.nodes[parent]['neighborhood'].issubset(dg.nodes[child]['neighborhood']):
				print("child neighborhood not subset")
				sys.exit(1)

	print("successfull dag")
	sys.exit(0)

if __name__ == '__main__':
    dag_check()
