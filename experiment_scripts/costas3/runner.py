import os
import time

# please input the number of mpi available cores
cores = int(os.environ["DAGSTER_PROCESSORS"])

with open("runtime_log.csv","w") as f:
	f.write("N,processors,dagster_time,return_code,solutions\n")
for N in range(5, 35):
	print("RUNNER: generating N={}".format(N))
	soluble0 = os.system("../../Benchmarks/costas/generate_costas_N -N {} > costas_{}.cnf 2> costas_{}.map".format(N,N,N))
	print("RUNNER: solving N={}".format(N))
	t = time.time()
	soluble = os.system("mpirun -n {} ../../dagster/dagster -g 2 -e 0 -b 0 -p 2 ./costas_{}.dag ./costas_{}.cnf -o dag_out_{}.txt".format(cores,N,N,N))
	t = time.time() - t
	solutions = 0
	with open("dag_out_{}.txt".format(N),"r") as f:
		solutions = len(f.readlines())
	with open("runtime_log.csv","a") as f:
		f.write("{},{},{},{},{}\n".format(N,cores,t,soluble,solutions))
		f.flush()
	os.system("rm costas_{}.cnf".format(N))
	os.system("rm costas_{}.map".format(N))
	os.system("rm costas_{}.dag".format(N))


