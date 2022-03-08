import os
import time

# please input number of processors for MPI
processors = int(os.environ.get('MPI_PROCESSORS_AVAILABLE',"2"))

with open("runtime_log.csv","w") as f:
	f.write("N,M,dagster_time,dagster_return_code\n")
	M = 8
	for N in range(9, 26):
		print("RUNNER: generating N={} M={}".format(N,M))
		soluble0 = os.system("../../Benchmarks/ramsay/generate_ramsey_NM -N {0} -M {1} -Z 3 > cnf_{0}_{1}.cnf 2> map_{0}_{1}.map".format(N,M))
		print("RUNNER: solving N={} M={}".format(N,M))
		t = time.time()
		soluble = os.system("mpirun -n {2} ../../dagster/dagster -g 2 -e 0 -b 0 -p 2 ./dag_{0}_{1}.dag ./cnf_{0}_{1}.cnf -o solutions_{0}_{1}.sols".format(N,M,processors))
		t = time.time() - t
		f.write("{},{},{},{}\n".format(N,M,t,soluble))
		f.flush()
		os.system("rm cnf_{0}_{1}.cnf".format(N,M))
		os.system("rm map_{0}_{1}.map".format(N,M))
		os.system("rm dag_{0}_{1}.dag".format(N,M))


