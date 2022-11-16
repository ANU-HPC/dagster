from statistics import median

with open("pentomino_timing_data2.csv","r") as f:
	lines = f.readlines()
	header = lines[0].strip().split(",")
	data = [d.strip().split(",") for d in lines[1:]]
data_dict = {}
for d in data:
	data_dict[int(d[1])] = data_dict.get(int(d[1]),[]) + [[float(dd) for dd in d[2:]]]

for k in data_dict.keys():
	data_dict[k] = map(list, zip(*(data_dict[k])))
	data_dict[k] = [median(kk) for kk in data_dict[k]]

print(",".join(header[1:]))
for k in data_dict.keys():
	print("{},{}".format(k,",".join([str(ss) for ss in data_dict[k]]) ))
	
transposed_data_dict = {}
data_dict_keys_sorted = sorted(data_dict.keys())
for i,head in enumerate(header[2:]):
	transposed_data_dict[head] = [(kk,data_dict[kk][i]) for kk in data_dict_keys_sorted]

colours = ["black","red","blue!0!green","blue!15!green","blue!30!green","blue!45!green","blue!60!green","blue!75!green","blue!90!green"]
#import pdb
#pdb.set_trace()

for i in range(len(header)-2):
	method = header[2+i]
	data = transposed_data_dict[method]
	color = colours[i]
	print("\\addplot[color={},line width=0.4pt] coordinates {{".format(color))
	for d in data:
		print("({},{})".format(*d),end="")
	print("\n}node[pos=0.8](endofplotsquare){} ;")
	print("\\addlegendentry{{{} cores}}".format(method))

