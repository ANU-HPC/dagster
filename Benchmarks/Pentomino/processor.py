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

import os
import time
import random
import re
from tqdm import tqdm
import json

# lay out all pentomino shapes
pentominos = {
	'F':[[0,1],[0,2],[1,0],[1,1],[2,1]],
	'I':[[0,0],[0,1],[0,2],[0,3],[0,4]],
	'L':[[0,0],[0,1],[0,2],[0,3],[1,3]],
	'N':[[0,0],[0,1],[0,2],[1,2],[1,3]],
	'P':[[0,0],[0,1],[1,0],[1,1],[1,2]],
	'T':[[1,0],[1,1],[1,2],[0,2],[2,2]],
	'U':[[0,0],[0,2],[1,0],[1,2],[1,1]],
	'V':[[0,0],[0,1],[0,2],[1,2],[2,2]],
	'W':[[0,0],[0,1],[1,1],[1,2],[2,2]],
	'X':[[1,0],[0,1],[1,1],[1,2],[2,1]],
	'Y':[[0,0],[0,1],[0,2],[1,2],[0,3]],
	'Z':[[0,0],[0,1],[1,1],[2,1],[2,2]]
}

# sort the indices
pentomino_keys = "".join(list(sorted(pentominos.keys())))
for k in pentomino_keys:
	pentominos[k] = sorted(pentominos[k])

# function to reflect pentomino coordinates
def reflect(coordinates, dimension):
	c = []
	mind = 0
	for cc in coordinates:
		ccc = cc[:]
		if -ccc[dimension] < mind:
			mind = -ccc[dimension]
		ccc[dimension] = -ccc[dimension]
		c.append(ccc)
	for i in range(len(c)):
		c[i][dimension] -= mind
	return sorted(c)

#function to transpose pentomino coordinates
def transpose(c):
	return sorted([[y,x] for x,y in c])

# function to return all unique dihedral mappings of pentomino coordinates
def generate_dihedral(c):
	A = 	[c, 
			reflect(c,0),
			reflect(reflect(c,0),1),
			reflect(reflect(reflect(c,0),1),0),
			transpose(c), 
			reflect(transpose(c),0),
			reflect(reflect(transpose(c),0),1),
			reflect(reflect(reflect(transpose(c),0),1),0)]
	B = []
	B_str = []
	for a in A:
		a_str = str(a)
		if a_str not in B_str:
			B.append(a)
			B_str.append(a_str)
	return B

# register and generate all dihedral mappings of pentominos
for k in pentomino_keys:
	pentominos[k] = generate_dihedral(pentominos[k])

# given lines of a puzzle, pretty print them.
def pretty_print_puzzle(lines):
	h = len(lines)
	assert h>0
	w = len(lines[0])
	assert False not in [len(l)==w for l in lines]

	possible_symbols = "-+|# "+pentomino_keys
	for y in range(h):
		for x in range(w):
			if (lines[y][x] not in possible_symbols):
				raise Exception("unrecognised symbol in file")
	for y in range(h):
		for x in range(w):
			if (lines[y][x]=='#'):
				print("##",end="")
			elif (lines[y][x]=='|'):
				print(" |",end="")
			elif (lines[y][x]=='+'):
				print("_|",end="")
			elif (lines[y][x]=='-'):
				print("__",end="")
			elif (lines[y][x] in pentomino_keys):
				print("{0}{0}".format(lines[y][x]),end="")
			else:
				print("  ",end="")
		print("")



def loadLines(filename):
	# input the file, and do basic checking
	with open(filename, "r") as f:
		lines = [l.rstrip('\n') for l in f.readlines()]
	h = len(lines)
	assert h>0
	w = len(lines[0])
	assert False not in [len(l)==w for l in lines]
	return lines,w,h


