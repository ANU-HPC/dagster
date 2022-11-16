from random import randint
from random import random

x_mul = 0.45
y_mul = -0.35

def gen_superior(array,weight):
	array[0] = 0
	array[1] = 0
	a = [1,1]
	length = len(array)
	for i in range(2,length):
		r = 1 if random()<weight else 0
		if sum(a)+r>sum(array[:i+1])+1:
			a.append(r)
		else:
			a.append(1)
	return a


def return_tree(sx,sy,array,crosses=False,color="black",purge_first=True):
	length = len(array)
	x = sx
	for d in range(sy,length+sy):
		print("\draw[fill={},draw=none] ({},{}) circle (0.06);".format(color,x*x_mul,d*y_mul))
		v = array[d-sy]
		new_x = x + 2*v-1
		non_new_x = x + 2*(1-v)-1
		print("\draw[line width=1pt,{}] ({},{}) -- ({},{});".format(color,x*x_mul,d*y_mul,new_x*x_mul,(d+1)*y_mul))
		if d!=sy or not purge_first:
			print("\draw[line width=1pt,{}] ({},{}) -- ({},{});".format(color,x*x_mul,d*y_mul,non_new_x*x_mul,(d+1)*y_mul))
			xx = non_new_x*x_mul
			yy = d+1
			alpha = 0.08
			print("\draw[fill=white,draw=none] ({},{}) circle (0.27);".format(xx,yy*y_mul))
			if crosses:
				print("\draw[line width=1pt,{}] ({},{}) -- ({},{});".format(color,xx-alpha,yy*y_mul-alpha,xx+alpha,yy*y_mul+alpha))
				print("\draw[line width=1pt,{}] ({},{}) -- ({},{});".format(color,xx-alpha,yy*y_mul+alpha,xx+alpha,yy*y_mul-alpha))
		if color=="black" and d>6 and d%4==0:
			print("\draw[line width=1pt,{}] ({},{}) -- ({},{});".format("blue",x*x_mul-0.7,d*y_mul-0.15,x*x_mul+0.7,d*y_mul-0.15))
		x = new_x
	

A = 5
B = 9
C = 9
D = 10
E = 9

z = [0]*A
z = gen_superior(z,0.51)
return_tree(0,0,z)

l = [0]*B
l = gen_superior(l,0.51)
p = gen_superior(l,0.51)
p = p[:C]+[1,1,0]
return_tree(sum(z)*2-A,A,p,False,"lightgray")
return_tree(sum(z)*2-A,A,l,False,"black")

j = [0]*D
j = gen_superior(j,0.51)
k = gen_superior(j,0.51)
k = k[:E]
return_tree((sum(z)+sum(l))*2-(A+B),(A+B),k,False,"lightgray")
return_tree((sum(z)+sum(l))*2-(A+B),(A+B),j,True,"black")



