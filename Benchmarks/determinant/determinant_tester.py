import click
import numpy as np
import pdb

def check(m):
	A = np.matrix(m)
	if (False in (A==A.transpose())):
		return 2
	Ainv = np.linalg.inv(A)
	print(Ainv)
	for x in range(A.shape[0]):
		for y in range(A.shape[1]):
			if A[x,y]==0 and Ainv[x,y]>0:
				return 0
			if A[x,y]==1 and Ainv[x,y]<0:
				return 0
	return 1


@click.command()
@click.argument('matrix', type=click.File('r'))
def hello(matrix):
	m = matrix.readlines()
	matrix.close()
	m = [mm.strip().split(" ") for mm in m]
	m = [[int(mmm) for mmm in mm if mmm!=''] for mm in m]
	for i in range(1,len(m)):
		assert len(m[i])==len(m[i-1]), "input matrix sizes are not square"
	c = check(m)
	if c==1:
		print('matrix is locally maximal')
	elif c==2:
		print('ERG, matrix is not symmetrical')
	else:
		print('NO, matrix is not locally maximal')

if __name__ == '__main__':
    hello()
