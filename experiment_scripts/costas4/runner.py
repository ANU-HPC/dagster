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
	soluble = os.system("mpirun -n {} ../../dagster/dagster -m 4 -e 1 -p 2 ./costas_{}.dag ./costas_{}.cnf -o dag_out_{}.txt".format(cores,N,N,N))
	t = time.time() - t
	t2 = time.time()
	soluble = os.system("mpirun -n {} ../../dagster/dagster -m 4 -e 1 -p 2 -q 2 ./costas_{}.dag ./costas_{}.cnf -o dag_out_{}.txt".format(cores,N,N,N))
	t2 = time.time() - t2
	with open("runtime_log.csv","a") as f:
		f.write("{},{},{},{}\n".format(N,cores,t,t2))
		f.flush()
	os.system("rm costas_{}.cnf".format(N))
	os.system("rm costas_{}.map".format(N))
	os.system("rm costas_{}.dag".format(N))


