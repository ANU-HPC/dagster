from os import listdir, getcwd
from os.path import isfile, join
#import statistics
import re
import time
import os
import pdb

cwd = getcwd()
onlyfiles = [f for f in listdir(cwd) if isfile(join(cwd, f))]


problem_files = {}
for f in onlyfiles:
	if len(re.findall("problem\d+.cnf$",f))>0:
		size = int(re.findall("problem(\d+)",f)[0])%100
		problem_files[size] = problem_files.get(size,[]) + [f.split(".")[0]]

print(problem_files)

run_strings = ["timeout 3000s mpirun -np 1 ~/Desktop/dsyrup2/Manager/bin/manager {}.cnf ~/Desktop/dsyrup2/Config/MampharosSAT.xml",
"timeout 3000s mpirun -np 1 ~/Desktop/dsyrup2/Manager/bin/manager {}.cnf ~/Desktop/dsyrup2/Config/MconcurrentSAT.xml",
"timeout 3000s mpirun -np 5 ~/Downloads/dmc-bin/DeMoniaCObjectsFinal/DeMoniaC -dmc {}.cnf"]

run_strings = ['timeout 3000s python spin.py {}.cnf "mpirun -np 1 ~/Desktop/dsyrup2/Manager/bin/manager {{}} ~/Desktop/dsyrup2/Config/MampharosSAT.xml"',
'timeout 3000s python spin.py {}.cnf "mpirun -np 1 ~/Desktop/dsyrup2/Manager/bin/manager {{}} ~/Desktop/dsyrup2/Config/MconcurrentSAT.xml"']


#run_strings = ['python3 spin4.py {}.cnf "mpirun ~/dsyrup2/Manager/bin/manager {{}} ~/dsyrup2/Config/MconcurrentSAT.xml"']

run_strings = ['python3 spin4.py {}.cnf "~/Paracooba-v_satcomp2021-1/build/parac --worker-count=17 --cadical-cubes {{}}"']


#run_strings = ['mpirun -n 17 ~/dagster/dagster/dagster -m 3 -e 1 -g 2 -b 0 -t 2 -j 5000 {0}_vert2.dag {0}.cnf']
run_strings = ['mpirun -n 17 ~/dagster/dagster/dagster -m 3 -e 1 -g 2 -b 0 -j 5000 {0}_vert2.dag {0}.cnf']
run_strings = ['mpirun -n 17 ~/dagster/dagster/dagster -m 3 -e 1 -g 2 -b 0 -j 5000 -h TEMP_DIR {0}_vert2.dag {0}.cnf']
#run_strings = ['timeout 1000 python3 spin4.py {}.cnf "/home/users/u4517355/painless-master/painless-mcomsps -c=17 {{}}"']
run_strings = ["timeout 3000s mpirun -np 17 ~/dmc-bin/DeMoniaCObjectsFinal/DeMoniaC -dmc {}.cnf"]

run_strings = ['''python3 spin4.py {}.cnf "bash -c '~/glucose-syrup-main/simp/glucose_static {{}} ./zog.txt; echo -n \\"v \\"; cat zog.txt'" ''']

from subprocess import STDOUT, check_output
from subprocess import TimeoutExpired, CalledProcessError
import shlex

def runtime(s):
	try_again = True
	counts = 0
	while try_again:
		t = time.time()
		print("running: {}".format(s))
		try:
			#output = check_output(shlex.split(s), stderr=STDOUT, timeout=1000)
			os.system(s)
			#print(output)
			try_again = False
		except TimeoutExpired as e:
			counts += 1
			pass
		except CalledProcessError as e:
			print(e)
			try_again = False
		if counts > 3:
			return -1
		#os.system(s)
	return time.time()-t

from os.path import exists

#import pdb
#pdb.set_trace()

existant_files = []
if exists("data_output_dagster.csv"):
	with open("data_output_dagster.csv","r") as f:
		fread = f.readlines()
	fread = [f.strip() for f in fread]
	fread = fread[1:]
	existant_files = [f.split(",")[0] for f in fread]
else:
	with open("data_output_dagster.csv",'w') as f:
		f.write("string,size,runtimes\n")


data = {}
for s in run_strings:
	#print(s)
	data[s] = {}
	for size in sorted(problem_files.keys()):
		#if size<12:
		#	continue
		#median_time = statistics.median([runtime(s.format(f)) for f in problem_files[size]])
		median_time = []
		for f in problem_files[size]:
			if f in existant_files:
				continue
			return_runtime = runtime(s.format(f))
			if return_runtime<0:
				continue
			median_time.append(return_runtime)
			with open("data_output_dagster.csv","a") as ff:
				ff.write("{},{},{}\n".format(f,size,return_runtime))
			print(size,return_runtime)
		#median_time = [runtime(s.format(f)) for f in problem_files[size]]
		data[s][size] = median_time
		#print(size,median_time)

print(data)

