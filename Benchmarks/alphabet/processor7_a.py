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



from collections import defaultdict
variable_index = 0
def a():
	global variable_index
	variable_index += 1
	return variable_index
CNF_variables = defaultdict(a)



with open("map.map","r") as f:
	for line in tqdm(f.readlines()):
		d = line.strip().split(" ")
		CNF_variables[d[0]] = int(d[1])

# process with minicard, and read back in the output
import os
os.system("minicard cnf2.txt zinky.txt")
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




for k in range(letters):
	print("'{}'".format(k))
	for y in range(letter_height):
		for x in range(letter_width):
			if processed_data[CNF_variables["letter_{}_{}_{}_b0".format(k,x,y)]]:
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

'''
