COSTAS benchmarks:
------------------

Benchmark generator script for Costas problem.
The costas array problem is to fill a nxn square grid of with marks such that:
A) there is exactly one mark in every column and row
B) the displacement vector between every pair of marks is unique.

It is a well knwon computation problem (https://en.wikipedia.org/wiki/Costas_array)

The script generating program will output a CNF corresponding to a nxn costas array where n is specified by commandline input, and a corresponding dag
to solve the problem using dagster

the default dag output includes two nodes, one for the first 3 columns , the second for solving the remainder of the problem.

the program will output the cnf to stdout and a map of variables to their meaning to stderr, and a dag to a file "costas_<N>.dag"

HOW TO RUN:
-----------

run the make file:
$ make

which will generate an executable: generate_costas_N
which if you run directly will output
"usage: -N [1..9]+[0..9]* (e.g. 32) (i.e. dimension of Costas array) [-F (FLAT|TREE)] [-b] [-s]"
asking you to specify a value N (which is the dimension of the square array) and options:
-F which is either FLAT or TREE, which identifies the the way it will encode the AT_MOST_ONE constraints in the coding of the costas problem
-b is no longer used,
and -s is a flag which if set will make the dag output a single node, rather than two by default.

example invocation:
$ ./generate_costas_N -N 8 > costas_8.cnf 2> costas_8.map
which will output a costas cnf and a map file, with a dag file in the current working directory, which can be loaded into dagster and run


Additional (old) scripts:
------------------------


./print_costas_solution.sh <N>

searches for costas_<N>.solution (say as might be emitted from minisat) and a costas_<N>.mapping which will then output some information about the solution and the displacements between the marks in the costas grid to the terminal

./visualize_costas.sh <solution file>

inputs a solution file (say from minisat) and spits an ASCII representation of it to the console.

./contract.sh






