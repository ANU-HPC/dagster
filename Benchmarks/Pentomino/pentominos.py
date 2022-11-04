#!/usr/bin/python3

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

print("----------------------------")
print(": PENTOMINO PUZZLE UTILITY :")
print("----------------------------")
print("Inspired by 'Airlocks' by Gliperal")
print("As solved by Mark Goodliffe - Cracking The Cryptic")
print("see: youtube.com/watch?v=S2aN-s3hG6Y")
print("")

import click
from processor import *
from generators import *
from tqdm import tqdm


@click.group()
def cli():
	pass

@click.command()
@click.argument('map_file')
@click.argument('cnf_name')
def generate(map_file,cnf_name):
	''' for a pentomino grid file, output a CNF&MAP '''
	lines,w,h = loadLines(map_file)
	pretty_print_puzzle(lines)
	cnf, CNF_variables, CNF_variable_indices = generate_CNF_and_MAP(lines)
	output_CNF(cnf, CNF_variables, "{}.cnf".format(cnf_name))
	output_MAP(CNF_variables, CNF_variable_indices, "{}.map".format(cnf_name))

@click.command()
@click.argument('map_file')
def display(map_file):
	''' for a pentomino grid file, display the grid '''
	lines,w,h = loadLines(map_file)
	pretty_print_puzzle(lines)
	
@click.command()
@click.argument('map_file')
def display_tikz(map_file):
	''' for a pentomino grid file, display as latex-tikz '''
	lines,w,h = loadLines(map_file)
	print("\\begin{tikzpicture}")
	for x in range(len(lines[0])):
		print("\\draw[gray,thin] ({},{})--({},{});".format(x,0,x,len(lines)))
	for y in range(len(lines)):
		print("\\draw[gray,thin] ({},{})--({},{});".format(0,y,len(lines[0]),y))
	for y,l in enumerate(lines):
		for x,v in enumerate(l):
			if v=="|" or v=="+":
				print("\\draw[ultra thick] ({},{})--({},{});".format(x+1,y,x+1,y+1))
			if v=="-" or v=="+":
				print("\\draw[ultra thick] ({},{})--({},{});".format(x,y+1,x+1,y+1))
	print("\\draw[ultra thick] ({},{})--({},{});".format(0,0,0,len(lines)))
	print("\\draw[ultra thick] ({},{})--({},{});".format(0,0,len(lines[0]),0))
	print("\\draw[ultra thick] ({},{})--({},{});".format(len(lines[0]),len(lines),0,len(lines)))
	print("\\draw[ultra thick] ({},{})--({},{});".format(len(lines[0]),len(lines),len(lines[0]),0))
	
	print("\\end{tikzpicture}")

@click.command()
@click.argument('cnf_name')
@click.argument('map_name')
def solve(cnf_name, map_name):
	''' solve pentomino problem for input CNF&MAP file '''
	cnf = import_CNF(cnf_name)
	CNF_variables,CNF_variable_indices = import_MAP(map_name)
	w = 0
	h = 0
	for k in CNF_variables.keys():
		k_split = k.split("_")
		if len(k_split)==3:
			x = int(k_split[0])
			y = int(k_split[1])
			if x>w:
				w=x
			if y>h:
				h=y
	solution, data = solve_minisat(cnf, CNF_variables,w,h)
	if data is None:
		print("NO SOLUTION")
	else:
		print("SOLUTION:")
		printSolution(solution)


@click.group()
def create():
	''' create a pentomino problem. '''
	pass

@click.command()
@click.argument('map_name')
@click.argument('output_map_name')
def forward(map_name, output_map_name):
	''' forward complete a puzzle for a unique solution. ---- for a given partially completed pentomino grid file, fill it out with walls to make it uniquely soluble, using the forward method (usually generated easier solutions)'''
	original_lines,w,h = loadLines(map_name)
	lines,t = generate_forward_problem(original_lines,w,h)
	if lines is None:
		pass
	else:
		printSolutionToFile(lines,output_map_name)
		print("done creating forward")

