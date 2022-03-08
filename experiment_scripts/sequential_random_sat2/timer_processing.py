
from statistics import median
import numpy as np

with open("timing_data.txt","r") as f:
	data = f.readlines()
data = data[4:]
data = [d.strip().split(',') for d in data]
data = [[int(d[0]),float(d[1]),float(d[2])] for d in data]

data_dict = {}
for d in data:
	data_dict[d[0]] = data_dict.get(d[0],[])
	data_dict[d[0]].append([d[1],d[2]])
for k in sorted(data_dict.keys()):
	print(k,len(data_dict[k]))
	data_dict[k] = list(zip(*data_dict[k]))
	data_dict[k] = [np.quantile(data_dict[k][0],0.25),
					np.quantile(data_dict[k][0],0.5),
					np.quantile(data_dict[k][0],0.75),
					np.quantile(data_dict[k][1],0.25),
					np.quantile(data_dict[k][1],0.5),
					np.quantile(data_dict[k][1],0.75)]

with open("processed_timing_data.csv","w") as f:
	keys = sorted(data_dict.keys())
	for k in keys:
		f.write("{},".format(k))
		f.write("{}\n".format(",".join([str(a) for a in data_dict[k]])))
#		f.write("{},{}\n".format(*data_dict[k]))

