import pdb
import sys
import re
assert len(sys.argv)==3, "need to supply <IN_FILE> <OUT_FILE>"

print("processing file {} into {}".format(sys.argv[1],sys.argv[2]))

f_in = open(sys.argv[1].strip(),'r')
f_out = open(sys.argv[2].strip(),'w')

l = f_in.readline()
begin = False
while l!='':
	if re.match("p cnf \d+ \d+\n",l):
		begin = True
	if begin:
		f_out.write(l)
	l = f_in.readline()

f_in.close()
f_out.close()
print("DONE")
