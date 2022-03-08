import os
import time

def timeit(command):
	print(command)
	t = time.time()
	os.system(command)
	return time.time()-t

size = 100
overlap = 5
parts = 4
clause_size = 5
multiplier = 10

with open("timing_data.txt","w") as f:
	f.write("size,overlap,parts,clause_size,multiplier\n")
	f.write("{},{},{},{},{}\n\n".format(size,overlap,parts,clause_size,multiplier))
	f.write("multiplier,dagster,dagster+gnovelty\n")

run_forever = True
i = 0
#import pdb
#pdb.set_trace()
with open("timing_data.txt","a") as f:
	while run_forever:
		print(i)
		multiplier = 2+i%18
		timeit("python ../../Benchmarks/sequential_random_sat/generate.py {} {} {} {} {} cnf.txt dag.txt".format(size,overlap,parts,clause_size,multiplier))
		a = timeit("timeout 60s mpirun -n 8 ../../dagster/dagster -e 0 -b 0 -g 1 dag.txt cnf.txt -o output.sols")
		c = timeit("timeout 60s mpirun -n 15 ../../dagster/dagster -m 1 -k 1 -e 0 -b 0 -g 1 dag.txt cnf.txt -o output.sols")
		b = 0#timeit("timeout 60s ~/minisat -verb=0 ./cnf.txt")
		f.write("{},{},{},{}\n".format(multiplier,a,c,b))
		f.flush()
		i += 1
		#run_forever = False
