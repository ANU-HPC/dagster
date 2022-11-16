from collections import defaultdict

with open("costas_runtime.csv","r") as f:
	lines = f.readlines()
	header = lines[0].strip().split(",")
	data = [d.strip().split(",") for d in lines[1:]]
tinisat_dict = {}
data_dict = defaultdict(dict)
size_array = []
for d in data:
	processors = int(d[1])
	size = int(d[0])
	data_dict[processors][size] = float(d[2])
	tinisat_dict[size] = float(d[3])
	if size not in size_array:
		size_array.append(size)
processors_array = data_dict.keys()

for i,processors in enumerate(processors_array):
	print("\\addplot[color=red!{}!blue,line width=0.4pt] coordinates {{".format(100*(i)/(len(processors_array)-1)))
	for size in size_array:
		print("({},{})".format(size,data_dict[processors][size]),end="")
	#print("\n}}node[pos=0.8](endofplotsquare){{{}}} ;".format(processors))
	print("\n}node[pos=0.8](endofplotsquare){} ;")
	print("\\addlegendentry{{{} cores}}".format(processors))
	
print("\\addplot[color=blue,line width=0.4pt] coordinates {")
for size in size_array:
	print("({},{})".format(size,tinisat_dict[size]),end="")
print("\n}node[pos=0.8](endofplotsquare){} ;")


'''
print("")
print("")
print("")
print("")
print("")

for i,processors in enumerate(processors_array):
	print("\\addplot[color=red!{}!blue,line width=0.4pt] coordinates {{".format(100*(i)/(len(processors_array)-1)))
	for size in size_array:
		print("({},{})".format(size,data_dict[processors][size]),end="")
	#print("\n}}node[pos=0.8](endofplotsquare){{{}}} ;".format(processors))
	print("\n}node[pos=0.8](endofplotsquare){} ;")
	print("\\addlegendentry{{{} cores}}".format(processors))
'''


