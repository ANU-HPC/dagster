from collections import defaultdict

with open("cnf_concatenator_data.csv","r") as f:
	lines = f.readlines()
	header = lines[0].strip().split(",")
	data = [d.strip().split(",") for d in lines[1:]]
tinisat_dict = {}
data_dict = defaultdict(dict)
copies_array = []
for d in data:
	processors = int(d[1])
	copies = int(d[0])
	data_dict[processors][copies] = float(d[2])
	tinisat_dict[copies] = float(d[3])
	if copies not in copies_array:
		copies_array.append(copies)
processors_array = data_dict.keys()

for i,processors in enumerate(processors_array):
	print("\\addplot[color=red!{}!blue,line width=0.4pt] coordinates {{".format(100*(i)/(len(processors_array)-1)))
	for copies in copies_array:
		print("({},{})".format(copies,data_dict[processors][copies]*1.0/tinisat_dict[copies]),end="")
	#print("\n}}node[pos=0.8](endofplotsquare){{{}}} ;".format(processors))
	print("\n}node[pos=0.8](endofplotsquare){} ;")
	print("\\addlegendentry{{{} cores}}".format(processors))
	
#print("\\addplot[color=blue,line width=0.4pt] coordinates {")
#for copies in copies_array:
#	print("({},{})".format(copies,tinisat_dict[copies]),end="")
#print("\n}node[pos=0.8](endofplotsquare){} ;")

print("\\addplot[color=black,line width=0.4pt] coordinates {")
for copies in copies_array:
	print("({},{})".format(copies,3.0/copies),end="")
print("\n}node[pos=0.8](endofplotsquare){} ;")


print("")
print("")
print("")
print("")
print("")

for i,processors in enumerate(processors_array):
	print("\\addplot[color=red!{}!blue,line width=0.4pt] coordinates {{".format(100*(i)/(len(processors_array)-1)))
	for copies in copies_array:
		print("({},{})".format(copies,data_dict[processors][copies]),end="")
	#print("\n}}node[pos=0.8](endofplotsquare){{{}}} ;".format(processors))
	print("\n}node[pos=0.8](endofplotsquare){} ;")
	print("\\addlegendentry{{{} cores}}".format(processors))



