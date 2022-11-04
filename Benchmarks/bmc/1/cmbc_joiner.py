import click
from tqdm import tqdm


# for an array of integers, return a string summarising them
# eg. [1,2,3,5,6,7,10] -> "1-3,5-6,7,10"
def contract_sequence(seq):
	seq = sorted(seq)
	s = []
	for ss in seq:
		if len(s)==0 or s[-1][0]+s[-1][1]!=ss-1:
			s.append([ss,0])
		else:
			s[-1][1] += 1
	st = []
	for ss in s:
		if ss[1]==0:
			st.append("{}".format(ss[0]))
		else:
			st.append("{}-{}".format(ss[0],ss[0]+ss[1]))
	return ",".join(st)


# Takes two input CNF files generated from CBMC ("C model bound checker") program, with variables named 'AI' that are commond between them
# generate and output CNF of thoes two joined together with links such that the AI variables from the first are constrained to be the same in the second
# and generate a DAG file such that the first is solved first and then the second
@click.command()
@click.argument('input1', type=click.File('r'))
@click.argument('input2', type=click.File('r'))
@click.argument('output_cnf', type=click.Path())
@click.argument('output_dag', type=click.File('w'))
def cbmc_join(input1, input2, output_cnf, output_dag):
	# parse headers and output new header
	print("parsing headers")
	header1 = input1.readline()
	header2 = input2.readline()
	assert len(header1)>0 and header1[0]=='p', "header on input1 not on line1"
	assert len(header2)>0 and header2[0]=='p', "header on input2 not on line1"
	header1 = header1.split(" ")
	header2 = header2.split(" ")
	assert len(header1)==4, "invalid header on input1"
	assert len(header2)==4, "invalid header on input2"
	output_cnf = open(output_cnf,"w")
	output_cnf.write("                                                                                      \n") # overwritable whitespace for later
	max_variables1 = int(header1[2])
	max_variables2 = int(header2[2])
	
	# pass through the first CNF file, picking out the AI variable lines
	print("parsing through CNF1")
	AI_lines1 = []
	cnf_lines1 = 0
	line = input1.readline()
	while line != '':
		if line[0]!='c':
			split_line = [abs(int(a)) for a in line.split(" ")]
			assert True not in [a>max_variables1 for a in split_line], "WARNING: variable count in header of CNF1 is wrong"
			assert split_line[-1] == 0, "badly formatted cnf1 line"
			output_cnf.write(line)
			cnf_lines1 += 1
		elif "AI" in line:
			AI_lines1.append(line)
		line = input1.readline()
	
	# pass through the second CNF file, picking out the AI variable lines, incrementing the variable numbers
	print("parsing through CNF2")
	AI_lines2 = []
	cnf_lines2 = 0
	line = input2.readline()
	while line != '':
		if line[0]!='c':
			split_line = [int(a) for a in line.split(" ")]
			assert True not in [abs(a)>max_variables2 for a in split_line], "WARNING variables count in header for CNF2 is wrong"
			assert split_line[-1] == 0, "badly formatted cnf2 line"
			for i in range(len(split_line)-1):
				magnitude = 1 if split_line[i]>0 else -1
				split_line[i] = magnitude*(abs(split_line[i])+max_variables1)
			new_line = " ".join([str(a) for a in split_line])
			output_cnf.write("{}\n".format(new_line))
			cnf_lines2 += 1
		elif "AI" in line:
			AI_lines2.append(line)
		line = input2.readline()
	
	# extract the AI variables into map from CNF1
	print("processing AI lines from CNF1")
	cnf_map1 = {}
	for line in AI_lines1:
		# Variable name is the second element in a split on whitespace
		split_line = line.split(" ")
		v_name = split_line[1]
		cnf_vars = []
		for i in range(2,len(split_line)):
			cnf_vars.append(int(split_line[i]))
		cnf_map1[v_name] = cnf_vars
	
	# extract the AI variables into map from CNF2
	print("processing AI lines from CNF2")
	cnf_map2 = {}
	for line in AI_lines2:
		# Variable name is the second element in a split on whitespace
		split_line = line.split(" ")
		v_name = split_line[1]
		cnf_vars = []
		for i in range(2,len(split_line)):
			cnf_vars.append(int(split_line[i]))
		cnf_map2[v_name] = cnf_vars
	
	# pair all the elements
	print("pairing and writing AI variable links")
	assert set(cnf_map1.keys())==set(cnf_map2.keys()), "mismatch in AI variables between the CNFs"
	AI_lines = 0
	for k in cnf_map1.keys():
		assert len(cnf_map1[k])==len(cnf_map2[k]), "AI variable has different number of bits between CNFs"
		for i in range(len(cnf_map1[k])):
			cnf_map2_var = abs(cnf_map2[k][i])+max_variables1
			output_cnf.write("{} {} 0\n".format(-cnf_map1[k][i],cnf_map2_var)) #A->B = -A\/B = -B->-A
			output_cnf.write("{} {} 0\n".format(cnf_map1[k][i],-cnf_map2_var)) #B->A = A\/-B = -A->-B
			AI_lines += 2
	
	# write new header
	print("writing new header")
	output_cnf.seek(0)
	output_cnf.write("p cnf {} {}".format(  max_variables1 + max_variables2, cnf_lines1 + cnf_lines2 + AI_lines  ))
	
	print(header1, header2)
	print(max_variables1, max_variables2)
	print(cnf_lines1,cnf_lines2,AI_lines)
	
	# write the DAG
	print("outputting new DAG")
	cnf_map1_variables = []
	for k in cnf_map1.keys():
		cnf_map1_variables += cnf_map1[k]
	output_dag.write("DAG-FILE\nNODES:2\nGRAPH:\n0->1:{}\nCLAUSES:\n0:0-{}\n1:{}-{}\nREPORTING:\n{}-{}\n".format(contract_sequence(cnf_map1_variables), cnf_lines1-1,cnf_lines1,cnf_lines1+cnf_lines2+AI_lines-1,max_variables1+1,max_variables1+max_variables2))
	
	output_cnf.close()
	print("DONE")


if __name__ == '__main__':
	cbmc_join()


