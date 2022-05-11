with open("data_output_dagster.csv","r") as f:
	data = f.readlines()

data = data[1:]
data = [d.strip().split(",") for d in data]
data = [[int(d[1]),float(d[2])] for d in data]

from collections import defaultdict
data_collection = defaultdict(list)

for d in data:
	if d[1]>1.0:
		data_collection[d[0]].append(d[1])

import statistics
print(data_collection)

for d in sorted(data_collection.keys()):
	print((d,statistics.median(data_collection[d])))
