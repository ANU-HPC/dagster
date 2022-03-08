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


# DAGIFY script:
#
# from a <CNF> file generate a <DAG> file using the 'min-cut algorithm' with <DESIRED_NODES> nodes appropriate for input into dagster program
#

try:
	import click
	import networkx as nx
	from networkx.algorithms.dag import transitive_reduction
	from networkx.algorithms.dag import topological_sort
	from networkx.drawing.nx_pydot import write_dot
	from math import exp
	from tqdm import tqdm
	import operator
	from random import sample, choice
except ImportError:
	print("Welcome to Dagify script")
	print("you need to install working python libraries: networkx,click,tqdm\n")
	raise

try:
	from utils import RepresentsInt, compact_list_of_integers, output_image, output_dot
except ImportError:
	print("Welcome to Dagify script")
	print("cannot find utils.py\n")
	raise


# for a cnf, return a dictionary in which for each litteral, there is its
# neighbhorhood litterals, and the clauses in which they occur
def get_cuts(cnf):
	cut_dict = {}
	for index,c in cnf:
		for variable in c:
			if variable not in cut_dict.keys():
				cut_dict[variable] = {"neighborhood":set(),'cnfs':[]}
			cut = cut_dict[variable]
			cut["cnfs"].append((index,c))
			for cc in c:
				cut["neighborhood"].add(cc)
	return cut_dict

# for a cnf and a specific variable, return
# neighbhorhood litterals, and the clauses in which they occur
def get_cut(cnf,variable):
	cut = {"neighborhood":set(),'cnfs':[]}
	for index,c in cnf:
		if variable in c:
			cut["cnfs"].append((index,c))
			for cc in c:
				cut["neighborhood"].add(cc)
	return cut




