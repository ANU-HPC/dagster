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
import cmath


letters = 2#15
letter_width = 15
letter_height = 15





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
	cnf.append(clause)



# define letter squares
for k in range(letters):
	for x in range(letter_width):
		for y in range(letter_height):
			CNF_variables["letter_{}_{}_{}".format(k,x,y)]
			

def quad_bound_or(term_dict1,lower1,upper1, term_dict2,lower2,upper2, term_dict3,lower3,upper3, term_dict4,lower4,upper4, aux_name1, aux_name2):
	# t1 >= upper1 - x(upper1-lower1) -y(upper1-lower1)
	# t2 >= upper2 - (1-x)(upper2-lower2) -y(upper2-lower2)
	#    >= lower2 + x(upper2-lower2) - y(upper2-lower2)
	# t3 >= upper3 - x(upper3-lower3) - (1-y)(upper3-lower3)
	#    >= lower3 - x(upper3-lower3) + y(upper3-lower3)
	# t4 >= upper4 - (1-x)(upper4-lower4) - (1-y)(upper4-lower4)
	#    >= -upper4 + 2*lower4 + x(upper4-lower4) +y(upper4-lower4)
	term_dict1 = term_dict1.copy()
	term_dict2 = term_dict2.copy()
	term_dict3 = term_dict3.copy()
	term_dict4 = term_dict4.copy()
	term_dict1[CNF_variables[aux_name1]] = upper1-lower1
	term_dict1[CNF_variables[aux_name2]] = upper1-lower1
	term_dict1["val"] = upper1
	term_dict2[CNF_variables[aux_name1]] = lower2-upper2
	term_dict2[CNF_variables[aux_name2]] = upper2-lower2
	term_dict2["val"] = lower2
	term_dict3[CNF_variables[aux_name1]] = upper3-lower3
	term_dict3[CNF_variables[aux_name2]] = lower3-upper3
	term_dict3["val"] = lower3
	term_dict4[CNF_variables[aux_name1]] = lower4-upper4
	term_dict4[CNF_variables[aux_name2]] = lower4-upper4
	term_dict4["val"] = -upper4 + 2*lower4
	return term_dict1,term_dict2,term_dict3,term_dict4
	


# consider all pairs of words, consider the difference in them with their blurs
# set constraints such that the blur difference must exceed some factor
print("pairing all words")


harshity = 3.2

for k1 in tqdm(range(letters)): #letter 1
	for k2 in range(k1+1,letters): #letter 2
			
			for xx in range(letter_width): # x-frequency xx
				for yy in range(letter_height): # y-frequency yy
					ex = cmath.exp(-2j*xx*cmath.pi/letter_width)
					ey = cmath.exp(-2j*yy*cmath.pi/letter_height)
					real_terms = defaultdict(lambda :0)
					imag_terms = defaultdict(lambda :0)
					# for given xy-frequency sum the terms to get the fourier component of that xy-frequency
					for x in range(letter_width): # comparing pixel x
						for y in range(letter_height): # comparing pixel y
							ee = ex**x*ey**y
							real_terms[CNF_variables["letter_{}_{}_{}".format(k1,x,y)]] += ee.real
							real_terms[CNF_variables["letter_{}_{}_{}".format(k2,x,y)]] -= ee.real
							imag_terms[CNF_variables["letter_{}_{}_{}".format(k1,x,y)]] += ee.imag
							imag_terms[CNF_variables["letter_{}_{}_{}".format(k2,x,y)]] -= ee.imag
					neg_real_terms = {k:-v for k,v in real_terms.items()}
					neg_imag_terms = {k:-v for k,v in imag_terms.items()}
					total_lower = -letter_width*letter_height
					[add_clause(c) for c in quad_bound_or(real_terms,total_lower,harshity, neg_real_terms,total_lower,harshity, imag_terms,total_lower,harshity, neg_imag_terms,total_lower,harshity,"aux_{}_{}_{}_{}_1".format(k1,k2,xx,yy),"aux_{}_{}_{}_{}_2".format(k1,k2,xx,yy))]




with open("map.map","w") as f:
	for k,v in tqdm(CNF_variables.items()):
		f.write("{} {}\n".format(k,v))



multiplier = 1000000

