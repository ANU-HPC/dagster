import os
import time
import math

def timeit(s):
	t = time.time()
	print("Running: {}".format(s))
	os.system(s)
	return time.time()-t

with open("timing_data.txt","w") as f:
	f.write("N,C,B,tinisat,dagster,tinisat\n")

processors = 4

for g in range(101):
	for B in [0.05,0.06,0.07,0.08,0.09,0.1]:
		N=200 + 8*g
		C=int(math.floor(N * 4.25))
		os.system("../../Benchmarks/gensat_sat/ggen {} {} 3 0 100 7 {}".format(N,C,B))
		for germe in range(1,101):
			f = "v{}c{}g{}".format(N,C,germe) 
			t0 = timeit("../../dagster/standalone_tinisat/tinisat ./{}".format(f))
			t2 = timeit("mpirun -n {} ../../dagster/dagster -m 0 -k 1 -g 2 -d 100 -s 100 -e 0 -r cdclfirst -a ghosts ./{}.dag ./{}".format(processors,f,f))
			with open("timing_data.txt","a") as f:
				f.write("{},{},{},{},{}\n".format(N,C,B,t0,t2))
		os.system("rm v*")