def generate_CNF_and_MAP(lines):
	# basic checks
	h = len(lines)
	assert h>0
	w = len(lines[0])
	assert False not in [len(l)==w for l in lines]

	# beginning to setup CNF
	from collections import defaultdict
	variable_index = 0
	def a():
		nonlocal variable_index
		variable_index += 1
		return variable_index
	CNF_variables = defaultdict(a)
	CNF_variable_indices = defaultdict(list)

	# declare variables for grid and links
	for k in pentomino_keys:
		for x in range(w):
			for y in range(h):
				CNF_variables["{}_{}_{}".format(x,y,k)]
	for x in range(w):
		for y in range(h-1):
			CNF_variables["{}_{}_ylink".format(x,y)]
	for x in range(w-1):
		for y in range(h):
			CNF_variables["{}_{}_xlink".format(x,y)]

	cnf = []
	def add_clause(clause):
		nonlocal cnf
		index = len(cnf)
		cnf.append(clause)
		for c in clause:
			CNF_variable_indices[abs(c)].append(index)
	
	# constraints according to the file
	possible_symbols = "-+|# " + pentomino_keys
	for y in range(h):
		for x in range(w):
			if (lines[y][x] not in possible_symbols):
				raise Exception("unrecognised symbol in file")
			if (lines[y][x]=="#"):
				for k in pentomino_keys:
					add_clause([-CNF_variables["{}_{}_{}".format(x,y,k)]])
			elif (lines[y][x] in pentomino_keys):
				add_clause([CNF_variables["{}_{}_{}".format(x,y,lines[y][x])]])
			else:
				if (y<h-1) and ((lines[y][x]=='-') or (lines[y][x]=='+')):
					add_clause([-CNF_variables["{}_{}_ylink".format(x,y)]])
				if (x<w-1) and ((lines[y][x]=='|') or (lines[y][x]=='+')):
					add_clause([-CNF_variables["{}_{}_xlink".format(x,y)]])
				add_clause([CNF_variables["{}_{}_{}".format(x,y,k)] for k in pentomino_keys])
				for i in range(len(pentomino_keys)):
					for j in range(i+1,len(pentomino_keys)):
						add_clause([-CNF_variables["{}_{}_{}".format(x,y,pentomino_keys[i])],
									-CNF_variables["{}_{}_{}".format(x,y,pentomino_keys[j])]])

	# no x or y links between forbidden '#' blocks 
	for y in range(h-1):
		for x in range(w):
			if lines[y][x]=='#' and lines[y+1][x]=='#':
				add_clause([-CNF_variables["{}_{}_ylink".format(x,y)]])
	for y in range(h):
		for x in range(w-1):
			if lines[y][x]=='#' and lines[y][x+1]=='#':
				add_clause([-CNF_variables["{}_{}_xlink".format(x,y)]])

	#produce mapping of squares to pentominos
	block_dict = {}
	for k in pentomino_keys:
		for i in range(len(pentominos[k])): #for each pentomino dihedral mapping
			for x in range(w):
				for y in range(h):
					coordinates = [[xx+x,yy+y] for xx,yy in pentominos[k][i]] #for every possible translation of its coordinates
					coordinate_out_of_bounds = [xx>=w or yy>=h for xx,yy in coordinates]
					if True not in coordinate_out_of_bounds: #if the translation is entirely in-bounds
						block = CNF_variables["{}{}@{}_{}".format(k,i,x,y)] #declare the existance of the translated pentomino
						for xx,yy in coordinates: #for all coordinates of that pentomino
							cnf_var = CNF_variables["{}_{}_{}".format(xx,yy,k)]
							add_clause([-block,cnf_var]) # if translated pentomino at thoes coordinates, then pentomino-shape at coordinate
							block_dict[cnf_var] = block_dict.get(cnf_var,[]) + [block]
							if xx+1<w:
								if [xx+1,yy] in coordinates: # x-link in the pentomino shape then pentomino implies xlink
									add_clause([-block,CNF_variables["{}_{}_xlink".format(xx,yy)]])
								else:
									add_clause([-block,-CNF_variables["{}_{}_xlink".format(xx,yy)]])
							if yy+1<h:
								if [xx,yy+1] in coordinates: # y-link in the pentomino shape then pentomino implies ylink
									add_clause([-block,CNF_variables["{}_{}_ylink".format(xx,yy)]])
								else:
									add_clause([-block,-CNF_variables["{}_{}_ylink".format(xx,yy)]])
							if xx-1>=0:
								if [xx-1,yy] not in coordinates: # if superior x-link not in the pentomino shape then pentomino implies not xlink
									add_clause([-block,-CNF_variables["{}_{}_xlink".format(xx-1,yy)]])
							if yy-1>=0:
								if [xx,yy-1] not in coordinates: # if superior y-link not in the pentomino shape then pentomino implies not ylink
									add_clause([-block,-CNF_variables["{}_{}_ylink".format(xx,yy-1)]])
	# if a pentomino shape does not cover an area then that area is not covered by that pentomino shape
	for x in range(w):
		for y in range(h):
			for k in pentomino_keys:
				cnf_var = CNF_variables["{}_{}_{}".format(x,y,k)]
				if cnf_var not in block_dict.keys():
					add_clause([-cnf_var])
	# pentomino-shape at coordinate implies a pentomino of that shape covering that coordinate
	for k in block_dict.keys():
		add_clause([-k]+block_dict[k])

	# no adjacent squares if not linked
	for x in range(w-1):
		for y in range(h):
			for k in pentomino_keys:
				add_clause([-CNF_variables["{}_{}_{}".format(x,y,k)], 
							-CNF_variables["{}_{}_{}".format(x+1,y,k)], 
							CNF_variables["{}_{}_xlink".format(x,y)] ])
	for x in range(w):
		for y in range(h-1):
			for k in pentomino_keys:
				add_clause([-CNF_variables["{}_{}_{}".format(x,y,k)], 
							-CNF_variables["{}_{}_{}".format(x,y+1,k)],
							CNF_variables["{}_{}_ylink".format(x,y)] ])
	
	return cnf, CNF_variables, CNF_variable_indices


