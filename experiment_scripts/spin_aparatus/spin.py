import os
import time
import re
import sys

def system_wrap(c):
	print(c)
	os.system(c)

def timeit(a):
	t = time.time()
	system_wrap(a)
	return time.time()-t


def timemodelcount(f,string):
	system_wrap("cp {} ./TEMP_FILE.cnf".format(f))
	t = time.time()
	while True:
		system_wrap("{} > TEMP_FILE.txt 2> TEMP_FILE2.txt".format(string.format("./TEMP_FILE.cnf")))
		try:
			ff = open("./TEMP_FILE.txt","r")
			data = "\n".join(ff.readlines())
			ff.close()
			ff = open("./TEMP_FILE2.txt","r")
			data += "\n".join(ff.readlines())
			ff.close()
			
			data = re.findall(r'v ([ \d-]+)\n',data)
			assert(len(data)==1),"returned {} solutions".format(len(data))
			sol = " ".join([str(-int(d)) for d in data[0].strip().split(" ")]) + '\n'
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
	#system_wrap("rm ./TEMP_FILE.cnf")
	#system_wrap("rm ./TEMP_FILE.txt")
	#system_wrap("rm ./TEMP_FILE2.txt")
	return time.time()-t

assert len(sys.argv)==3

#import pdb
#pdb.set_trace()
#timemodelcount("./cnf.txt","mpirun -np 1 ~/Desktop/dsyrup2/Manager/bin/manager {} ~/Desktop/dsyrup2/Config/MconcurrentSAT.xml")
timemodelcount(sys.argv[1],sys.argv[2])
