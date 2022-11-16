import re
import math
import numpy as np
from collections import defaultdict

width = 900
header = '<svg xmlns="http://www.w3.org/2000/svg" version="1.1" xmlns:xlink="http://www.w3.org/1999/xlink" height="{0}" width="{0}"><rect width="{0}" height="{0}" style="fill:rgb(255,255,255)" />\n'.format(width)
header = '<svg xmlns="http://www.w3.org/2000/svg" version="1.1" xmlns:xlink="http://www.w3.org/1999/xlink" height="{0}" width="{0}" style="font-size: medium;"  viewBox="0 -30 {0} {0}">\n'.format(width)
tail = '</svg>'



cnf = [[1, 2, 3],[3, 4, 5],[5, 6],[-3, -5],[-2, 3, -4],[-4, 5, -6],[1, 2, 4],[3, 4, -6],[-2,-3,4,-6],[-2,-3]]
instructions = [[-2],[1],[-5],[float('inf')]]
#instructions = [[-2],[4],[-5],[-6],[6],[-6],[6],[-6],[6],[-6],[6],[None]]

output_svg = open("svg_out2.svg","w")

output_svg.write('<!--{{ "setup": [\n{}\n]}}-->\n'.format(",\n".join(['{{ "element": "#part_{0}", "modifier": "attr", "parameters": [ {{"class": "fragment", "data-fragment-index": "{0}"}} ] }}'.format(i) for i in range(len(instructions))])  ))
output_svg.write(header)



def write_assignments_2(array, offset_x, offset_y, spacing, styling):
	for i,a in enumerate(array):
		x = offset_x+i*spacing
		y = offset_y
		if styling is not None:
			style = styling[a]
		else:
			style = ""
		output_svg.write('<circle cx="{}" cy="{}" r="{}" style="{}"/>\n'.format(x,y,10,style))
		output_svg.write('<text x="{}" y="{}" fill="black" dominant-baseline="middle" text-anchor="middle" alignment-baseline="middle" >{}</text>\n'.format(x,y,a))


def write_assignments(var_assignments, offset_y):
	bar_labels = []
	for v in var_assignments:
		if v is None:
			bar_labels.append('N')
		if v is True:
			bar_labels.append('T')
		if v is False:
			bar_labels.append('F')
	spacing = 30
	offset_x = width-len(var_assignments)*spacing
	styling = {'N':"fill:rgb(0,200,255);",'F':"fill:rgb(0,100,255);",'T':"fill:rgb(128,255,0);"}
	for i,a in enumerate(bar_labels):
		x = offset_x+i*spacing
		y = offset_y
		if styling is not None:
			style = styling[a]
		else:
			style = ""
		output_svg.write('<circle cx="{}" cy="{}" r="{}" style="{}"/>\n'.format(x,y,10,style))
		output_svg.write('<text x="{}" y="{}" fill="black" dominant-baseline="middle" text-anchor="middle" alignment-baseline="middle" >{}</text>\n'.format(x,y,a))

def write_cnf(cnf,x,y, box_w, box_h, var_assignments):
	dx = 12
	dy = 24
	#output_svg.write('<rect x="{}" y="{}" width="{}" height="{}" style="fill: pink;"/>'.format(x,y,box_w,box_h))
	for i in range(len(cnf)):
		s = ""
		satisfied = False
		for j in cnf[i]:
			if j<0:
				if var_assignments[-j-1] == False:
					satisfied = True
			else:
				if var_assignments[j-1] == True:
					satisfied = True
		for j in cnf[i]:
			satisfying = False
			var_val = None
			if j<0:
				var_val = var_assignments[-j-1]
				if var_val == False:
					satisfying = True
			else:
				var_val = var_assignments[j-1]
				if var_val == True:
					satisfying = True
			if satisfied and satisfying:
				s += "<tspan style='fill:cornflowerblue;'>{}</tspan> ".format(j)
			elif satisfied:
				s += "<tspan style='fill:blue;'>{}</tspan> ".format(j)
			elif var_val is None:
				s += "<tspan style='fill:gray;'>{}</tspan> ".format(j)
			else:
				s += "<tspan style='fill:red;'>{}</tspan> ".format(j)
		output_svg.write('<text x="{}" y="{}" alignment-baseline="hanging" style="font-size: x-large;">{}</text>\n'.format(x+dx/2,y+i*dy+dy,s ))
		


def add_group(identifier):
	output_svg.write("<g id={}>".format(identifier))
def end_group():
	output_svg.write("</g>")


