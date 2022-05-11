import os
import time
import re
import sys
from integer_range import integer_range

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

			ff = open("./TEMP_FILE.cnf","r")
			line0 = ff.readline().split(" ")
			ff.close()

			numvars = int(line0[2])
			original_numvars = numvars

			data = re.findall(r'v ([ \d-]+)\n',data)
			assert(len(data)>=1),"returned {} solutions".format(len(data))
			data = [[int(d) for d in dd.strip().split(" ")] for dd in data]
			abs_data = integer_range()
			split_string = []
			for dd in data:
				for d in dd:
					if d==0:
						continue
					abs_d = abs(d)
					if abs_d not in abs_data:
						abs_data.add(abs_d)
						split_string.append(-d)
			#print("Solution length: {}".format(len(split_string)))
			sol = ""
			new_clauses = 0
			for i in range(0,len(split_string),10):
				max_i = min(i+10,len(split_string))
				sub_string = split_string[i:max_i]
				numvars += 1
				sol = sol + "{} {} 0\n".format(-numvars," ".join([str(a) for a in sub_string]))
				new_clauses += 1
				for s in sub_string:
					sol = sol + "{} {} 0\n".format(numvars,-s)
					new_clauses += 1
			sol = sol + " ".join([str(a) for a in range(original_numvars+1,numvars+1)]) + " 0\n"
			new_clauses += 1
			#print("Added variables: {}".format(list(range(original_numvars+1,numvars+1))))

			ff = open("./TEMP_FILE.cnf","a")
			ff.write(sol)
			ff.close()

			line0[2] = str(numvars)
			line0[3] = str(int(line0[3])+new_clauses)
			line0 = " ".join(line0)
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