@click.command()
@click.argument('cnf', type=click.File('r'))
@click.argument('dag', type=click.File('w'))
@click.argument('desired_nodes', type=click.INT)
@click.option('--icf', default=1.0, help='the factor by which the algorithm will prioritise minimising incomming branches to a node', type=float)
@click.option('--ocf', default=1.0, help='the factor by which the algorithm will prioritise minimising outgoing branches to a node', type=float)
@click.option('--continuous_reduction', is_flag=True, help="is a flag that instructs dagify to do transitive reduction in the process of minimising the dag it tends to produce dags which are less branching, but is slightly more expensive computationally")
@click.option('--not_pass_all_data', is_flag=True, help="is a flag which instructs dagify to make it such that nodes only pass variables onwards if future nodes need them not reccommended for reporting of solutions X-(")
@click.option('--dot_output', default=None, type=str, help="a filename to output an image displaying the connection graph dag structure")
@click.option('--image_output', default=None, type=str, help="a filename to output an image to, showing the min-fill association between the variables, hopefully problem structure should be visible")
@click.option('--dag_reduction_sample_size', default=None, type=int, help="is a option to specify how much to sample for the best places to minimise when the dag is being compacted unspecified means a complete search, specify to create a less perfect search at the prospect of faster dag generation")
@click.option('--min_fill_sample_size', default=None, type=int, help="is a option to specify how much to sample for the best variables to eliminate in the min-fill algorithm unspecified means a complete search, specify to create a less perfect dag at the prospect of faster dag generation")
def dagify(cnf, dag, desired_nodes, icf, ocf, continuous_reduction, not_pass_all_data, dot_output, image_output, dag_reduction_sample_size, min_fill_sample_size):
	"""from a <CNF> file generate a <DAG> file with <DESIRED_NODES> nodes appropriate for input into dagster program"""
	print("Welcome to Dagify script")
	max_variable = 0

	# parse the cnf file, and absolute value all the literals
	print("parsing CNF file")
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
	cnf = list(enumerate(cnf))
	
	#begin to generate a dag by instigating the min-fill algorithm
	# in which each turn a selected litteral (and it associated clauses) are eliminated from the cnf
	# and itteratively placed into a graph structure, where edges indicate shared neighborhoods
	dg = nx.DiGraph()
	print("Generating dag by min-fill algorithm")
	old_t = 0
	with tqdm(total = original_cnf_length) as t:
		while len(cnf)!=0:
			new_t = original_cnf_length-len(cnf)
			t.update(new_t - old_t)
			old_t = new_t
			# find the variable which has the smallest neighborhood in the remaining clauses
			if min_fill_sample_size is None:
				cuts = get_cuts(cnf) #scan all variables
			else: # otherwise create a sample of variable cuts
				#import pdb
				#pdb.set_trace()
				sample_variables = set([choice(choice(cnf)[1]) for i in range(min_fill_sample_size)])
				cuts = {v:get_cut(cnf,v) for v in sample_variables}
			min_cut_variable = min([(len(cut["neighborhood"]),variable) for variable,cut in cuts.items()])[1]
			# add the cut to the dag, and make all the edges where appropriate
			tree_node_index = len(dg.nodes)
			dg.add_node(tree_node_index)
			for i,l in enumerate(list(dg.nodes.values())[:-1]):
				if min_cut_variable in l["neighborhood"]:
					dg.add_edge(i,tree_node_index)
			# add the associated cnfs and neighborhood to node attributes
			dg.nodes[tree_node_index].update(cuts[min_cut_variable])
			# purge the cnf of the clauses where the selected litteral occurs
			cnf = [c for c in cnf if min_cut_variable not in c[1]]

	# attempt to output an image giving the min-fill connections
	# (hopefully problem structure should be easily discernable)
	if image_output is not None:
		output_image(list(dg.edges), image_output)

	#add a terminal node if there isnt already a singularly defined one
	print("adding a terminal node if nessisary")
	edge_keys = list(dg.nodes.keys())
	terminals = [len(list(dg.successors(k)))==0 for k in edge_keys]
	if sum(terminals)>1: # if there is more than one terminal node on the dag, make a new terminal node linking all the others
		terminal_index = len(dg.nodes)
		dg.add_node(terminal_index)
		dg.nodes[terminal_index].update({"neighborhood":set(),'cnfs':[]})
		for i,k in enumerate(edge_keys):
			if terminals[i]:
				dg.add_edge(k,terminal_index)

	# do a transitive reduction on the graph, and reattach node data
	print("conducting transitive reduction on graph")
	node_data = dict(dg.nodes.items())
	dg = transitive_reduction(dg)
	nx.set_node_attributes(dg, node_data)

	# iteratively eliminate nodes in the dag by contracting edges, bassed on a weighted score
	# the score is greater for smaller cnf sizes and also contractions that reduce the number of network connections
	print("contracting edges to make smaller dag")
	original_num_nodes = len(dg.nodes)
	old_t = 0
	with tqdm(total= original_num_nodes-desired_nodes) as t:
		while len(dg.nodes) > desired_nodes:
			new_t = original_num_nodes - len(dg.nodes)
			t.update(new_t-old_t)
			old_t = new_t
			# do a scan of the dag to get the best edge
			best_edge = None
			best_edge_score = -float("inf")
			edge_list = list(dg.edges)
			if dag_reduction_sample_size is not None:
				edge_list = sample(edge_list,min(dag_reduction_sample_size,len(edge_list)))
			for i,j in edge_list:
				# the score is negative the number of clauses that will be attached to the contracted node
				score = -len(dg.nodes[i]['cnfs'])-len(dg.nodes[j]['cnfs'])
				prei = set(dg.predecessors(i))
				prej = set(dg.predecessors(j))
				suci = set(dg.successors(i))
				sucj = set(dg.successors(j))
				# the negative score is then exponentially decayed depending on how many network edges are eliminated
				score *= exp(-icf*(len(prei) + len(prej) - len(prei.union(prej))) - ocf*(len(suci) + len(sucj) - len(suci.union(sucj))))
				# a quick check to make sure the collapse will not create cycles
				#safe_to_collapse = len(list(nx.node_disjoint_paths(dg,i,j))) == 1
				dg.remove_edge(i,j)
				safe_to_collapse = not nx.has_path(dg,i,j)
				dg.add_edge(i,j)
				if safe_to_collapse and score>best_edge_score:
					best_edge = (i,j)
					best_edge_score = score
			# amalgamate the cnfs and neighborhood info prior to edge contraction
			data1 = dg.nodes[best_edge[0]]
			data2 = dg.nodes[best_edge[1]]
			new_data = {"cnfs":data1['cnfs']+data2['cnfs'],"neighborhood":data1['neighborhood'].union(data2['neighborhood'])}
			# do contraction and update cnfs/neighborhood info
			dg = nx.contracted_edge(dg, best_edge, self_loops=False)
			dg.nodes[best_edge[0]].clear()
			dg.nodes[best_edge[0]].update(new_data)

			# if flag continouse_reduction is on, then do transitive reduction each contracted edge.
			# tends to result in more linear dags
			if continuous_reduction:
				node_data = dict(dg.nodes.items())
				dg = transitive_reduction(dg)
				nx.set_node_attributes(dg, node_data)

	# do another transitive reduction on the graph, and reattach node data
	print("reconducting transitive reduction on graph")
	node_data = dict(dg.nodes.items())
	dg = transitive_reduction(dg)
	nx.set_node_attributes(dg, node_data)
	
	# define the litterals to be passed along all the edges of the dag
	print("assigning litterals along dag edges and relabelling nodes")
	order = list(topological_sort(dg))
	for parent in tqdm(order):
		parent_neighborhood = set(dg.nodes[parent]['neighborhood'])
		for grand_parent in dg.predecessors(parent):
			parent_neighborhood.update(dg[grand_parent][parent]['variables'])
		for child in dg.successors(parent):
			if not_pass_all_data:
				child_neighborhood = set(dg.nodes[child]['neighborhood'])
				for grand_child in nx.descendants(dg,child):
					child_neighborhood.update(dg.nodes[grand_child]['neighborhood'])
				dg[parent][child]['variables'] = parent_neighborhood.intersection(child_neighborhood)
			else:
				dg[parent][child]['variables'] = parent_neighborhood

	# relabel the nodes to be consequtive integers
	dg = nx.relabel_nodes(dg,dict([(i,j) for j,i in enumerate(order)]))
	
	# outputing a DOT file of the graph of the dag
	if dot_output is not None:
		print("generating DOT file for dag")
		output_dot(dg, dot_output)
	
	# writing dag file
	print("outputting dag file")
	dag.write("DAG-FILE\n")
	dag.write("NODES:{}\n".format(len(dg.nodes)))
	dag.write("GRAPH:\n")
	for a,b in sorted(list(dg.edges), key = operator.itemgetter(0, 1) ):
		dag.write("{}->{}:{}\n".format(a,b, compact_list_of_integers(dg[a][b]['variables'])))
	dag.write("CLAUSES:\n")
	for n in sorted(list(dg.nodes)):
		dag.write("{}:{}\n".format(n, compact_list_of_integers( [i for i,c in dg.nodes[n]['cnfs']] )  ))
	dag.write("REPORTING:\n")
	dag.write("{}-{}\n".format(1,max_variable))
	dag.close()

if __name__ == '__main__':
    dagify()
