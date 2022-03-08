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

from processor import *
from copy import deepcopy as copy
from random import random, randint

def generate_forward_problem(original_lines,w,h):
	lines = copy(original_lines)
	print("Original MAP:")
	pretty_print_puzzle(lines)

	xybias = 0.5
	connect_bias = 0.9

	while (True): # continue while the puzzle is multiply soluble
		no_solutions,solution1,solution2,t = is_multiply_soluble(lines,w,h)
		if no_solutions==0:
			print("CANNOT MAKE PROBLEM")
			sys.exit(1)
		elif no_solutions==1:
			print("finished generating problem")
			break
		while (True): # continue trying to place random new constraint until success
			if random() >xybias: # place random xlink
				x = randint(0,w-2)
				y = randint(0,h-1)
				if (solution1[y][x] != solution1[y][x+1]):
					if (solution2[y][x] == solution2[y][x+1]):
						if (lines[y][x+1]!='#' and (lines[y][x+1] not in pentomino_keys)):
							if y<h-1:
								if (lines[y+1][x]!="|" and lines[y+1][x]!="+"): # if connected
									if random() < connect_bias:
										continue
							elif random() < connect_bias:
								continue
							if y>0:
								if (lines[y-1][x]!="|" and lines[y-1][x]!="+"): # if connected
									if random() < connect_bias:
										continue
							elif random() < connect_bias:
								continue
							if lines[y][x]==' ':
								lines[y] = change_char(lines[y],x,'|')
								#lines[y][x]='|'
								break
							elif lines[y][x]=='-':
								lines[y] = change_char(lines[y],x,'+')
								#lines[y][x]='+'
								break
			else: # place random ylink
				x = randint(0,w-1)
				y = randint(0,h-2)
				if (solution1[y][x] != solution1[y+1][x]):
					if (solution2[y][x] == solution2[y+1][x]):
						if (lines[y+1][x]!='#' and (lines[y+1][x] not in pentomino_keys)):
							if x<w-1:
								if (lines[y][x+1]!="-" and lines[y][x+1]!="+"): # if connected
									if random() < connect_bias:
										continue
							elif random() < connect_bias:
								continue
							if x>0:
								if (lines[y][x-1]!="-" and lines[y][x-1]!="+"): # if connected
									if random() < connect_bias:
										continue
							elif random() < connect_bias:
								continue
							if lines[y][x]==' ':
								lines[y] = change_char(lines[y],x,'-')
								#lines[y][x]='-'
								break
							elif lines[y][x]=='|':
								lines[y] = change_char(lines[y],x,'+')
								#lines[y][x]='+'
								break
		pretty_print_puzzle(lines)
	print("done generating")
	pretty_print_puzzle(lines)
	return lines,t







def generate_reverse_problem(original_lines,w,h, count_max):
	lines = copy(original_lines)
	print("Original MAP:")
	pretty_print_puzzle(lines)

	cnf, CNF_variables, CNF_variable_indices = generate_CNF_and_MAP(lines)
	original_solution, data = solve_minisat(cnf, CNF_variables,w,h)
	printSolution(original_solution)

	for x in range(w):
		for y in range(h):
			if original_lines[y][x]!='#':
				s0 = original_solution[y][x]
				if x<w-1:
					s1 = original_solution[y][x+1]
				else:
					s1 = s0
				if y<h-1:
					s2 = original_solution[y+1][x]
				else:
					s2 = s0
				if s0!=s1 and s0!=s2:
					lines[y] = change_char(lines[y],x,'+')
				elif s0!=s1:
					lines[y] = change_char(lines[y],x,'|')
				elif s0!=s2:
					lines[y] = change_char(lines[y],x,'-')
				else:
					lines[y] = change_char(lines[y],x,' ')
	pretty_print_puzzle(lines)

	set_x = None
	set_y = None
	set_char = None

	xybias = 0.5
	connectionbias = 0.04

	count = 0
	unique_t = 0
	while (True): # continue while the puzzle is multiply soluble
		no_solutions,solution1,solution2,t = is_multiply_soluble(lines,w,h)
		if no_solutions==0:
			print("CANNOT MAKE PROBLEM")
			return None
		elif no_solutions==1: # unique solution, try and remove a line
			unique_t = t
			if lines == original_lines:
				count += 1
			else:
				pretty_print_puzzle(lines)
				count = 0
			print("counter: {}, solve_time: {}".format(count,t))
			if count > count_max:
				break
			original_lines = copy(lines)
			while (True): # continue trying to remove a line
				if random() >xybias: # remove random xlink
					x = randint(0,w-1)
					y = randint(0,h-1)
					if lines[y][x]!='#':
						connections = 0
						if y>0 and (lines[y-1][x] == "|" or lines[y-1][x] == "+"):
							connections += 1
						if y<h-1 and (lines[y+1][x] == "|" or lines[y+1][x] == "+"):
							connections += 1
						if lines[y][x]=='|':
							if random()< connections*connectionbias:
								continue
							lines[y] = change_char(lines[y],x,' ')
							set_x = x
							set_y = y
							set_char = '|'
							break
						elif lines[y][x]=='+':
							if random()< connections*connectionbias:
								continue
							lines[y] = change_char(lines[y],x,'-')
							set_x = x
							set_y = y
							set_char = '+'
							break
				else: # remove random ylink
					x = randint(0,w-1)
					y = randint(0,h-1)
					if lines[y][x]!='#':
						connections = 0
						if x>0 and (lines[y][x-1] == "|" or lines[y][x-1] == "+"):
							connections += 1
						if x<w-1 and (lines[y][x+1] == "|" or lines[y][x+1] == "+"):
							connections += 1
						if lines[y][x]=='-':
							if random()< connections*connectionbias:
								continue
							lines[y] = change_char(lines[y],x,' ')
							set_x = x
							set_y = y
							set_char = '-'
							break
						elif lines[y][x]=='+':
							if random()< connections*connectionbias:
								continue
							lines[y] = change_char(lines[y],x,'|')
							set_x = x
							set_y = y
							set_char = '+'
							break
		elif no_solutions==2:
			lines[set_y] = change_char(lines[set_y],set_x,set_char)

	print("done generating")
	pretty_print_puzzle(lines)
	return lines, unique_t









