with open("map.txt","r") as f:
	lines = f.readlines()
	map_file = [l.strip().split(" ") for l in lines]
map_file = [[l[0],int(l[1])] for l in map_file if l[0][0:2]=="m_"]
map_file = {l[1]:l[0] for l in map_file}
print(map_file)



N = 0
for k in map_file.values():
	kk = k.split("_")
	k1 = int(kk[1])
	k2 = int(kk[2])
	if k1>N:
		N = k1
	if k2>N:
		N = k2
N += 1
print("detected N of {}".format(N))

with open("dag_out.txt","r") as f:
	lines = f.readlines()
	solutions = [l.strip().split(" ") for l in lines]
solutions = [[map_file[int(ss)] for ss in s if int(ss)>0] for s in solutions]

for i,sol in enumerate(solutions):
	print("")
	print(lines[i])
	for y in range(N):
		for x in range(N):
			if "m_{}_{}".format(x,y) in sol:
				print("1 ",end="")
			else:
				print("0 ",end="")
		print("")






