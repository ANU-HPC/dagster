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

run_strings = ['python ~/summer1819/Benchmarks/Pentomino/pentominos.py dag-make complex-cubes5 {0}.cnf {0}.map {0}_vert2.dag 15']


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
		#except CalledProcessError as e:
		#	print(e)
		#	try_again = False
		if counts > 3:
			return -1
		#os.system(s)
	return time.time()-t

from os.path import exists

#import pdb
#pdb.set_trace()

existant_files = []
if exists("data_output2.csv"):
	with open("data_output2.csv","r") as f:
		fread = f.readlines()
	fread = [f.strip() for f in fread]
	fread = fread[1:]
	existant_files = [f.split(",")[0] for f in fread]
else:
	with open("data_output2.csv",'w') as f:
		f.write("string,size,runtimes\n")


data = {}
for s in run_strings:
	#print(s)
	data[s] = {}
	for size in sorted(problem_files.keys()):
		#if size>13:
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
			with open("data_output2.csv","a") as ff:
				ff.write("{},{},{}\n".format(f,size,return_runtime))
			print(size,return_runtime)
		#median_time = [runtime(s.format(f)) for f in problem_files[size]]
		data[s][size] = median_time
		#print(size,median_time)

print(data)