@click.command()
@click.argument('map_name')
@click.argument('output_map_name')
@click.argument('iteration_threshold', type=click.INT)
def backward(map_name, output_map_name, iteration_threshold):
	''' backward complete a puzzle for a unique solution. ---- for a given partially completed pentomino grid file, fill it out with walls to make it uniquely soluble, using the backward method (usually generates harder solutions) '''
	original_lines,w,h = loadLines(map_name)
	lines,t = generate_reverse_problem(original_lines,w,h,iteration_threshold)
	if lines is None:
		pass
	else:
		printSolutionToFile(lines,output_map_name)
		print("done creating backward")

@click.command()
@click.argument('sizex', type=click.INT)
@click.argument('sizey', type=click.INT)
@click.argument('buffer_units', type=click.INT)
@click.argument('repeat', type=click.INT)
@click.argument('problem_name')
def combination_1(sizex,sizey,buffer_units,repeat,problem_name):
	assert sizex*sizey%5==0, "sizex times sizey must be tilable by pentominos"
	# generate solution to an empty problem of sizex,sizey which can be appended end-on-end
	print("Generating a random unit solution")
	lines = [" "*sizex for i in range(sizey)]
	cnf, CNF_variables, CNF_variable_indices = generate_CNF_and_MAP(lines)
	unit_solution, data = solve_gnovelty(cnf, CNF_variables, sizex, sizey)
	while True in [l[0]==l[-1] for l in unit_solution]:
		cnf.append([str(-dd) for dd in data])
		unit_solution, data = solve_gnovelty(cnf, CNF_variables, sizex, sizey)
	# generate a problem which has unit_solutions multiple times in a row, 2*buffer_units+1
	print("Generating a random buffered solution")
	buffer_lines = [l*(2*buffer_units+1) for l in unit_solution]
	new_lines,t = generate_reverse_problem(buffer_lines,sizex*(2*buffer_units+1),sizey, 10)
	print("testing solutbility for small repeat factor")
	conjoined_new_lines = repeat_inner_section(new_lines,buffer_units,sizex,3)
	pretty_print_puzzle(conjoined_new_lines)
	no_solutions, sol1, sol2, t = is_multiply_soluble(conjoined_new_lines,len(conjoined_new_lines[0]),sizey)[0]
	print("Number of solutions {}".format(no_solutions))
	assert no_solutions == 1, "Woops, bad run, retry..."
	print("YAY, proceeding to generate large problem")
	conjoined_new_lines = repeat_inner_section(new_lines,buffer_units,sizex,repeat)
	cnf, CNF_variables, CNF_variable_indices = generate_CNF_and_MAP(conjoined_new_lines)
	output_CNF(cnf, CNF_variables, "{}.cnf".format(problem_name ))
	output_MAP(CNF_variables, CNF_variable_indices, "{}.map".format(problem_name ))
	printSolutionToFile(conjoined_new_lines,problem_name)



