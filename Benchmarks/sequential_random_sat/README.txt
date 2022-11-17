Sequential Random Sat Generator
-------------------------------


this python script generates and outputs a CNF&DAG for a sequence of joined random sat problems

particularly, the first random sat problem will be generated on the first X many variables and C clauses, the last Y variables of which will also be a part of the next X block of variables and C clauses generated, the last Y variables of which will also be a part of the next X block of variables and C clauses generated.
A dag will be generated stringing the solution of these random sat problems in turn.

invoke by calling:
$python generate.py


which will take the following arguments:


'size', type=INT
	- the size of each block of variables that occur in each of the sequential random sat problems

'overlap', type=INT
	- how many variables of one problem should feed and overlap into the next random sat in the sequence
	
'n', type=INT
	- how many random sat problems are in the sequence

'clause_size', type=INT
	- what is the size of the clauses in each of the random sat instances eg. 3-sat is clauses of size 3, etc.
	
'multiplier', typeINT
	- the number of clauses in each of the randomly generated sat problems in the sequence

'cnf_file', type=STRING
	- the output cnf filename
	
'dag_file', type=STRING
	- the output dag filename
	
