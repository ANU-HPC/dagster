
from tqdm import tqdm

cnf = open("cnf.txt","r")
cnf_lines = cnf.readlines()
cnf.close()

for c in tqdm(cnf_lines):
	if 'c' in c or 'p' in c:
		continue
	c = [int(cc) for cc in c.strip().split(" ")]
	for i in range(len(c)):
		for j in range(i):
			if c[i]==c[j]:
				print("duplicate literals")
			if c[i]==-c[j]:
				print("contradicting literals")
				print(c)
				raise Exception("whoa there")
