from statistics import median

with open("large_experiment_data.csv","r") as f:
	lines = f.readlines()
	header = lines[0].strip().split(",")
	data = [d.strip().split(",") for d in lines[1:]]
data_dict = {}
for d in data:
	dd = int(d[0])
	if dd not in data_dict.keys():
		data_dict[dd] = {"size":0,"runtime_size":{}}
	data_dict[dd]['size'] = int(d[4])*1.0/1024
	data_dict[dd]['runtime_size'][int(d[1])] = int(d[2])*1.0/1024
data_dict_keys = sorted(data_dict.keys())

print("\\addplot[color=blue,line width=0.4pt] coordinates {")
for size in data_dict_keys:
	print("({},{})".format(size,data_dict[size]['size']),end="")
print("\n}node[pos=0.8](endofplotsquare){} ;")
print("\\addlegendentry{CNF}")

colours = ["blue!0!green","blue!33!green","blue!67!green","blue!100!green"]
cores = list(data_dict[data_dict_keys[0]]['runtime_size'].keys())
for i,c in enumerate(cores):
	color = colours[i]
	print("\\addplot[color={},only marks,mark=x] coordinates {{".format(color))
	for size in data_dict_keys:
		print("({},{})".format(size+i*2,data_dict[size]['runtime_size'][c]),end="")
	print("\n}node[pos=0.8](endofplotsquare){} ;")
	print("\\addlegendentry{{{} cores}}".format(c))

	
print("")
print("")
print("")
print("")
print("")


colours = ["blue!0!green","blue!33!green","blue!67!green","blue!100!green"]
cores = list(data_dict[data_dict_keys[0]]['runtime_size'].keys())
for i,c in enumerate(cores):
	color = colours[i]
	print("\\addplot[color={},only marks,mark=x] coordinates {{".format(color))
	for size in data_dict_keys:
		print("({},{})".format(data_dict[size]['size'],data_dict[size]['runtime_size'][c]),end="")
	print("\n}node[pos=0.8](endofplotsquare){} ;")
	print("\\addlegendentry{{{} cores}}".format(c))

