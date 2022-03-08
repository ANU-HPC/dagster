import os
import time
import subprocess
import re

def system_call(command):
	print("Running: {}".format(command))
	return subprocess.getoutput(command)

with open("timing_datum.txt","w") as f:
	f.write("copies,processors,dagster memory (KB),subproblem CNF size (KB),combined CNF size (KB)\n")

os.system("python ../../Benchmarks/sequential_random_sat/generate.py 80 5 1 7 2120 bog.cnf bog2.dag")
os.system("python ../../utilities/make_unique.py ./bog.cnf ./bog2.cnf")
for cc in range(1,5000,50):
	print("Generating and for {} copies of subproblem\n".format(cc))
	s = "python ../../utilities/union_concatenator.py cnf5.txt dag5.txt"
	for i in range(cc):
		s += " ./bog2.cnf ./bog2.dag"
	os.system(s)
	subproblem_size = os.path.getsize("bog.cnf")//1024
	bigproblem_size = os.path.getsize("cnf5.txt")//1024
	for processors in [2,3,5,9,17,33,64]:
		t1 = system_call("/usr/bin/time -f \"MONKEY %M MOO\" mpirun -n {} ../../dagster/dagster -m 0 -g 2 -e 1 ./dag5.txt ./cnf5.txt".format(processors))
		t1 = re.findall("MONKEY (\d+) MOO",t1)[0]
		with open("timing_datum.txt","a") as f:
			f.write("{},{},{},{},{}\n".format(cc,processors,t1,subproblem_size,bigproblem_size))