@click.command()
@click.argument('sizex', type=click.INT)
@click.argument('sizey', type=click.INT)
@click.argument('repeat', type=click.INT)
@click.argument('problem_name')
def combination_2(sizex,sizey,repeat,problem_name):
	sub_sizex = 5
	sub_sizey = 5
	assert sizex%sub_sizex==0, "sizex must be modulo 5 to be tilable by pentominos"
	assert sizey%sub_sizey==0, "sizey must be modulo 5 to be tilable by pentominos"
	# generate solution to an empty problem of sizex,sizey which can be appended end-on-end
	print("Generating a random unit solution")
	lines = [" "*sub_sizex for i in range(5)]
	cnf, CNF_variables, CNF_variable_indices = generate_CNF_and_MAP(lines)
	unit_solution, data = solve_gnovelty(cnf, CNF_variables, sub_sizex, sub_sizey)
	while (True in [l[0]==l[-1] for l in unit_solution]) or (True in [unit_solution[0][i]==unit_solution[-1][i] for i in range(len(unit_solution[0]))]):
		cnf.append([str(-dd) for dd in data])
		unit_solution, data = solve_gnovelty(cnf, CNF_variables, sub_sizex, sub_sizey)
	unit_solution = [l*int(sizex/sub_sizex) for l in unit_solution]
	unit_solution = unit_solution*int(sizey/sub_sizey)
	# generate a problem which has unit_solutions multiple times in a row, 2*buffer_units+1
	print("tiling unit solution repeat times")
	pretty_print_puzzle(unit_solution)
	print(unit_solution)
	new_lines,t = generate_reverse_problem(unit_solution,sizex,sizey, 10)
	for i in range(len(new_lines)):
		if new_lines[i][-1]==" ":
			new_lines[i] = change_char(new_lines[i],len(new_lines[i])-1,"|")
		if new_lines[i][-1]=="-":
			new_lines[i] = change_char(new_lines[i],len(new_lines[i])-1,"+")
	buffer_lines = [l*repeat for l in new_lines]
	pretty_print_puzzle(buffer_lines)
	cnf, CNF_variables, CNF_variable_indices = generate_CNF_and_MAP(buffer_lines)
	output_CNF(cnf, CNF_variables, "{}.cnf".format(problem_name ))
	output_MAP(CNF_variables, CNF_variable_indices, "{}.map".format(problem_name ))
	printSolutionToFile(buffer_lines,problem_name)






@click.command()
@click.argument('sizex', type=click.INT)
@click.argument('sizey', type=click.INT)
@click.argument('repeatx', type=click.INT)
@click.argument('repeaty', type=click.INT)
@click.argument('problem_name')
def combination_3(sizex,sizey,repeatx,repeaty,problem_name):
	sub_sizex = 5
	sub_sizey = 5
	assert sizex%sub_sizex==0, "sizex must be modulo 5 to be tilable by pentominos"
	assert sizey%sub_sizey==0, "sizey must be modulo 5 to be tilable by pentominos"
	# generate solution to an empty problem of sizex,sizey which can be appended end-on-end
	print("Generating a random unit solution")
	lines = [" "*sub_sizex for i in range(5)]
	cnf, CNF_variables, CNF_variable_indices = generate_CNF_and_MAP(lines)
	unit_solution, data = solve_gnovelty(cnf, CNF_variables, sub_sizex, sub_sizey)
	while (True in [l[0]==l[-1] for l in unit_solution]) or (True in [unit_solution[0][i]==unit_solution[-1][i] for i in range(len(unit_solution[0]))]):
		cnf.append([str(-dd) for dd in data])
		unit_solution, data = solve_gnovelty(cnf, CNF_variables, sub_sizex, sub_sizey)
	unit_solution = [l*int(sizex/sub_sizex) for l in unit_solution]
	unit_solution = unit_solution*int(sizey/sub_sizey)
	# generate a problem which has unit_solutions multiple times in a row, 2*buffer_units+1
	print("tiling unit solution repeat times")
	pretty_print_puzzle(unit_solution)
	print(unit_solution)
	new_lines,t = generate_reverse_problem(unit_solution,sizex,sizey, 10)
	for i in range(len(new_lines)):
		if new_lines[i][-1]==" ":
			new_lines[i] = change_char(new_lines[i],len(new_lines[i])-1,"|")
		if new_lines[i][-1]=="-":
			new_lines[i] = change_char(new_lines[i],len(new_lines[i])-1,"+")
	for i in range(len(new_lines[-1])):
		if new_lines[-1][i]==" ":
			new_lines[-1] = change_char(new_lines[-1],i,"-")
		if new_lines[-1][i]=="|":
			new_lines[-1] = change_char(new_lines[-1],i,"+")
	buffer_lines = [l*repeatx for l in new_lines]
	buffer_lines = buffer_lines*repeaty
	pretty_print_puzzle(buffer_lines)
	cnf, CNF_variables, CNF_variable_indices = generate_CNF_and_MAP(buffer_lines)
	output_CNF(cnf, CNF_variables, "{}.cnf".format(problem_name ))
	output_MAP(CNF_variables, CNF_variable_indices, "{}.map".format(problem_name ))
	printSolutionToFile(buffer_lines,problem_name)





