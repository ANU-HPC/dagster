import click
import numpy as np
import pdb

def swap(array, a,b):
	temp = array[b]
	array[b]=array[a]
	array[a]=temp
	
def calculate_lex(m, display=False):
	l = len(m)
	lex = 0
	for x in range(l):
		for y in range(l):
			if x+y>=l:
				continue
			lex = (lex << 1) + m[x+y][y]
	if display:
		l = lex
		s = ""
		while l!=0:
			s = str(l&1) + s
			l = l >> 1
		print(s)
	return lex

def calculate_lex1(m, display=False):
	l = len(m)
	lex = 0
	for x in range(l):
		for y in range(l):
			xx = x
			yy = y
			if xx<yy:
				continue
			if xx<0 or xx>=l:
				continue
			if yy<0 or yy>=l:
				continue
			lex = (lex << 1) + m[xx][yy]
	if display:
		l = lex
		s = ""
		while l!=0:
			s = str(l&1) + s
			l = l >> 1
		print(s)
	return lex
	
def rearrange(m):
	shift_made = True
	l = len(m)
	while shift_made==True:
		lex = calculate_lex(m)
		shift_made = False
		for i in range(l):
			for j in range(i+1,l):
				swap(m,i,j)
				for k in range(l):
					swap(m[k],i,j)
				new_lex = calculate_lex(m)
				if new_lex>lex:
					shift_made = True
					break
				else:
					swap(m,i,j)
					for k in range(l):
						swap(m[k],i,j)
			if shift_made:
				break

def print_matrix(m):
	for mm in m:
		print(" ".join([str(mmm) for mmm in mm]))

@click.command()
@click.argument('matrix', type=click.File('r'))
def hello(matrix):
	m = matrix.readlines()
	matrix.close()
	m = [mm.strip().split(" ") for mm in m]
	m = [[int(mmm) for mmm in mm if mmm!=''] for mm in m]
	for i in range(1,len(m)):
		assert len(m[i])==len(m[i-1]), "input matrix sizes are not square"
	print_matrix(m)
	#calculate_lex(m,True)
	print("")
	rearrange(m)
	print_matrix(m)
	#calculate_lex(m,True)

if __name__ == '__main__':
    hello()
