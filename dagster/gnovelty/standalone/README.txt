gNOVELTY+ Standalone tool:
--------------------------

This tool uses the gNovelty+ algorithm to solve an input CNF file.
Particularly using multiple threads

To compile:
$ make

To Run
$ ./gnovelty <CNF_filename> <num_threads>

where <CNF_filename> is a mandatory valid CNF file
and <num_threads> is the number of threads that will be instantiated and run the process simultaneously
The tool will spit solutions out to stdout.

NOTE:
	The solver may take infinite time for SAT problems which are UNSAT.
	Or, alternative may take in incredibly long time for problems which SLS processes are less efficient at solving.
	If the problem is quickly satisfied, the output to stdout may be garbled, as multiple process race to report their solution to the problem.
