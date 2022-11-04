gNOVELTY+ Simple Standalone tool:
---------------------------------

This tool uses the gNovelty+ algorithm to solve an input CNF file.
Particularly using a single thread.

To compile:
$ make

To Run
$ ./gnovelty <CNF_filename> [solution_file]

where <CNF_filename> is a mandatory valid CNF file
and [solution_file] is an optional output file to output any valid solution to.

NOTE:
	The simple standalone solver may take infinite time for SAT problems which are UNSAT.
	Or, alternative may take in incredibly long time for problems which SLS processes are less efficient at solving.
