import os
import time
import math
import statistics


with open("timing_data.txt","r") as f:
	data = [ff.strip().split(",") for ff in f.readlines()[1:]]
data = [[int(a[0])] + [float(a) for a in a[2:]] for a in data]
data_separated = {}
for d in data:
	k = (d[0],d[1])
	if k not in data_separated.keys():
		data_separated[k] = []
	data_separated[k].append(d[2:])
for k in data_separated.keys():
	median_1 = statistics.median([a[0] for a in data_separated[k]])
	median_2 = statistics.median([a[1] for a in data_separated[k]])
	data_separated[k] = (median_1,median_2)
tinisat_times = {}
for k in data_separated.keys():
	kk = k[0]
	if kk not in tinisat_times.keys():
		tinisat_times[kk] = []
	tinisat_times[kk].append(data_separated[k][0])
for k in tinisat_times.keys():
	median_1 = statistics.median(tinisat_times[k])
	tinisat_times[k] = median_1

for k in data_separated.keys():
	data_separated[k] = data_separated[k][1]/tinisat_times[k[0]]

axis_0 = sorted(list(set([k[0] for k in data_separated.keys()])))
axis_1 = sorted(list(set([k[1] for k in data_separated.keys()])))
print(","+",".join([str(a) for a in axis_0]))
for a in axis_1:
	print("{}".format(a)+",",end="")
	for aa in axis_0:
		if (aa,a) in data_separated.keys():
			print(data_separated[(aa,a)],end="")
		print(",",end="")
	print("")