@click.group()
def dag_make():
	''' construct a dag for exsiting pentomino problem '''
	pass


# basic binary search lookup
# assumes list 'elements' is sorted and comparable
def find_index(elements, value):
	left, right = 0, len(elements) - 1
	while left <= right:
		middle = (left + right) // 2
		middle_element = elements[middle]
		if middle_element == value:
			return middle
		if middle_element < value:
			left = middle + 1
		else:
			right = middle - 1
	return None

import collections
def nested_dict():
	return collections.defaultdict(nested_dict)
def compiled_CNF_variables(CNF_variables):
	d = nested_dict()
	for k,v in CNF_variables.items():
		if '@' in k:
			ksplit = k.split("@")
			assert len(ksplit)==2
			xy = ksplit[1].split("_")
			xy = (int(xy[0]),int(xy[1]))
			d[ksplit[0]][xy[0]][xy[1]] = v
		else:
			ksplit = k.split("_")
			assert len(ksplit)==3
			xy = (int(ksplit[0]),int(ksplit[1]))
			d[ksplit[2]][xy[0]][xy[1]] = v
	return d
			



def get_relevent_cnf_indices(cnf, compiled_variables,start,stop):
	relevent_variables = []
	for k in compiled_variables.keys():
		for x_tile in compiled_variables[k].keys():
			if (x_tile >= start) and (x_tile < stop):
				relevent_variables = relevent_variables + list(compiled_variables[k][x_tile].values())
	relevent_variables = sorted(relevent_variables)
	min_relevent_variable = min(relevent_variables)
	max_relevent_variable = max(relevent_variables)
	cnf_indices = []
	for i in range(len(cnf)):
		for j in cnf[i]:
			k = abs(j)
			if k>=min_relevent_variable and k<=max_relevent_variable:
				if find_index(relevent_variables,k) is not None:
					break
		else:
			continue
		cnf_indices.append(i)
	return cnf_indices


def get_relevent_cnf_indices_from_indices(CNF_variable_indices, compiled_variables,start,stop):
	relevent_variables = []
	for k in compiled_variables.keys():
		for x_tile in compiled_variables[k].keys():
			if (x_tile >= start) and (x_tile < stop):
				relevent_variables = relevent_variables + list(compiled_variables[k][x_tile].values())
	relevent_variables = sorted(relevent_variables)
	cnf_indices = set()
	for r in relevent_variables:
		cnf_indices.update(set(CNF_variable_indices[r]))
	return list(cnf_indices)

def get_relevent_cnf_indices_from_indices_2D(CNF_variable_indices, compiled_variables,xstart,xstop,ystart,ystop):
	relevent_variables = []
	for k in compiled_variables.keys():
		for x_tile in compiled_variables[k].keys():
			if (x_tile >= xstart) and (x_tile < xstop):
				for y_tile in compiled_variables[k][x_tile].keys():
					if (y_tile >= ystart) and (y_tile < ystop):
						relevent_variables.append(compiled_variables[k][x_tile][y_tile])
	relevent_variables = sorted(relevent_variables)
	cnf_indices = set()
	for r in relevent_variables:
		cnf_indices.update(set(CNF_variable_indices[r]))
	return list(cnf_indices)

def get_relevent_forward_connection_literals(compiled_variables, to_x):
	relevent_variables = []
	for k in list(pentomino_keys) + ['x_link','y_link']:
		for x_tile in compiled_variables[k].keys():
			if (x_tile < to_x):
				relevent_variables = relevent_variables + list(compiled_variables[k][x_tile].values())
	return relevent_variables

