import numpy as np

n = 6
A = np.round(np.random.rand(n,n))

n = 3
A = np.array([[1,3,5],[2,4,6],[3,5,7]])

print(A)

B = np.random.rand(n,n)*0
C = np.random.rand(n,n)*0

print(B)
import pdb
pdb.set_trace()
for i in range(n):
	a = A[:,i]
	C[i,i]=1
	for j in range(i):
		bb = B[:,j]
		print("dotba={}".format(np.dot(bb,a)))
		dd = np.dot(bb,a)/np.dot(bb,bb)
		print("dd={}".format(dd))
		C[j,i]=dd
		a = a - bb*dd
		print("setting column {} to {}".format(i,a))
	B[:,i] = a

print(B)
print(C)
print(np.matmul(B,C)-A)

'''Bneg = np.linalg.inv(B)
print(Bneg)

print(Bneg*A)
print(A*Bneg)
'''



print("")
A = np.array([[1.0,3.0,9.0],[2.0,4.0,10.0],[3.0,5.0,11.0]])
for i in range(n):
	for j in range(i):
		bb = A[:,j]
		dd = np.dot(bb,A[:,i])/np.dot(bb,bb)
		A[:,i] = A[:,i] - bb*dd
print(A)




'''for i in range(n):
	for j in range(i):
		bb = A[:,j]
		dd = np.dot(bb,A[:,i])/np.dot(bb,bb)
		A[:,i] = A[:,i] - bb*dd'''
