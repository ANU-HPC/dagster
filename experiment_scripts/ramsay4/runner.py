import os
import time

# please input number of processors for MPI
processors = int(os.environ.get('MPI_PROCESSORS_AVAILABLE',"2"))

with open("runtime_log.csv","w") as f:
	f.write("N,M,dagster_time,dagster_return_code\n")
	M = 8
	for N in range(9, 26):
		for Z in range(1):
			print("RUNNER: generating N={} M={}".format(N,M))
			soluble0 = os.system("../../Benchmarks/ramsay/generate_ramsey_NM -N {0} -M {1} -Z {2} > cnf_{0}_{1}.cnf 2> map_{0}_{1}.map".format(N,M,Z))
			print("RUNNER: solving N={} M={} Z={}".format(N,M,Z))
			t = time.time()
			soluble = os.system("mpirun -n {2} ../../dagster/dagster -m 4 -g 2 -e 1 -b 0 -p 2 ./dag_{0}_{1}.dag ./cnf_{0}_{1}.cnf -o solutions_{0}_{1}.sols".format(N,M,processors))
			t = time.time() - t
			t2 = time.time()
			soluble = os.system("mpirun -n {2} ../../dagster/dagster -m 4 -g 2 -e 1 -b 0 -p 2 -q 2 ./dag_{0}_{1}.dag ./cnf_{0}_{1}.cnf -o solutions_{0}_{1}.sols".format(N,M,processors))
			t2 = time.time() - t2
			f.write("{},{},{},{}\n".format(N,M,t,t2))
			f.flush()
			os.system("rm cnf_{0}_{1}.cnf".format(N,M))
			os.system("rm map_{0}_{1}.map".format(N,M))
			os.system("rm dag_{0}_{1}.dag".format(N,M))