print("outputting OPB")
with open("zog.opb","w") as f:
	f.write("* #variable= {} #constraint= {}\n".format(len(CNF_variables.keys()), len(cnf)))
	for c in tqdm(cnf):
		keys = [cc for cc in c.keys() if isinstance(cc,int)]
		for k in keys:
			f.write("{}{} x{} ".format('+' if c[k]>0 else '',int(c[k]*multiplier),k))
		if "val" not in c.keys():
			print(c)
			print(c.keys())
		f.write(">= {} ;\n".format(int(c["val"]*multiplier)))






import os
os.system("~/Downloads/sat4j-2_3_6/dist/CUSTOM/sat4j-pb.jar CompetMinPBResLongWLMixedConstraintsObjectiveExpSimp ./zog.opb > delete_me.txt")
with open("delete_me.txt","r") as f:
	data = [d[2:].strip() for d in f.readlines() if len(d)>0 and d[0]=='v']

assert(len(data)==1)
data = "".join([dd for dd in data[0] if dd!='x'])
data = data.strip().split(" ")
data = [int(d) for d in data]
processed_data = {}
for d in data:
	if d>0:
		processed_data[d] = True
	else:
		processed_data[-d] = False
for k in range(letters):
	print("'{}'".format(k))
	for y in range(letter_height):
		for x in range(letter_width):
			if processed_data[CNF_variables["letter_{}_{}_{}".format(k,x,y)]]:
				print("{}  {}".format(bg('yellow'),attr('reset')),end='')
			else:
				print("{}  {}".format(bg('red'),attr('reset')),end='')
		print("")
	print("")


import numpy as np
from numpy.fft import fft2

import pdb
pdb.set_trace()

letter_arrays = {}
for k in range(letters):
	letter_arrays[k] = np.array([[1 if processed_data[CNF_variables["letter_{}_{}_{}".format(k,x,y)]] else 0 for x in range(letter_width)] for y in range(letter_height)])

print(abs(fft2(letter_arrays[1]) - fft2(letter_arrays[1])))

'''

for w in words: #[0:2]:
	print(w)
	for y in range(letter_height):
		for ww in w:
			for x in range(letter_widths[ww]):
				if processed_data[CNF_variables["letter_{}_{}_{}".format(ww,x,y)]]:
					print("{}  {}".format(bg('yellow'),attr('reset')),end='')
				else:
					print("{}  {}".format(bg('red'),attr('reset')),end='')
		print("")
	print("")



lorem_ipsum = "lorem ipsum dolor sit amet consectetur adipiscing elit sed do eiusmod tempor incididunt ut labore et dolore magna aliqua ut enim ad minim veniam quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur excepteur sint occaecat cupidatat non proident sunt in culpa qui officia deserunt mollit anim id est laborum"

lorem_ipsum = "cutting a cake is often used as a metaphor for allocating a divisible good the difficulty is not cutting the cake into pieces of equal size but rather that the cake is not uniformly tasty different agents prefer different parts of the cake depending on whether the toppings are strawberries or cookies the goal is to divide the cake in a way that is fair"

from PIL import Image
img_w = len(lorem_ipsum)*5
img_h = letter_height*30
newImg1 = Image.new('RGB', (img_w,img_h))

for x in range(img_w):
	for y in range(img_h):
		newImg1.putpixel((x,y),(255,255,255))

for w in [lorem_ipsum]: #[0:2]:
	print(w)
	for y in range(letter_height):
		upto_x = 0
		upto_y = 0
		space_counter = 0
		for ww in w:
			width = 9
			if ww!=' ':
				width = letter_widths[ww]
			else:
				space_counter += 1
				if space_counter==10:
					space_counter = 0
					upto_x = 0
					upto_y += 1
			for x in range(width):
				if ww!=' ' and processed_data[CNF_variables["letter_{}_{}_{}".format(ww,x,y)]]:
					newImg1.putpixel((upto_x+x,y+upto_y*int(letter_height*1.5)),(0,0,0))
				else:
					newImg1.putpixel((upto_x+x,y+upto_y*int(letter_height*1.5)),(255,255,255))
			upto_x += width
		print("")
	print("")

newImg1.save("img1.png")

'''
