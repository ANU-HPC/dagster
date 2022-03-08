import os
import time

# please input number of processors for MPI
processors = 60
# please input log_level
log_level = 3


def timeit(a):
        t = time.time()
        print(a)
        os.system(a)
        return time.time()-t

def system_wrap(c):
        print(c)
        os.system(c)


def timetinisat(f):
        system_wrap("cp {} ./TEMP_FILE.cnf".format(f))
        t = time.time()
        while True:
                system_wrap("{} ./TEMP_FILE.cnf ./TEMP_FILE.txt".format(os.environ.get('TINISAT_LOCATION',"~/summer1819/Originals/tinisat0.22/tinisat")))
                try:
                        ff = open("./TEMP_FILE.txt","r")
                        data = ff.readlines()
                        assert(len(data)==2)
                        sol = " ".join([str(-int(d)) for d in data[1].strip().split(" ")])
                        ff.close()

                        ff = open("./TEMP_FILE.cnf","r")
                        line0 = ff.readline().split(" ")
                        line0[3] = str(int(line0[3])+1)
                        line0 = " ".join(line0)
                        ff.close()

                        ff = open("./TEMP_FILE.cnf","a")
                        ff.write(sol)
                        ff.close()

                        timeit("sed -i '1s/.*/{}/' ./TEMP_FILE.cnf".format(line0))
                        system_wrap("rm ./TEMP_FILE.txt")
                except Exception as e:
                        print(e)
                        break
        system_wrap("rm ./TEMP_FILE.cnf")
        return time.time()-t

os.system("export GLOG_v={}".format(log_level))
os.system("export GLOG_logtostderr=true")

with open("runtime_log.csv","w") as f:
	f.write("N,M,processors,dagster,tinisat,code1,code2\n")
	M = 8
	#for M in range(8,9):
	#for processors in [2,3,5,9,17,33,64]:
	for processors in [2]:
		#for N in range(5, 16):
		for N in [6]:
			print("RUNNER: generating N={} M={}".format(N,M))
			soluble0 = os.system("../../Benchmarks/ramsay/generate_ramsey_NM -N {0} -M {1} > cnf_{0}_{1}.cnf 2> map_{0}_{1}.map".format(N,M))
			print("RUNNER: solving N={} M={}".format(N,M))
			t = time.time()
			soluble = os.system("mpirun -n {2} ../../dagster/dagster -g 2 -e 0 -b 0 -p 2 ./dag_{0}_{1}.dag ./cnf_{0}_{1}.cnf -o solutions_{0}_{1}.sols".format(N,M,processors))
			t = time.time() - t
			t2 = timetinisat("./cnf_{}_{}.cnf".format(N,M))
			f.write("{},{},{},{},{},{},{}\n".format(N,M,processors,t,t2,soluble,soluble0))
			f.flush()
			os.system("rm cnf_{0}_{1}.cnf".format(N,M))
			os.system("rm map_{0}_{1}.map".format(N,M))
			os.system("rm dag_{0}_{1}.dag".format(N,M))