def get_relevent_forward_connection_literals_2D(compiled_variables, to_x, to_y):
	relevent_variables = []
	for k in list(pentomino_keys) + ['x_link','y_link']:
		for x_tile in compiled_variables[k].keys():
			if (x_tile < to_x):
				for y_tile in compiled_variables[k][x_tile].keys():
					if (y_tile < to_y):
						relevent_variables.append(compiled_variables[k][x_tile][y_tile])
	return relevent_variables


@click.command()
@click.argument('cnf_file')
@click.argument('map_file')
@click.argument('dag_file')
@click.argument('horisontal_lines', type=click.INT)
def simple_cubes(cnf_file,map_file,dag_file,horisontal_lines):
	cnf = import_CNF(cnf_file)
	CNF_variables, CNF_variable_indices = import_MAP(map_file)
	max_variables = max(CNF_variables.values())
	compiled_variables = compiled_CNF_variables(CNF_variables)
	f = open(dag_file,"w")
	f.write("DAG-FILE\nNODES:{}\nGRAPH:\n".format(2))
	f.write("0->1:{}\n".format(contract_sequence(get_relevent_forward_connection_literals(compiled_variables, horisontal_lines))   ))
	f.write("CLAUSES:\n")
	f.write("0:{}\n".format(contract_sequence(get_relevent_cnf_indices_from_indices(CNF_variable_indices, compiled_variables,0,horisontal_lines))))
	f.write("1:0-{}\n".format(len(cnf)-1))
	f.write("REPORTING:\n")
	f.write("{}-{}".format(1,max_variables))
	f.write("\n")
	f.close()


@click.command()
@click.argument('cnf_file')
@click.argument('map_file')
@click.argument('dag_file')
@click.argument('horisontal_line_iterator', type=click.INT)
def complex_cubes(cnf_file,map_file,dag_file,horisontal_line_iterator):
	cnf = import_CNF(cnf_file)
	CNF_variables, CNF_variable_indices = import_MAP(map_file)
	max_variables = max(CNF_variables.values())
	CNF_variable_keys = CNF_variables.keys()
	compiled_variables = compiled_CNF_variables(CNF_variables)
	blocks = 0
	while ("{}_0_F".format(blocks*horisontal_line_iterator) in CNF_variable_keys):
		blocks += 1
	f = open(dag_file,"w")
	f.write("DAG-FILE\nNODES:{}\nGRAPH:\n".format(blocks))
	print("writing graph links")
	for i in tqdm(range(blocks-1)):
		f.write("{}->{}:{}\n".format(i,i+1,contract_sequence(get_relevent_forward_connection_literals(compiled_variables, (i+1)*horisontal_line_iterator))   ))
	f.write("CLAUSES:\n")
	print("writing clause indices")
	for i in tqdm(range(blocks)):
		f.write("{}:{}\n".format(i,contract_sequence(get_relevent_cnf_indices_from_indices(CNF_variable_indices, compiled_variables,i*horisontal_line_iterator,(i+1)*horisontal_line_iterator))))
	f.write("REPORTING:\n")
	f.write("{}-{}".format(1,max_variables))
	f.write("\n")
	f.close()



@click.command()
@click.argument('cnf_file')
@click.argument('map_file')
@click.argument('dag_file')
@click.argument('horisontal_line_iterator', type=click.INT)
def complex_cubes2(cnf_file,map_file,dag_file,horisontal_line_iterator):
	cnf = import_CNF(cnf_file)
	CNF_variables, CNF_variable_indices = import_MAP(map_file)
	max_variables = max(CNF_variables.values())
	CNF_variable_keys = CNF_variables.keys()
	compiled_variables = compiled_CNF_variables(CNF_variables)
	blocks = 0
	while ("{}_0_F".format(blocks*horisontal_line_iterator) in CNF_variable_keys):
		blocks += 1
	f = open(dag_file,"w")
	f.write("DAG-FILE\nNODES:{}\nGRAPH:\n".format(blocks))
	print("writing graph links")
	for i in tqdm(range(blocks-1)):
		f.write("{}->{}:{}\n".format(i,i+1,contract_sequence(get_relevent_forward_connection_literals(compiled_variables, (i+1)*horisontal_line_iterator))   ))
	f.write("CLAUSES:\n")
	print("writing clause indices")
	for i in tqdm(range(blocks)):
		f.write("{}:{}\n".format(i,contract_sequence(get_relevent_cnf_indices_from_indices(CNF_variable_indices, compiled_variables,0,(i+1)*horisontal_line_iterator))))
	f.write("REPORTING:\n")
	f.write("{}-{}".format(1,max_variables))
	f.write("\n")
	f.close()