def output_CNF(cnf, CNF_variables, filename):
	with open(filename, "w") as f:
		f.write("p cnf {} {}\n".format(len(CNF_variables.keys()), len(cnf)))
		for c in cnf:
			f.write("{} 0\n".format(" ".join([str(a) for a in c])))

def import_CNF(filename):
	with open(filename,"r") as f:
		cnf = [a.strip() for a in f.readlines() if ('p' not in a) and ('c' not in a)]
	print("importing CNF")
	for i in tqdm(range(len(cnf))):
		cnf[i] = [int(k) for k in re.findall("-?[1-9][\d]+", cnf[i])]  #extract all whole integers, excluding zero
	return cnf

def output_MAP(CNF_variables, CNF_variable_indices, filename):
	with open(filename, "w") as f:
		for c,v in CNF_variables.items():
			f.write("{}\t{}\t{}\n".format(c,v,CNF_variable_indices[v]))

def import_MAP(filename):
	CNF_variables = {}
	CNF_variable_indices = {}
	with open(filename,"r") as f:
		print("importing MAP")
		for a in tqdm(f.readlines()):
			s = a.strip().split("\t")
			CNF_variables[s[0]] = int(s[1])
			CNF_variable_indices[int(s[1])] = json.loads(s[2])
	return CNF_variables, CNF_variable_indices


def loadSolution(file_name):
	with open(file_name,"r") as f:
		solution_readlines = f.readlines()
		if len(solution_readlines)>0 and solution_readlines[0].strip()=="SAT":
			data = [int(a) for a in solution_readlines[1].strip().split(" ") if a!="0"]
		else:
			data = None
	return data


def processSolution(data, CNF_variables, w, h):
	if data is None:
		return None
	data_pos = [d for d in data if d>0]
	solution_keys = []
	for k in CNF_variables.keys():
		if CNF_variables[k] in data_pos:
			solution_keys.append(k)
	solution = []
	for y in range(h):
		solution.append("")
		for x in range(w):
			for k in pentomino_keys:
				if "{}_{}_{}".format(x,y,k) in solution_keys:
					solution[-1] += k
					break
			else:
				solution[-1] += " "
	return solution

def printSolution(solution):
	print("")
	for s in solution:
		print(s)
	print("")

def printSolutionToFile(solution, filename):
	with open(filename,"w") as f:
		for s in solution:
			f.write("{}\n".format(s))

def solve_minisat(cnf, CNF_variables, w, h):
	r = int(random.random()*100000)
	temp_cnf = "TEMPORARY_FILE{}.cnf".format(r)
	temp_sols = "TEMPORARY_FILE{}.sols".format(r)
	output_CNF(cnf, CNF_variables, temp_cnf)
	os.system("{} {} {} -verb=0 -rnd-seed={}".format(os.environ.get("MINISAT_LOCATION","minisat"), temp_cnf, temp_sols, time.time()))
	data = loadSolution(temp_sols)
	os.system("rm {}".format(temp_cnf))
	os.system("rm {}".format(temp_sols))
	return processSolution(data, CNF_variables,w,h), data


def solve_gnovelty(cnf, CNF_variables, w, h):
	r = int(random.random()*100000)
	temp_cnf = "TEMPORARY_FILE{}.cnf".format(r)
	temp_sols = "TEMPORARY_FILE{}.sols".format(r)
	output_CNF(cnf, CNF_variables, temp_cnf)
	os.system("../../dagster/gnovelty/simple_standalone/gnovelty {} {}".format(temp_cnf, temp_sols, time.time()))
	with open(temp_sols,"r") as f:
		solution_readlines = f.readlines()
		if len(solution_readlines)>0:
			data = [int(a) for a in solution_readlines[0].strip().split(" ") if a!="0"]
		else:
			data = None
	os.system("rm {}".format(temp_cnf))
	os.system("rm {}".format(temp_sols))
	return processSolution(data, CNF_variables,w,h), data



def is_multiply_soluble(lines,w,h):
	cnf, CNF_variables,CNF_variable_indices  = generate_CNF_and_MAP(lines)
	t = time.time()
	solution, data = solve_minisat(cnf, CNF_variables,w,h)
	t = time.time()-t
	if data is None:
		return 0, None, None, t
	cnf.append([str(-dd) for dd in data])
	solution2, data2 = solve_minisat(cnf, CNF_variables,w,h)
	if data2 is None:
		return 1, solution, None, t
	return 2, solution, solution2, t

def change_char(text,pos,c):
	return text[:pos] + c + text[pos+1:]


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

def repeat_inner_section(a,buffer_units,sizex,repeat):
	r = []
	for aa in a:
		left = aa[0:buffer_units*sizex]
		middle = aa[buffer_units*sizex:(buffer_units+1)*sizex]
		end = aa[(buffer_units+1)*sizex:]
		r.append(left+(middle*repeat)+end)
	return r



