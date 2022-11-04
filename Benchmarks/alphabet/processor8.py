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
from pysat.card import *
from pysat import formula
import pdb
from collections import defaultdict
import click


class CNF_(object):
	variable_index = None
	CNF_variables = None
	cnf = None
	pos_cardinality_cnf = None
	neg_cardinality_cnf = None
	
	def a(self):
		self.variable_index += 1
		return self.variable_index
	def add_clause(self,clause):
		self.cnf.append(clause)
	def add_pos_cardinality_clause(self,clause, value): # add clause a,b,c >= value
		self.pos_cardinality_cnf.append(clause+[value])
	def add_neg_cardinality_clause(self,clause, value): # add clause a,b,c <= value
		self.neg_cardinality_cnf.append(clause+[value])

	def __init__(self):
		self.variable_index = 0
		self.CNF_variables = defaultdict(self.a)
		self.cnf = []
		self.pos_cardinality_cnf = []
		self.neg_cardinality_cnf = []


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



@click.command()
@click.argument('config_file', type=click.types.Path(), default="header.json")
@click.argument('cnf_file', type=click.types.Path(), default="cnf.txt")
@click.argument('map_file', type=click.types.Path(), default="map.txt")
@click.argument('dag_file', type=click.types.Path(), default="dag.txt")
def alphabet_problem(config_file, cnf_file, map_file, dag_file):
	with open(config_file,"r") as f:
		header_data = json.load(f)
		for k in header_data.keys():
			globals()[k] = header_data[k]
	c = CNF_()

	# define letter squares
	for k in range(letters):
		for x in range(letter_width):
			for y in range(letter_height):
				c.CNF_variables["letter_{}_{}_{}_r0".format(k,x,y)]
	for k in range(letters):
		for x in range(letter_width):
			for y in range(letter_height):
				for b,r in enumerate(radii):
					c.CNF_variables["letter_{}_{}_{}_r{}".format(k,x,y,r)]
	
	# force the boarders to be empty
	for k1 in tqdm(range(letters)):
		for x1 in range(letter_width):
			for y1 in range(letter_height):
				if min(x1+1,letter_width-x1)<=border_width or min(y1+1,letter_height-y1)<=border_width:
					c.add_clause([-c.CNF_variables["letter_{}_{}_{}_r0".format(k1,x1,y1)]])
	
	# define the blur variables
	for k1 in tqdm(range(letters)):
		for x1 in range(letter_width):
			for y1 in range(letter_height):
				for b,r in enumerate(radii):
					if r==0:
						continue
					surrounding_vars = []
					if r>0:   # for positive radius, ie positive (outward) blur
						for xx in range(-r,r+1):
							for yy in range(-r, r+1):
								if xx**2+yy**2>r**2:
									continue
								vx = map_x(x1+xx)
								vy = map_y(y1+yy)
								surrounding_vars.append(c.CNF_variables["letter_{}_{}_{}_r0".format(k1,vx,vy)])
						v = c.CNF_variables["letter_{}_{}_{}_r{}".format(k1,x1,y1,r)]
					else:   # for negative radius, ie negative (inward) blur
						for xx in range(r,-r+1):
							for yy in range(r, -r+1):
								if xx**2+yy**2>r**2:
									continue
								vx = map_x(x1+xx)
								vy = map_y(y1+yy)
								surrounding_vars.append(c.CNF_variables["letter_{}_{}_{}_r{}".format(k1,vx,vy,r)])
						v = c.CNF_variables["letter_{}_{}_{}_r0".format(k1,x1,y1)]
					c.add_clause([-v]+surrounding_vars) # if all surrounding vars are false, then v is false
					for k in surrounding_vars: # if any surrounding var is true, then v is true
						c.add_clause([-k,v])
	
	#    - - -           -
	#      -           - - -
	# define blur level differences, with transposition
	for k1 in range(letters):
		print(k1)
		for k2 in tqdm(range(k1+1,letters)):
			for b,r in enumerate(radii):
				for x in range(letter_width):
					for y in range(letter_height):
						var1 = c.CNF_variables["letter_{}_{}_{}_r{}".format(k1,x,y,r)]
						var2 = c.CNF_variables["letter_{}_{}_{}_r{}".format(k2,map_x(x),map_y(y),r)]
						var_diff = c.CNF_variables["diff_{}_{}_{}_{}_r{}".format(k1,k2,x,y,r)]
						c.add_clause([var_diff,-var1,var2])
						c.add_clause([var_diff,var1,-var2])
						c.add_clause([-var_diff,var1,var2])
						c.add_clause([-var_diff,-var1,-var2])

	# add difference constraints
	for k1 in range(letters):
		print(k1)
		for k2 in tqdm(range(k1+1,letters)):
			for b,r in enumerate(radii):
				difference_variables = []
				for x in range(letter_width):
					for y in range(letter_height):
						difference_variables.append(c.CNF_variables["diff_{}_{}_{}_{}_r{}".format(k1,k2,x,y,r)])
				c.add_pos_cardinality_clause(difference_variables,radii_differences[b])

	# output the CNF, and MAP file
	print("outputting CNF and MAP")
	with open(cnf_file, "w") as f:
		f.write("p cnf+ {} {}\n".format(len(c.CNF_variables.keys()), len(c.cnf)+len(c.pos_cardinality_cnf)+len(c.neg_cardinality_cnf)))
		for cc in tqdm(c.cnf):
			f.write("{} 0\n".format(" ".join([str(a) for a in cc])))
		for cc in tqdm(c.pos_cardinality_cnf):
			f.write("{} >= {}\n".format(" ".join([str(a) for a in cc[:-1]]),cc[-1]))
		for cc in tqdm(c.neg_cardinality_cnf):
			f.write("{} <= {}\n".format(" ".join([str(a) for a in cc[:-1]]),cc[-1]))

	with open(map_file,"w") as f:
		for k,v in tqdm(c.CNF_variables.items()):
			f.write("{} {}\n".format(k,v))




if __name__ == '__main__':
	alphabet_problem()




