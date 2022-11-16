ith open("su_output.txt","r") as f:
	data = "".join([str(((int(a)-1)%9)+1) for a in f.readlines()[1].split(" ") if int(a)>0])
for i in range(9):
	print(data[9*i:9*i+9])