@click.command()
@click.argument('cnf_file')
@click.argument('map_file')
@click.argument('dag_file')
@click.argument('horisontal_line_iterator', type=click.INT)
def complex_cubes3(cnf_file,map_file,dag_file,horisontal_line_iterator):
	cnf = import_CNF(cnf_file)
	CNF_variables, CNF_variable_indices = import_MAP(map_file)
	max_variables = max(CNF_variables.values())
	CNF_variable_keys = CNF_variables.keys()
	compiled_variables = compiled_CNF_variables(CNF_variables)
	xblocks = 0
	while ("{}_0_F".format(xblocks*horisontal_line_iterator) in CNF_variable_keys):
		xblocks += 1
	yblocks = 0
	while ("0_{}_F".format(yblocks*horisontal_line_iterator) in CNF_variable_keys):
		yblocks += 1
	node_index = lambda x,y:y*xblocks+x
	f = open(dag_file,"w")
	f.write("DAG-FILE\nNODES:{}\nGRAPH:\n".format(xblocks*yblocks))
	print("writing graph links")
	for i in tqdm(range(xblocks-1)):
		for j in range(yblocks):
			f.write("{}->{}:{}\n".format(node_index(i,j),node_index(i+1,j),contract_sequence(get_relevent_forward_connection_literals_2D(compiled_variables, (i+1)*horisontal_line_iterator, (j+1)*horisontal_line_iterator))   ))
	for i in tqdm(range(xblocks)):
		for j in range(yblocks-1):
			f.write("{}->{}:{}\n".format(node_index(i,j),node_index(i,j+1),contract_sequence(get_relevent_forward_connection_literals_2D(compiled_variables, (i+1)*horisontal_line_iterator, (j+1)*horisontal_line_iterator))   ))
	f.write("CLAUSES:\n")
	print("writing clause indices")
	for i in tqdm(range(xblocks)):
		for j in range(yblocks):
			f.write("{}:{}\n".format(node_index(i,j),contract_sequence(
				get_relevent_cnf_indices_from_indices_2D(CNF_variable_indices, compiled_variables,0,(i+1)*horisontal_line_iterator,0,(j+1)*horisontal_line_iterator)
				)))
	f.write("REPORTING:\n")
	f.write("{}-{}".format(1,max_variables))
	f.write("\n")
	f.close()




def get_relevent_forward_connection_literals_block(compiled_variables, from_x, to_x, from_y, to_y):
	relevent_variables = []
	for k in list(pentomino_keys): #+ ['x_link','y_link']:
		for x_tile in compiled_variables[k].keys():
			if (x_tile < to_x) and (x_tile >= from_x):
				for y_tile in compiled_variables[k][x_tile].keys():
					if (y_tile < to_y) and (y_tile >= from_y):
						relevent_variables.append(compiled_variables[k][x_tile][y_tile])
	return relevent_variables


