import sys
import re
assert len(sys.argv)==2

with open(sys.argv[1]) as f:
	lines = f.readlines()

lines = [l.strip() for l in lines]

print("Checking CNF {}".format(sys.argv[1]))
for l in lines:
	if not re.match("^p cnf \d+ \d+\s*$|^[ \d-]+ 0$",l):
		print("bad CNF")
		print(l)
		sys.exit(-1)
