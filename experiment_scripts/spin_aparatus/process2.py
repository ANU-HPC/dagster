from os import listdir, getcwd
from os.path import isfile, join
#import statistics
import re
import time
import os

cwd = getcwd()
onlyfiles = [f for f in listdir(cwd) if isfile(join(cwd, f))]


problem_files = {}
for f in onlyfiles:
	if len(re.findall("^problem\d+.cnf$",f))>0:
		size = int(re.findall("problem(\d+)",f)[0])%100
		problem_files[size] = problem_files.get(size,[]) + [f]

print(problem_files)

run_strings = ["timeout 3000s mpirun -np 1 ~/Desktop/dsyrup2/Manager/bin/manager {} ~/Desktop/dsyrup2/Config/MampharosSAT.xml",
"timeout 3000s mpirun -np 1 ~/Desktop/dsyrup2/Manager/bin/manager {} ~/Desktop/dsyrup2/Config/MconcurrentSAT.xml",
"timeout 3000s mpirun -np 5 ~/Downloads/dmc-bin/DeMoniaCObjectsFinal/DeMoniaC -dmc {}"]

run_strings = ['timeout 3000s python spin.py {} "mpirun -np 1 ~/Desktop/dsyrup2/Manager/bin/manager {{}} ~/Desktop/dsyrup2/Config/MampharosSAT.xml"',
'timeout 3000s python spin.py {} "mpirun -np 1 ~/Desktop/dsyrup2/Manager/bin/manager {{}} ~/Desktop/dsyrup2/Config/MconcurrentSAT.xml"']


run_strings = ['timeout 3000s python spin.py {} "mpirun -np 1 valgrind --tool=callgrind ~/Desktop/dsyrup2/Manager/bin/manager {{}} ~/Desktop/dsyrup2/Config/MconcurrentSAT.xml"']

#run_strings = ['echo {0}; date "+%T"; time /bin/bash -c "./spin2.sh {0} > /dev/null 2> /dev/null"']
run_strings = ['echo {0}; date "+%T"; ./spin3.sh {0}']


def runtime(s):
	t = time.time()
	print("running: {}".format(s))
	os.system(s)
	return time.time()-t

ff = open("spinner.sh","w")
for s in run_strings:
	for size in sorted(problem_files.keys()):
		if size!=3:
			continue
		for f in sorted(problem_files[size]):
			ff.write("{}\n".format(s.format(f)))
ff.close()