@click.command()
@click.argument('cnf_file')
@click.argument('map_file')
@click.argument('dag_file')
@click.argument('horisontal_line_iterator', type=click.INT)
def complex_cubes4(cnf_file,map_file,dag_file,horisontal_line_iterator):
	cnf = import_CNF(cnf_file)
	CNF_variables, CNF_variable_indices = import_MAP(map_file)
	max_variables = max(CNF_variables.values())
	CNF_variable_keys = CNF_variables.keys()
	compiled_variables = compiled_CNF_variables(CNF_variables)
	xblocks = 0
	while ("{}_0_F".format(xblocks*horisontal_line_iterator) in CNF_variable_keys):
		xblocks += 1
	yblocks = 0
	while ("0_{}_F".format(yblocks*horisontal_line_iterator) in CNF_variable_keys):
		yblocks += 1
	node_index = lambda x,y:y*xblocks+x
	f = open(dag_file,"w")
	f.write("DAG-FILE\nNODES:{}\nGRAPH:\n".format(xblocks*yblocks+1))
	print("writing graph links")
	for i in tqdm(range(1,xblocks)):
		for j in range(yblocks-1):
			upper_sequence = get_relevent_forward_connection_literals_block(compiled_variables, 0, xblocks*horisontal_line_iterator, 0, (j)*horisontal_line_iterator)
			f.write("{}->{}:{}\n".format(node_index(i,j),node_index(i-1,j+1),contract_sequence(get_relevent_forward_connection_literals_block(compiled_variables, i*horisontal_line_iterator, (i+1)*horisontal_line_iterator, j*horisontal_line_iterator, (j+1)*horisontal_line_iterator)+upper_sequence)   ))
	for i in tqdm(range(xblocks)):
		for j in range(yblocks-1):
			upper_sequence = get_relevent_forward_connection_literals_block(compiled_variables, 0, xblocks*horisontal_line_iterator, 0, (j)*horisontal_line_iterator)
			f.write("{}->{}:{}\n".format(node_index(i,j),node_index(i,j+1),contract_sequence(get_relevent_forward_connection_literals_block(compiled_variables, i*horisontal_line_iterator, (i+1)*horisontal_line_iterator, j*horisontal_line_iterator, (j+1)*horisontal_line_iterator)+upper_sequence)   ))
	for i in tqdm(range(xblocks-1)):
		for j in range(yblocks-1):
			upper_sequence = get_relevent_forward_connection_literals_block(compiled_variables, 0, xblocks*horisontal_line_iterator, 0, (j)*horisontal_line_iterator)
			f.write("{}->{}:{}\n".format(node_index(i,j),node_index(i+1,j+1),contract_sequence(get_relevent_forward_connection_literals_block(compiled_variables, i*horisontal_line_iterator, (i+1)*horisontal_line_iterator, j*horisontal_line_iterator, (j+1)*horisontal_line_iterator)+upper_sequence)   ))
	for i in tqdm(range(xblocks)):
		j = yblocks-1
		#j = 1
		upper_sequence = get_relevent_forward_connection_literals_block(compiled_variables, 0, xblocks*horisontal_line_iterator, 0, (j)*horisontal_line_iterator)
		f.write("{}->{}:{}\n".format(node_index(i,j),node_index(0,yblocks),contract_sequence(get_relevent_forward_connection_literals_block(compiled_variables, i*horisontal_line_iterator, (i+1)*horisontal_line_iterator, j*horisontal_line_iterator, (j+1)*horisontal_line_iterator)+upper_sequence)   ))
	f.write("CLAUSES:\n")
	print("writing clause indices")
	for i in tqdm(range(xblocks)):
		for j in range(yblocks):
			f.write("{}:{}\n".format(node_index(i,j),contract_sequence(
				get_relevent_cnf_indices_from_indices_2D(CNF_variable_indices, compiled_variables,max(0,i*horisontal_line_iterator-1),(i+1)*horisontal_line_iterator,max(0,j*horisontal_line_iterator-1),(j+1)*horisontal_line_iterator)
				)))
	f.write("{}:{}\n".format(node_index(0,yblocks),contract_sequence([1])))
	f.write("REPORTING:\n")
	f.write("{}-{}".format(1,max_variables))
	f.write("\n")
	f.close()





