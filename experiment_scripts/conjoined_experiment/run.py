import os
import time

def timeit(s):
	t = time.time()
	print("Running: {}".format(s))
	os.system(s)
	return time.time()-t

with open("timing_data.txt","w") as f:
	f.write("copies,processors,dagster,tinisat\n")

timeit("../../Benchmarks/sgen/sgen1 -n 55 -sat > bog.cnf")
timeit("python ../../utilities/make_unique.py ./bog.cnf ./bog2.cnf")
timeit("python ../../utilities/dumb_dag_generator.py ./bog2.cnf ./bog2.dag")
for cc in range(5,5000,30):
	print("Generating and for {} copies of subproblem\n".format(cc))
	s = "python ../../utilities/union_concatenator.py cnf5.txt dag5.txt"
	for i in range(cc):
		s += " ./bog2.cnf ./bog2.dag"
	os.system(s)
	os.system("python ../../utilities/dumb_dag_generator.py ./cnf5.txt dag5_dumb.txt")
	t2 = timeit("mpirun -n 2 ../../dagster/dagster -m 0 -g 2 -e 1 ./dag5_dumb.txt ./cnf5.txt")
	for processors in [2,3,5,9,17,33,64]:
		t1 = timeit("mpirun -n {} ../../dagster/dagster -m 0 -g 2 -e 1 ./dag5.txt ./cnf5.txt".format(processors))
		with open("timing_data.txt","a") as f:
			f.write("{},{},{},{}\n".format(cc,processors,t1,t2))
