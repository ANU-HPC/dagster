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

try:
	import networkx as nx
	from networkx.drawing.nx_pydot import write_dot
	from random import sample
except ImportError:
	print("Welcome to Dagify utils")
	print("you need to install working python libraries: networkx\n")
	raise


# if input is castable to an int, then return the int otherwise return None
def RepresentsInt(s):
	try:
		return int(s)
	except ValueError:
		return None


# for a list of integers, sort and convert them into a formatted string
# eg. [1,3,7,4,5,9] becommes "1,3-5,7,9"
def compact_list_of_integers(l):
	l = sorted(l)
	terms = []
	while len(l)!=0:
		begin = l.pop(0)
		end = begin
		while (len(l)!=0) and (l[0] == end+1):
			end = l.pop(0)
		if begin==end:
			terms.append(str(begin))
		else:
			terms.append("{}-{}".format(begin,end))
	return ",".join(terms)

# for a list of x-y points, output an image named <image_output>
def output_image(points, image_output):
	try:
		from PIL import Image
	except ImportError:
		print("you need python PIL library for image export")
	else:
		maxx = max([i for i,j in points])
		maxy = max([j for i,j in points])
		minx = min([i for i,j in points])
		miny = min([j for i,j in points])
		img = Image.new('RGB', (maxx-minx+1, maxy-miny+1))
		for i,j in list(points):
			img.putpixel((i-minx,j-miny), (255,255,255))
		img.save(image_output)

#for a graph dg, output a dot file <dot_output> stripped of all annotations from nodes and edges
def output_dot(dg, dot_output):
	# create a graph copy and strip all annotations, and export
	dg2 = nx.DiGraph(dg)
	for n in list(dg2.nodes):
		dg2.nodes[n].clear()
	for i,j in list(dg2.edges):
		dg2[i][j].clear()
	write_dot(dg2, dot_output)