@click.command()
@click.argument('cnf_file')
@click.argument('map_file')
@click.argument('dag_file')
@click.argument('horisontal_line_iterator', type=click.INT)
def complex_cubes5(cnf_file,map_file,dag_file,horisontal_line_iterator):
	cnf = import_CNF(cnf_file)
	CNF_variables, CNF_variable_indices = import_MAP(map_file)
	max_variables = max(CNF_variables.values())
	CNF_variable_keys = CNF_variables.keys()
	compiled_variables = compiled_CNF_variables(CNF_variables)
	xblocks = 0
	while ("{}_0_F".format(xblocks*horisontal_line_iterator) in CNF_variable_keys):
		xblocks += 1
	yblocks = 0
	while ("0_{}_F".format(yblocks*horisontal_line_iterator) in CNF_variable_keys):
		yblocks += 1
	node_index = lambda x,y:y*xblocks+x
	f = open(dag_file,"w")
	f.write("DAG-FILE\nNODES:{}\nGRAPH:\n".format(xblocks*yblocks+1))
	print("writing graph links")
	for i in tqdm(range(xblocks)):
		for j in range(yblocks-1):
			f.write("{}->{}:{}\n".format(node_index(i,j),node_index(i,j+1),contract_sequence(get_relevent_forward_connection_literals_block(compiled_variables, i*horisontal_line_iterator, (i+1)*horisontal_line_iterator, 0, (j+1)*horisontal_line_iterator))   ))
	for i in tqdm(range(xblocks)):
		j = yblocks-1
		#j = 1
		f.write("{}->{}:{}\n".format(node_index(i,j),node_index(0,yblocks),contract_sequence(get_relevent_forward_connection_literals_block(compiled_variables, i*horisontal_line_iterator, (i+1)*horisontal_line_iterator, 0, (j+1)*horisontal_line_iterator))   ))
	f.write("CLAUSES:\n")
	print("writing clause indices")
	for i in tqdm(range(xblocks)):
		for j in range(yblocks):
			f.write("{}:{}\n".format(node_index(i,j),contract_sequence(
				get_relevent_cnf_indices_from_indices_2D(CNF_variable_indices, compiled_variables,max(0,i*horisontal_line_iterator-1),(i+1)*horisontal_line_iterator,max(0,j*horisontal_line_iterator-1),(j+1)*horisontal_line_iterator)
				)))
	f.write("{}:{}\n".format(node_index(0,yblocks),contract_sequence([1])))
	f.write("REPORTING:\n")
	f.write("{}-{}".format(1,max_variables))
	f.write("\n")
	f.close()




@click.command()
@click.argument('map_file')
def check_multiply_soluble(map_file):
	''' for a pentomino grid file, is it multiply soluble '''
	lines,w,h = loadLines(map_file)
	no_solutions, sol1, sol2, t = is_multiply_soluble(lines,w,h)
	print(no_solutions)




@click.command()
@click.argument('map_file')
@click.argument('solution_file')
@click.argument('w', type=click.INT)
@click.argument('h', type=click.INT)
def view_solution(map_file,solution_file,w,h):
	''' print solution, input MAP, solution file,w,h'''
	CNF_variables, CNF_variable_indices = import_MAP(map_file)
	with open(solution_file,"r") as f:
		solutions = [[int(fff) for fff in ff.strip().split(' ')] for ff in f.readlines()]
	for solution in solutions:
		processed_solution = processSolution(solution, CNF_variables, w, h)
		printSolution(processed_solution)
		print("")
	



dag_make.add_command(simple_cubes)
dag_make.add_command(complex_cubes)
dag_make.add_command(complex_cubes2)
dag_make.add_command(complex_cubes3)
dag_make.add_command(complex_cubes4)
dag_make.add_command(complex_cubes5)
create.add_command(forward)
create.add_command(backward)
create.add_command(combination_1)
create.add_command(combination_2)
create.add_command(combination_3)
cli.add_command(generate)
cli.add_command(solve)
cli.add_command(display)
cli.add_command(display_tikz)
cli.add_command(create)
cli.add_command(dag_make)
cli.add_command(check_multiply_soluble)
cli.add_command(view_solution)

if __name__ == '__main__':
	cli()






