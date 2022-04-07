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


'''
	WITNESS CNF generator:
    ------------------

	create an input file of characters in a square - characters of 'A','B' or '_',
	for 'A' belonging to the region of the top left, for 'B' belonging to region of the bottorm right , or for '_' undecided.
	the program will generate an CNF file of litterals an clauses, and a key file giving the description to what the litterals correspond to (in terms of grid positions etc)

	the usage of the program is, calling: 

		python witness.py <input_file> <cnf_output> <key_output>

	where <input_file> is problem input file
	where <cnf_output> is output of the problem in CNF
	where <key_output> is a json-formatted array file

	the key_output is a json formatted array file with the first index being the size of the block +2, and the rest being the details of the litteral at the index.
'''
import time
start_time = time.time()

# Open the input file into an array.
import sys
assert len(sys.argv)==4, "You must supply an input problem file, an output CNF filename, and an output CNF key file as arguments"
with open(sys.argv[1], "r") as f:
	lines = [l.strip() for l in f.readlines()]

# Check the input file is square of characters
n = len(lines[0])
assert n==len(lines), "input file needs to be square"
for l in lines:
	assert n==len(l), "input file needs to be square"


max_len = sum([i for i in range(1,n+1)])		#maximum possible depth of any tree branch
n += 2											#add 2 to n for inclusion of left and right super-blocks


cnf = []										# begin with an empty list of CNF clauses

litteral_dictionary = [0]						# keep an active running list of litterals by their index in an array, zero is dummy index

# for a specification s, if a litteral dosnt exist for that specification then add it to the list.
# and return the index corresponding to that specification
def get(*s):
	if s not in litteral_dictionary:
		litteral_dictionary.append(s)
	return litteral_dictionary.index(s)

# load CNF clauses corresponding to only one of the list of specificated litterals being true
def exactly_one_of(spec_array):
	cnf.append([get(*s) for s in spec_array])
	for i,s in enumerate(spec_array[:-1]):
		for s2 in spec_array[i+1:]:
			cnf.append([-get(*s),-get(*s2)])


super_block_left = []
super_block_right = []				# the cartesian coordinates of the middle-square, and left and right super-blocks
mid_block = []

for x in range(1,n-1):
	for y in range(1,n-1):
		mid_block.append((x,y))
for i in range(n):
	super_block_left.append((0,i))
	super_block_right.append((n-1,i))
for i in range(1,n-1):
	super_block_left.append((i,0))
	super_block_right.append((i,n-1))


# for each coordinate of the middle-block, 
#it has to either belong to the left or right region, 
# and has to be assigned exactly one (non-zero) depth.
for x,y in mid_block:
	exactly_one_of([(x,y,True,level) for level in range(1,max_len+1)]+
				[(x,y,False,level) for level in range(1,max_len+1)])
	cnf.append([-get(x,y,True,0)])
	cnf.append([-get(x,y,False,0)])

# for each coordinate of the middle-block,
# it has to be surrounded by a block of the same color region one less depth than it
for x,y in mid_block:
	for pos in [True,False]:
		for level in range(1,max_len+1):
			cnf.append([-get(x,y,pos,level),
						get(x+1,y,pos,level-1),
						get(x,y+1,pos,level-1),
						get(x-1,y,pos,level-1),
						get(x,y-1,pos,level-1)])

# for blocks on left, it has to be of one region at a depth of zero only
for x,y in super_block_left:
	cnf.append([get(x,y,True,0)])
	for i in range(1,max_len+1):
		cnf.append([-get(x,y,True,i)])
	for i in range(0,max_len+1):
		cnf.append([-get(x,y,False,i)])

# for blocks on right, it has to be of the other region at a depth of zero only
for x,y in super_block_right:
	cnf.append([get(x,y,False,0)])
	for i in range(1,max_len+1):
		cnf.append([-get(x,y,False,i)])
	for i in range(0,max_len+1):
		cnf.append([-get(x,y,True,i)])

# for the characters of the input file, add the constraints that thoes positions
#  must belong to thoes regions
for y,l in enumerate(lines):
	for x,ll in enumerate(l):
		if ll=='A':
			cnf.append([get(x+1,y+1,True,level) for level in range(1,max_len+1)])
		elif ll=='B':
			cnf.append([get(x+1,y+1,False,level) for level in range(1,max_len+1)])
		elif ll=="_":
			pass
		else:
			raise Exception("INPUT FILE HAS BAD CHARACTER!")

# remove the dummy index, spit some debug info, and output CNF file
del litteral_dictionary[0]
print "{} litterals".format(len(litteral_dictionary))
print "{} clauses".format(len(cnf))

with open(sys.argv[2],"w") as f:
	f.write("p cnf {} {}\n".format(len(litteral_dictionary), len(cnf)))
	for c in cnf:
		f.write("{} 0\n".format(" ".join([str(cc) for cc in c])))

# output the keys corresponding to what information (grid position x,y and region) 
#  identifies what litteral in the CNF file
import json
litteral_dictionary = [(s[0],s[1],s[2]) for s in litteral_dictionary]
litteral_dictionary = [n]+litteral_dictionary
with open(sys.argv[3],"w") as f:
	json.dump(litteral_dictionary,f)


print "Finished generating CNF file in {} seconds".format(time.time()-start_time)