def draw_tree(cnf, offset, instructions, style1, style2, circle_width, transform, var_assignments):
	variables = max([max([abs(aa) for aa in a]) for a in cnf])
	coordinates = [np.matrix([[0],[0]])]
	for i, instruction in enumerate(instructions):
		old_coord = coordinates[-1]
		new_coord = old_coord.copy()
		
		if instruction is None: #undo
			transformed_old = transform*old_coord + offset
			transformed_old_old = transform*coordinates[-2] + offset
			write_assignments(var_assignments[-1], transformed_old_old[1,0])
			#write_cnf(cnf,0,0,100,500, var_assignments[-1])
			output_svg.write('<circle cx="{}" cy="{}" r="{}" style="{}"/>\n'.format(transformed_old[0,0],transformed_old[1,0],circle_width,"fill:white;"))
			output_svg.write('<circle cx="{}" cy="{}" r="{}" style="{}"/>\n'.format(transformed_old[0,0],transformed_old[1,0]-6,2,"fill:black;"))
			output_svg.write('<circle cx="{}" cy="{}" r="{}" style="{}"/>\n'.format(transformed_old[0,0],transformed_old[1,0],2,"fill:black;"))
			output_svg.write('<circle cx="{}" cy="{}" r="{}" style="{}"/>\n'.format(transformed_old[0,0],transformed_old[1,0]+6,2,"fill:black;"))
			#output_svg.write('<line x1="{}" y1="{}" x2="{}" y2="{}" style="{}" />\n'.format(transformed_old[0,0]-5,transformed_old[1,0]-5,transformed_old[0,0]+5,transformed_old[1,0]+5,"stroke:red;stroke-width:2"))
			#output_svg.write('<line x1="{}" y1="{}" x2="{}" y2="{}" style="{}" />\n'.format(transformed_old[0,0]-5,transformed_old[1,0]+5,transformed_old[0,0]+5,transformed_old[1,0]-5,"stroke:red;stroke-width:2"))
			coordinates.pop()
			var_assignments.pop()
			continue
		elif instruction == float('inf'):
			transformed_old = transform*old_coord + offset
			transformed_old_old = transform*coordinates[-2] + offset
			write_assignments(var_assignments[-1], transformed_old_old[1,0])
			#write_cnf(cnf,0,0,100,500, var_assignments[-1])
			output_svg.write('<circle cx="{}" cy="{}" r="{}" style="{}"/>\n'.format(transformed_old[0,0],transformed_old[1,0],circle_width,"fill:white;"))
			output_svg.write('<line x1="{}" y1="{}" x2="{}" y2="{}" style="{}" />\n'.format(transformed_old[0,0]-5,transformed_old[1,0]-5,transformed_old[0,0]+0,transformed_old[1,0]+5,"stroke:green;stroke-width:2"))
			output_svg.write('<line x1="{}" y1="{}" x2="{}" y2="{}" style="{}" />\n'.format(transformed_old[0,0]+0,transformed_old[1,0]+5,transformed_old[0,0]+10,transformed_old[1,0]-5,"stroke:green;stroke-width:1.7"))
			continue
		elif instruction>0: #assignment True
			text = "{}".format(instruction)
			new_coord[1] += 1
			v2 = var_assignments[-1].copy()
			v2[instruction-1] = True
			var_assignments.append(v2)
			coordinates.append(new_coord)
		elif instruction<0: #assignment False
			text = "¬{}".format(-instruction)
			text = "{}".format(-instruction)
			new_coord[0] += 1
			v2 = var_assignments[-1].copy()
			v2[-instruction-1] = False
			var_assignments.append(v2)
			coordinates.append(new_coord)
		transformed_old = transform*old_coord + offset
		transformed_new = transform*new_coord + offset
		write_assignments(var_assignments[-1], transformed_old[1,0])
		#write_cnf(cnf,0,0,100,500, var_assignments[-1])
		output_svg.write('<line x1="{}" y1="{}" x2="{}" y2="{}" style="{}" />\n'.format(transformed_old[0,0],transformed_old[1,0],transformed_new[0,0],transformed_new[1,0],style1))
		output_svg.write('<circle cx="{}" cy="{}" r="{}" style="{}"/>\n'.format(transformed_old[0,0],transformed_old[1,0],circle_width,style2))
		output_svg.write(
		'<text x="{}" y="{}" fill="black" dominant-baseline="middle" text-anchor="middle" alignment-baseline="middle" >{}</text>\n'.format(transformed_old[0,0],transformed_old[1,0],text))
	write_cnf(cnf,0,0,100,500, var_assignments[-1])
		


def draw_tree2(cnf, offset, overall_instructions, style1, style2, circle_width, transform):
	variables = max([max([abs(aa) for aa in a]) for a in cnf])
	var_assignments = [[False,True,True,False,True,True]]
	write_assignments_2(list(range(1,variables+1)),width-variables*30,0,30,defaultdict(lambda :"fill:none; stroke:black; stroke-width:1px"))
	write_assignments(var_assignments[-1], 30)
	write_cnf(cnf,0,0,100,500, var_assignments[-1])
	instructions = []
	for i, instruction_group in enumerate(overall_instructions):
		output_svg.write("<g id='part_{}'>".format(i))
		output_svg.write('<rect x="0" y="-30" width="{0}" height="{1}" style="fill:rgb(253,246,227)" />'.format(width,width+30)) #fdf6e3
		write_assignments_2(list(range(1,variables+1)),width-variables*30,0,30,defaultdict(lambda :"fill:none; stroke:black; stroke-width:1px"))
		write_assignments(var_assignments[-1], 30)
		for instruction in instruction_group:
			if instruction is None and len(instructions)>=3: #6,None,-6,None,None
				if instruction is None and instructions[-1] is None:
					instructions.pop()
					instructions.pop()
					instructions.pop()
					instructions.pop()
			instructions.append(instruction)
			print(instructions)
		draw_tree(cnf, offset, instructions, style1, style2, circle_width, transform, var_assignments.copy())
		output_svg.write("</g>")


draw_tree2(cnf, np.matrix([[width/2],[60]]), instructions, "stroke:black;stroke-width:2", "fill:white; stroke-width:0.7; stroke:black;", 15, np.matrix([[-30,30],[30,30]]).transpose())

#write_cnf(cnf,10,10,50,50, [True, False, None, False, None, True, None, False, None])


# append the SVG tail, and done
output_svg.write(tail)

output_svg.close()


