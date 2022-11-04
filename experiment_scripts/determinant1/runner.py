import os
import time

# please input number of processors for MPI
processors = int(os.environ.get('MPI_PROCESSORS_AVAILABLE',"2"))

with open("runtime_log.csv","w") as f:
	f.write("N,no_incremental,incremental\n")
	for N in range(3, 15):
		print("RUNNER: generating N={}".format(N))
		soluble0 = os.system("python ../../Benchmarks/determinant/determinant.py {0} 16 cnf_{0}.cnf map_{0}.map dag_{0}.dag".format(N))
		print("RUNNER: solving N={}".format(N))
		t = time.time()
		soluble = os.system("mpirun -n {1} ../../dagster/dagster -m 4 -e 1 -b 0 ./dag_{0}.dag ./cnf_{0}.cnf -o solutions_{0}.sols".format(N,processors))
		t = time.time() - t
		print("RUNNER: solving N={} part 2".format(N))
		t2 = time.time()
		soluble = os.system("mpirun -n {1} ../../dagster/dagster -m 4 -e 1 -b 0 -q 2 ./dag_{0}.dag ./cnf_{0}.cnf -o solutions_{0}.sols".format(N,processors))
		t2 = time.time() - t2
		print("done in {},{}".format(t,t2))
		f.write("{},{},{}\n".format(N,t,t2))
		f.flush()
		os.system("rm cnf_{0}.cnf".format(N))
		os.system("rm map_{0}.map".format(N))
		os.system("rm dag_{0}.dag".format(N))


