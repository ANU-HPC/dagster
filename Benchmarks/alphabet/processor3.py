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

# lay out all letter shapes
letter_widths = {'a':5,'b':5,'c':5,'d':5,'e':5,
	'f':5,'g':5,'h':5,'i':5,'j':5,'k':5,'l':5,
	'm':5,'n':5,'o':5,'p':5,'q':5,'r':5,'s':5,
	't':5,'u':5,'v':5,'w':5,'x':5,'y':5,'z':5}
letter_height = 12

with open("enwiki-20190320-words-frequency.txt","r") as f:
	d = f.readlines()
words = [dd.split(" ")[0] for dd in d[:40]]


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
for k in letter_widths.keys():
	for w in range(letter_widths[k]):
		for h in range(letter_height):
			CNF_variables["letter_{}_{}_{}".format(k,w,h)]

# all letters must be different:
print("considering all letters")
letter_difference_factor = 17
for k1 in tqdm(letter_widths.keys()):
	for k2 in letter_widths.keys():
		if k1!=k2 and letter_widths[k1]==letter_widths[k2]:
			letter_diff = []
			for x in range(letter_widths[k1]):
				for y in range(letter_height):
					d = CNF_variables["letter_{}_{}_diff_{}_{}".format(k1,k2,x,y)]
					letter_diff.append(d)
					add_clause([d,-CNF_variables["letter_{}_{}_{}".format(k1,x,y)],CNF_variables["letter_{}_{}_{}".format(k2,x,y)]])
					add_clause([d,CNF_variables["letter_{}_{}_{}".format(k1,x,y)],-CNF_variables["letter_{}_{}_{}".format(k2,x,y)]])
					add_clause([-d,-CNF_variables["letter_{}_{}_{}".format(k1,x,y)],-CNF_variables["letter_{}_{}_{}".format(k2,x,y)]])
					add_clause([-d,CNF_variables["letter_{}_{}_{}".format(k1,x,y)],CNF_variables["letter_{}_{}_{}".format(k2,x,y)]])
			add_pos_cardinality_clause(letter_diff,letter_difference_factor)

word_letter_mapping = {}
print("considering all single words")
for w in tqdm(words):
	word_width = sum([letter_widths[ww] for ww in w])
	# map letter xy to each word xy
	upto_x = 0
	for ww in w:
		for x in range(letter_widths[ww]):
			for y in range(letter_height):
				word_letter_mapping["word_{}_{}_{}".format(w,upto_x,y)] = CNF_variables["letter_{}_{}_{}".format(ww,x,y)]
			upto_x += 1

# consider all pairs of words, consider the difference in them with their blurs
# set constraints such that the blur difference must exceed some factor
print("pairing all words")
blur0_factor = 6
blur1_factor = 6
blur2_factor = 6
blur3_factor = 6
blur4_factor = 6
for w1 in tqdm(words):
	for w2 in words:
		min_len = min(len(w1),len(w2))
		if w1[0:min_len]==w2[0:min_len]:
			continue
		if w1<w2:
			word_width1 = sum([letter_widths[ww] for ww in w1])
			word_width2 = sum([letter_widths[ww] for ww in w2])
			
			min_word_width = min(word_width1,word_width2)
			
			diff_vars = []
			
			for x in range(min_word_width):
				for y in range(letter_height):
					for xx in range(min_word_width):
						for yy in range(letter_height):
							
					diff_var = CNF_variables["diff_word_{}_{}_{}_{}".format(w1,w2,x,y)]

					add_clause([diff_var,-word_letter_mapping["word_{}_{}_{}".format(w1,x,y)],word_letter_mapping["word_{}_{}_{}".format(w2,x,y)]])
					add_clause([diff_var,word_letter_mapping["word_{}_{}_{}".format(w1,x,y)],-word_letter_mapping["word_{}_{}_{}".format(w2,x,y)]])
					add_clause([-diff_var,-word_letter_mapping["word_{}_{}_{}".format(w1,x,y)],-word_letter_mapping["word_{}_{}_{}".format(w2,x,y)]])
					add_clause([-diff_var,word_letter_mapping["word_{}_{}_{}".format(w1,x,y)],word_letter_mapping["word_{}_{}_{}".format(w2,x,y)]])
					
					diff_vars.append(diff_var)

			add_pos_cardinality_clause(diff_vars,blur0_factor)
			


with open("map.map","w") as f:
	for k,v in tqdm(CNF_variables.items()):
		f.write("{} {}\n".format(k,v))


print("outputting OPB")
with open("zog.opb","w") as f:
	f.write("* #variable= {} #constraint= {}\n".format(len(CNF_variables.keys()), len(cnf)+len(pos_cardinality_cnf)+len(neg_cardinality_cnf)))
	for c in tqdm(cnf):
		for a in c:
			f.write("{} x{} ".format(-1 if a<0 else 1,abs(a)))
		f.write(">= {} ;\n".format(-len(c)+1))
	for c in tqdm(pos_cardinality_cnf):
		for a in c[:-1]:
			f.write("{} x{} ".format(-1 if a<0 else 1,abs(a)))
		f.write(">= {} ;\n".format(-len(c)+c[-1]))
	for c in tqdm(neg_cardinality_cnf):
		for a in c[:-1]:
			f.write("{} x{} ".format(-1 if a<0 else 1,abs(a)))
		f.write("<= {} ;\n".format(-len(c)+c[-1]))


