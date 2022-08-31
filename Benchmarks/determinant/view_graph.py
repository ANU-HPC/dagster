import click
import re
import math

width = 200
radius = 80
text_radius = 90
header = '<svg height="{0}" width="{0}"><rect width="{0}" height="{0}" style="fill:rgb(255,255,255)" />'.format(width)
tail = '</svg>'
colours = [(255,0,0),(0,255,0),(0,0,255),(0,0,0),(255,255,0),(255,0,255),(0,255,255),(128,128,128)]

'''
Pass in a binary matrix, view it as svg
'''
@click.command()
@click.argument('matrix_file', type=click.File('r'))
@click.argument('output_svg', type=click.File('w'))
def view_ramsay(matrix_file, output_svg):

	matrix = [l.strip().split(" ") for l in matrix_file.readlines()]
	matrix = [[int(i) for i in l] for l in matrix]
	
	true_variables = []
	max_variable = 0
	for v1,ml in enumerate(matrix):
		for v2,m in enumerate(ml):
			if m==1:
				true_variables.append((v1,v2,1))
			if v2>max_variable:
				max_variable = v2
		if v1>max_variable:
			max_variable = v1
	
	# output vertex labels in the SVG
	output_svg.write(header)
	for i in range(max_variable+1):
		radians = 2*math.pi*i*1.0/(max_variable+1)
		output_svg.write('<text x="{}" y="{}" fill="black" dominant-baseline="middle" text-anchor="middle">{}</text>'.format(width/2+text_radius*math.cos(radians),width/2+text_radius*math.sin(radians),i))

	# output coloured edges in the SVG
	for v1,v2,c in true_variables:
		r1 = 2*math.pi*v1*1.0/(max_variable+1)
		r2 = 2*math.pi*v2*1.0/(max_variable+1)
		x1 = width/2 + radius*math.cos(r1)
		y1 = width/2 + radius*math.sin(r1)
		x2 = width/2 + radius*math.cos(r2)
		y2 = width/2 + radius*math.sin(r2)
		colour = colours[c]
		output_svg.write('<line x1="{}" y1="{}" x2="{}" y2="{}" style="stroke:rgb{};stroke-width:2" />'.format(x1,y1,x2,y2,colour))
	
	# append the SVG tail, and done
	output_svg.write(tail)
	
	
if __name__ == '__main__':
    view_ramsay()
