import os
import time

# please input the number of mpi available cores
cores = int(os.environ["DAGSTER_PROCESSORS"])
# timeout
timeout = "60m"

with open("runtime_log.csv","w") as f:
	f.write("N,processors,dagster,tinisat\n")
for N in range(5, 18):
	print("RUNNER: generating N={}".format(N))
	soluble0 = os.system("../../Benchmarks/costas/generate_costas_N -N {} -s > costas_{}.cnf 2> costas_{}.map".format(N,N,N))
	t2 = time.time()
	soluble = os.system("timeout {} mpirun -n {} ../../dagster/dagster -g 2 -e 1 -b 0 -p 2 ./costas_{}.dag ./costas_{}.cnf".format(timeout,2,N,N))
	t2 = time.time()-t2
	soluble0 = os.system("../../Benchmarks/costas/generate_costas_N -N {} > costas_{}.cnf 2> costas_{}.map".format(N,N,N))
	for processors in [2**i+1 for i in range(cores) if 2**i+1<=cores]:
		print("RUNNER: solving N={}, processors={}".format(N,processors))
		t = time.time()
		soluble = os.system("timeout {} mpirun -n {} ../../dagster/dagster -g 2 -e 1 -b 0 -p 2 ./costas_{}.dag ./costas_{}.cnf".format(timeout,processors,N,N))
		t = time.time() - t
		with open("runtime_log.csv","a") as f:
			f.write("{},{},{},{}\n".format(N,processors,t,t2))
			f.flush()
	os.system("rm costas_{}.cnf".format(N))
	os.system("rm costas_{}.map".format(N))
	os.system("rm costas_{}.dag".format(N))
	os.system("rm dag_out.txt")


