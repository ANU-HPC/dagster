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

import fire
import networkx as nx
import re
from tqdm import tqdm
import os


def prepend_line(file_name, line):
    """ Insert given string as a new line at the beginning of a file """
    dummy_file = file_name + '.bak'
    with open(file_name, 'r') as read_obj, open(dummy_file, 'w') as write_obj:
        write_obj.write(line + '\n')
        for line in read_obj:
            write_obj.write(line)
    os.remove(file_name)
    os.rename(dummy_file, file_name)

def convert_adder(offset):
	def inner(m):
		return str(int(m.group(0))+offset)
	return inner

def add_to_variable(v,offset):
	if v==0:
		return 0
	a = abs(v)
	a += offset
	if v>0:
		return a
	else:
		return -a


def cnf_concatenator(cnf_out, dag_out, *arg):
	"""UNION_CONCATENATOR:
	descripition:
		Concatenate a series of cnfs/dags together in paralell, passing the reporting variables of each of the dags to the inputs of a final artificial node
	args:
		cnf_out : the name of the cnf file to output
		dag_out : the name of the dag file to output
		*args   : a list of cnf/dag input filenames to concatenate together
	"""
	assert len(arg)%2==0, "must pass pairs of inputs, as series of CNF & DAG file paths"
	graph = nx.DiGraph()
	cnf_lines = 0
	var_iterator = 0
	node_iterator = 1
	reporting = []
	with open(cnf_out,"w") as cnf_f:
		for i in tqdm(range(0,len(arg),2)):
			var_max = 0
			node_max = 0
			adder1 = convert_adder(var_iterator)
			adder2 = convert_adder(cnf_lines)
			with open(arg[i],"r") as f: #read CNF file
				for l in f.readlines():
					l = l.strip()
					if l[0] != 'p' and l[0] != 'c':
						l = [add_to_variable(int(ll),var_iterator) for ll in l.split(" ")]
						m = max([abs(ll) for ll in l])
						cnf_f.write("{}\n".format(" ".join([str(cc) for cc in l])))
						cnf_lines += 1
						if m>var_max:
							var_max = m
			with open(arg[i+1],"r") as f: #read corresponding DAG file
				for l in f.readlines():
					match = re.search("^(\d+)->(\d+):([\d,-]*)", l)
					if match: #if match for link between nodes
						g = list(match.groups())
						assert(len(g)==3)
						g[2] = re.sub("\d+",adder1,g[2]) #add offset to the variables
						graph.add_edge(int(g[0])+node_iterator,int(g[1])+node_iterator, string=g[2]) #add edge to graph corresponding to dag link
					match = re.search("^(\d+):([\d,-]*)",l)
					if match: #if match for node clause specification
						g = list(match.groups())
						assert(len(g)==2)
						g[1] = re.sub("\d+",adder2,g[1]) #add offset to cnf lines
						new_node = int(g[0])+node_iterator
						if new_node not in graph.nodes: #add the node to the graph if not already in it
							graph.add_node(new_node)
						graph.nodes[new_node]['string'] = g[1] #append the string of clause indices
						if new_node > node_max:
							node_max = new_node #add to max node if nessisary
					match = re.search("^([\d,-]+)$",l)
					if match: #if reporting section
						g = list(match.groups())
						g[0] = re.sub("(\d+)+",adder1,g[0]) #add offset to the variables and append to reporting array
						reporting.append(g[0])
			node_iterator = node_max + 1
			print(var_iterator,var_max)
			var_iterator = var_max
	prepend_line(cnf_out, "p cnf {} {}".format(var_iterator,cnf_lines))
	reporting = ",".join(reporting)
	
	with open(dag_out,"w") as f:
		nodes = len(graph.nodes)+1
		f.write("DAG-FILE\nNODES:{}\nGRAPH:\n".format(nodes))
		for edge in graph.edges:
			f.write("{}->{}:{}\n".format(edge[0],edge[1],graph.edges[edge[0],edge[1]]['string']))
		for i in graph.nodes.keys():
			if i not in [j for j,k in graph.edges]:
				f.write("{}->{}:{}\n".format(i,0,reporting))
		f.write("CLAUSES:\n")
		for i in graph.nodes:
			f.write("{}:{}\n".format(i,graph.nodes[i]['string']))
		f.write("REPORTING:\n")
		f.write("{}\n".format(reporting))


if __name__ == '__main__':
	fire.Fire(cnf_concatenator)
