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
from colored import fg, bg, attr
import json

with open("header.json","r") as f:
	header_data = json.load(f)
for k in header_data.keys():
	globals()[k] = header_data[k]

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
#	nonlocal cnf
	cnf.append(clause)
pos_cardinality_cnf = []
def add_pos_cardinality_clause(clause, value): # add clause a,b,c >= value
#	nonlocal pos_cardinality_cnf
	pos_cardinality_cnf.append(clause+[value])
neg_cardinality_cnf = []
def add_neg_cardinality_clause(clause, value): # add clause a,b,c <= value
#	nonlocal neg_cardinality_cnf
	neg_cardinality_cnf.append(clause+[value])


# define letter squares
for k in range(letters):
	for x in range(letter_width):
		for y in range(letter_height):
			for b,r in enumerate(radii):
				CNF_variables["letter_{}_{}_{}_b{}".format(k,x,y,b)]

# wrap mappings
def map_x(x):
	while x<0:
		x += letter_width
	while x>=letter_width:
		x -= letter_width
	return x
def map_y(x):
	while x<0:
		x += letter_height
	while x>=letter_height:
		x -= letter_height
	return x

# define the blur variables
for k1 in tqdm(range(letters)):
	for x1 in range(letter_width):
		for y1 in range(letter_height):
			if min(x1+1,letter_width-x1)<=border_width or min(y1+1,letter_height-y1)<=border_width:
				add_clause([-CNF_variables["letter_{}_{}_{}_b0".format(k1,x1,y1)]])
			for b,r in enumerate(radii):
				if b==0:
					continue
				surrounding_vars = []
				if r>0:   # for positive radius, ie positive (outward) blur
					for xx in range(-r,r+1):
						for yy in range(-r, r+1):
							if xx**2+yy**2>r**2:
								continue
							vx = map_x(x1+xx)
							vy = map_y(y1+yy)
							surrounding_vars.append(CNF_variables["letter_{}_{}_{}_b0".format(k1,vx,vy)])
					v = CNF_variables["letter_{}_{}_{}_b{}".format(k1,x1,y1,b)]
				else:   # for negative radius, ie negative (inward) blur
					for xx in range(r,-r+1):
						for yy in range(r, -r+1):
							if xx**2+yy**2>r**2:
								continue
							vx = map_x(x1+xx)
							vy = map_y(y1+yy)
							surrounding_vars.append(CNF_variables["letter_{}_{}_{}_b{}".format(k1,vx,vy,b)])
					v = CNF_variables["letter_{}_{}_{}_b0".format(k1,x1,y1)]
				add_clause([-v]+surrounding_vars) # if all surrounding vars are false, then v is false
				for k in surrounding_vars: # if any surrounding var is true, then v is true
					add_clause([-k,v])


#    - - -           -
#      -           - - -


# define blur level differences, with transposition
for k1 in range(letters):
	print(k1)
	for k2 in tqdm(range(k1+1,letters)):
		for b,r in enumerate(radii):
			for x in range(letter_width):
				for y in range(letter_height):
					for tx in ttx:
						for ty in tty:
							var1 = CNF_variables["letter_{}_{}_{}_b{}".format(k1,x,y,b)]
							var2 = CNF_variables["letter_{}_{}_{}_b{}".format(k2,map_x(x+tx),map_y(y+ty),b)]
							var_diff = CNF_variables["diff_{}_{}_{}_{}_{}_{}_b{}".format(k1,k2,x,y,tx,ty,b)]
							add_clause([var_diff,-var1,var2])
							add_clause([var_diff,var1,-var2])
							add_clause([-var_diff,var1,var2])
							add_clause([-var_diff,-var1,-var2])
							

# add difference constraints
for k1 in range(letters):
	print(k1)
	for k2 in tqdm(range(k1+1,letters)):
		for b,r in enumerate(radii):
			#radii_difference = radii_differences[b]
			for tx in ttx:
				for ty in tty:
					difference_variables = []
					for x in range(letter_width):
						for y in range(letter_height):
							difference_variables.append(CNF_variables["diff_{}_{}_{}_{}_{}_{}_b{}".format(k1,k2,x,y,tx,ty,b)])
					#add_pos_cardinality_clause(difference_variables,radii_difference)
					add_pos_cardinality_clause(difference_variables,"{{radii_differences[{}]}}".format(b))

# output the CNF, and MAP file
print("outputting CNF and MAP")
with open("cnf.txt", "w") as f:
	f.write("p cnf+ {} {}\n".format(len(CNF_variables.keys()), len(cnf)+len(pos_cardinality_cnf)+len(neg_cardinality_cnf)))
	for c in tqdm(cnf):
		f.write("{} 0\n".format(" ".join([str(a) for a in c])))
	for c in tqdm(pos_cardinality_cnf):
		f.write("{} >= {}\n".format(" ".join([str(a) for a in c[:-1]]),c[-1]))
	for c in tqdm(neg_cardinality_cnf):
		f.write("{} <= {}\n".format(" ".join([str(a) for a in c[:-1]]),c[-1]))

with open("map.map","w") as f:
	for k,v in tqdm(CNF_variables.items()):
		f.write("{} {}\n".format(k,v))




