import os
import time

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

def timelingeling(f):
	system_wrap("cp {} ./TEMP_FILE2.cnf".format(f))
	t = time.time()
	while True:
		system_wrap("{} ./TEMP_FILE2.cnf > ./TEMP_FILE2.txt".format(os.environ.get("LINGELING_LOCATION","~/lingeling/lingeling")))
		try:
			ff = open("./TEMP_FILE2.txt","r")
			data = [d.strip() for d in ff.readlines()]
			sol = []
			for d in data:
				if d[0] =='v':
					sol = sol + [int(ii) for ii in d.split(" ")[1:]]
			assert len(sol)>0
			sol = " ".join([str(-int(d)) for d in sol]) +"\n"
			ff.close()

			ff = open("./TEMP_FILE2.cnf","r")
			line0 = ff.readline().split(" ")
			line0[3] = str(int(line0[3])+1)
			line0 = " ".join(line0)
			ff.close()

			ff = open("./TEMP_FILE2.cnf","a")
			ff.write(sol)
			ff.close()

			timeit("sed -i '1s/.*/{}/' ./TEMP_FILE2.cnf".format(line0))
			system_wrap("rm ./TEMP_FILE2.txt")
		except Exception as e:
			print(e)
			break
	system_wrap("rm ./TEMP_FILE2.cnf")
	return time.time()-t



#n_range = list(range(5,91,5))
n_range = [2,3,4,5,6]
#n_range = [1,2]

with open("timing_data.txt","w") as f:
	f.write("i,j,tinisat,lingeling,{}\n".format(",".join(["dagster{}".format(n) for n in n_range])))

for i in range(1):
	for j in range(1,46):
		p = j + i*100
		system_wrap("python ../../Benchmarks/Pentomino/pentominos.py create combination-3 15 15 {} {} problem{}".format(j,j,p))
		system_wrap("python ../../Benchmarks/Pentomino/pentominos.py dag-make complex-cubes3 problem{}.cnf problem{}.map problem{}.dag 15".format(p,p,p))
		tinisattime = 0 #timetinisat("./problem{}.cnf".format(p))
		lingelingtime = 0 #timelingeling("./problem{}.cnf".format(p))
		dagstertimes = []
		for n in n_range:
			t = time.time()
			#system_wrap("mpirun -n {} ../../dagster/dagster -m 0 -e 0 -g 2 ./problem{}.dag ./problem{}.cnf".format(n+1,p,p))
			t = time.time() - t
			dagstertimes.append(t)
			t = time.time()
			#system_wrap("mpirun -n {} ../../dagster/dagster -m 3 -e 0 -g 2 ./problem{}.dag ./problem{}.cnf".format(n*2+1,p,p))
			t = time.time() - t
			dagstertimes.append(t)
		with open("timing_data.txt","a") as f:
			f.write("{},{},{},{},{}\n".format(i,j,tinisattime,lingelingtime,",".join([str(t) for t in dagstertimes]) ))