import sys
sys.exit(0)


import os
os.system("minicard cnf.txt zinky.txt")
with open("zinky.txt","r") as f:
	data = f.readlines()
assert(len(data)==2)
data = data[1].strip().split(" ")
data = [int(d) for d in data]
processed_data = {}
for d in data:
	if d>0:
		processed_data[d] = True
	else:
		processed_data[-d] = False
for k in letter_widths.keys():
	print("'{}'".format(k))
	for y in range(letter_height):
		for x in range(letter_widths[k]):
			if processed_data[CNF_variables["letter_{}_{}_{}".format(k,x,y)]]:
				print("{}  {}".format(bg('yellow'),attr('reset')),end='')
			else:
				print("{}  {}".format(bg('red'),attr('reset')),end='')
		print("")
	print("")

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

'''for w in words[0:2]:
	print(w)
	for y in range(letter_height):
		word_width1 = sum([letter_widths[ww] for ww in w])
		for x in range(word_width1):
			if processed_data[word_letter_mapping["word_{}_{}_{}".format(w,x,y)]]:
				print("{}  {}".format(bg('yellow'),attr('reset')),end='')
			else:
				print("{}  {}".format(bg('red'),attr('reset')),end='')
		print("")
	print("")

for w in words[0:2]:
	print(w)
	for y in range(letter_height):
		word_width1 = sum([letter_widths[ww] for ww in w])
		for x in range(word_width1):
			if processed_data[CNF_variables["word_{}_{}_{}_blur1".format(w,x,y)]]:
				print("{}  {}".format(bg('yellow'),attr('reset')),end='')
			else:
				print("{}  {}".format(bg('red'),attr('reset')),end='')
		print("")
	print("")'''

'''
for w in words[0:2]:
	print(w)
	for y in range(letter_height):
		word_width1 = sum([letter_widths[ww] for ww in w])
		for x in range(word_width1):
			if processed_data[CNF_variables["word_{}_{}_{}_blur2".format(w,x,y)]]:
				print("{}  {}".format(bg('yellow'),attr('reset')),end='')
			else:
				print("{}  {}".format(bg('red'),attr('reset')),end='')
		print("")
	print("")
'''

'''for w1 in words[0:2]:
	for w2 in words[0:2]:
		min_len = min(len(w1),len(w2))
		if w1[0:min_len]==w2[0:min_len]:
			continue
		if w1<w2:
			print(w1,w2)
			word_width1 = sum([letter_widths[ww] for ww in w1])
			word_width2 = sum([letter_widths[ww] for ww in w2])
			
			min_word_width = min(word_width1,word_width2)
			
			for y in range(letter_height):
				for x in range(min_word_width):
					if processed_data[CNF_variables["diff_word_{}_{}_{}_{}".format(w1,w2,x,y)]]:
						print("{}  {}".format(bg('yellow'),attr('reset')),end='')
					else:
						print("{}  {}".format(bg('red'),attr('reset')),end='')
				print("")
			print("")

for w1 in words[0:2]:
	for w2 in words[0:2]:
		min_len = min(len(w1),len(w2))
		if w1[0:min_len]==w2[0:min_len]:
			continue
		if w1<w2:
			print(w1,w2)
			word_width1 = sum([letter_widths[ww] for ww in w1])
			word_width2 = sum([letter_widths[ww] for ww in w2])
			
			min_word_width = min(word_width1,word_width2)
			
			for y in range(letter_height):
				for x in range(min_word_width):
					if processed_data[CNF_variables["diff_word1_{}_{}_{}_{}".format(w1,w2,x,y)]]:
						print("{}  {}".format(bg('yellow'),attr('reset')),end='')
					else:
						print("{}  {}".format(bg('red'),attr('reset')),end='')
				print("")
			print("")

for w1 in words[0:2]:
	for w2 in words[0:2]:
		min_len = min(len(w1),len(w2))
		if w1[0:min_len]==w2[0:min_len]:
			continue
		if w1<w2:
			print(w1,w2)
			word_width1 = sum([letter_widths[ww] for ww in w1])
			word_width2 = sum([letter_widths[ww] for ww in w2])
			
			min_word_width = min(word_width1,word_width2)
			
			for y in range(letter_height):
				for x in range(min_word_width):
					if processed_data[CNF_variables["diff_word2_{}_{}_{}_{}".format(w1,w2,x,y)]]:
						print("{}  {}".format(bg('yellow'),attr('reset')),end='')
					else:
						print("{}  {}".format(bg('red'),attr('reset')),end='')
				print("")
			print("")
'''


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


