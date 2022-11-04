with open("dag_out.txt","r") as f:
	data = [(int(a)-1) for a in f.readlines()[0].split(" ") if (a!='\n' and int(a)>0)]
data = {(d//81,(d//9)%9):(d%9+1,d) for d in data}
print(data)
for y in range(9):
	for x in range(9):
		if (y,x) in data.keys():
			print(data[(y,x)][0], end ='')
	print("")
