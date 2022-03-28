# Dagster - a Distributed Hybrid SAT Solver

Dagster is a C++ implementation of a parallel decision procedure to solve Boolean SAT(isfiability) problems.

Dagster reads a SAT problem represented as a DIMACS CNF file and a decomposition of it into distinct subproblems with separate subsets of clauses for each part. The solutions to one subproblem serve as constraints for the next subproblem, constraining the order of execution. Once there is a chain of solutions for all subproblems, the original problem is solved.

The subproblems for a given CNF are specified by a Directed Acyclic Graph (DAG) file. The DAG file specifies each subproblem as a list of clause indices; it also specifies the dependencies between subproblems as a list of variables that are passed from one subproblem to the next.

Subproblems are solved in parallel by solving units, which communicate using message passing (MPI). A solving unit is composed of a number of processes that cooperate to solve a subproblem. Currently, Dagster supports different configurations for these solving units, depending on its 'mode' parameter:

- mode 0: each solving unit consists of a single process running a CDCL (conflict-driven clause learning) SAT solving procedure; 
- mode 1: each solving unit consists of a CDCL procedure assisted by one or more stochastic local search (SLS) processes;
- mode 2: each solving unit consists of a CDCL process, one or more SLS processes, and a separate CDCL procedure for clause minimization.

## Dependencies:

The following packages are required for the full compilation of Dagster:

 - [Google Logging library 'glog'](https://github.com/google/glog)
 - MPI (tested with [OpenMPI](https://www.open-mpi.org/))
 - the [CUDD library](https://davidkebo.com/cudd)
 - [Google Testing library 'googletest'](https://github.com/google/googletest) - for running tests


## Using Docker

This project can be run using Docker, as follows:

```bash
docker run -it --mount src=`pwd`,target=/home/appuser,type=bind milthorpe/async-neighbours
```

#### Building Docker Image

```bash
docker build --no-cache -t milthorpe/async-neighbours .
```


## Build

```
cd dagster && make
```

## Example Invocation

At a minimum, Dagster requires a DIMACS-formatted CNF file containing a SAT problem and a DAG file specifying its decomposition.

A general invocation of Dagster is as follows:

```
mpirun -n <Number_of_processors> ./dagster [OPTIONS] <DAG_FILE> <CNF_FILE>
```

This invocation will run Dagster using MPI on a specified number of processors, with the specified CNF file, solving it according to a decomposition given by the DAG file.

A minimal example invocation involves a CNF file such as found in `tests/minimal/unit_tests/good/c1.txt`:

```
p cnf 6 8
1 2 3 0
3 4 5 0
5 6 0
-3 -5 0
-2 3 -4 0
-4 5 -6 0
1 2 4 0
3 4 -6 0
```

This CNF file consists of a set of 8 clauses, between 6 binary variables, for instance the first clause reads "either variable 1 is true, and/or variable 2 is true and/or variable 3 is true" etc.
This CNF file is passed to Dagster with a corresponding appropriate DAG file, such as found in `./tests/minimal/unit_tests/good/d1.txt`:

```
DAG-FILE
NODES:6
GRAPH:
0->1:1,2
2->3:3,4
1->4:1,2,3
3->4:3,4,5
5->4:5,6
CLAUSES:
0:0,4,6
1:0,1,3,4,6
2:1,3
3:1,3,5,6,7
4:0-7
5:2,3,7
REPORTING:
1-6
```

This DAG file specifies that the CNF is decomposed into six subproblems (indexed 0-5), where the first subproblem is to find solutions to clauses indexed (0,4,6), and the second subproblem is to find solutions to clauses indexed (0,1,3,4,6), etc.  The subproblems are represented as a directed acyclic graph, where the edges (dependencies) determine the execution order.  Particularly, solutions of subproblem 0 are passed as constraints to invocations of subproblem 1, subproblem 1 solutions are passed to subproblem 4, and subproblem 4 are passed to subproblem 5 etc.
The solutions found for each subproblem are thus compatible with the solution of the previous subproblem.
In this DAG, solutions to subproblem 5 will implicitly be compatible with all former subproblems - consequently solving the entire problem.

This CNF and DAG can be executed by Dagster with a minimal invocation:

```
mpirun -n 2 ./dagster ./tests/minimal/unit_tests/good/d1.txt ./tests/minimal/unit_tests/good/c1.txt
```

This command instructs dagster to run using MPI with two processes, executing the CNF file c1.txt with DAG file d1.txt.
This execution will solve the problem and output the solution/s to `dag_out.txt`
In this case, there is one Master process and one CDCL process.
The solutions to the problem in `dag_out.txt` should be:

```
1 2 3 -4 -5 6 
1 2 -3 -4 5 -6 
-1 2 3 -4 -5 6 
-1 2 -3 -4 5 -6 
1 -2 3 -4 -5 6 
1 -2 -3 4 5 -6 
1 -2 -3 4 5 6 
1 -2 -3 -4 5 -6 
```

where each line identifies a possible solution to the original SAT problem, for instance, the first line reads that variables 1,2,3,6 being true, and variables 4,5 being false is one possible solution to the SAT problem.

## Dagster Options

The command-line invocation of Dagster has a number of options, the most important of which is the **mode** (`m`) option.
 - `-m 0` (default): solving units consist of a single process running a CDCL SAT solving procedure.
 - `-m 1` : solving units consist of a CDCL procedure assisted by one or more SLS (stochastic local search) processes (this number is specified by the `k` parameter).
 - `-m 2` : solving units consist of a CDCL process, one or more SLS processes (given by `k` parameter), and a separate CDCL procedure for clause minimization.

An example invocation with these options is:

```
mpirun -n 3 ./dagster -m 1 -k 1 ./tests/minimal/unit_tests/good/d1.txt ./tests/minimal/unit_tests/good/c1.txt
```

which runs Dagster with three compute processes: one Master process, and one CDCL process assisted by an SLS process.

The `k` parameter and the `m` parameter are the most important options which can be passed to Dagster. Most other options relate to performance configuration and/or experimental behaviour.

The following commands determine the behaviour of the stochastic local search:

 - `-w` determines if we are using clause weights or not. `1` means yes, `0` means no. 
 
 CHARLES TBD -- -a ghosts -r cdclfirst

The following performance-turning options affect the relationship between the CDCL process and the SLS processes:

 - `-s` is the suggestion size, is a parameter controlling the buffer size between the SLS processes and their associated CDCL instance (only relevant for modes 1&2)
 - `-d` is the decision interval in which SLS processes are updated by the CDCL process (only relevant for modes 1&2)
 - `-a` is the SLS heuristic = {`ghosts`,`normal`} affects the way in which the SLS process reports literals to the CDCL process

The following experimental options affect the behaviour of the Master node:

 - `-b` is the configuration option whether the master node will allocate multiple solving units to a given subproblem at at time = {`1`,`0`} = 'on', 'off'
 - `-g` is the master mode = {`1`,`0`} if using BDD assistance to compile solutions between subproblems
 - `-c` is the BDD compilation mode if using BDD assistance = {`cubes`,`minisat`,`paths`}


## Generating DAG files for CNFs

Inside the `dag-generator` folder there is a utility for automatically generating Dag files from a CNF file.
The dagify script will generate a DAG file suitable for execution in Dagster program from a Dimacs formatted CNF file.

the minimal invocation is:

```
python dagify.py <CNF_FILE> <DAG_FILE> <number_of_nodes>
```

where `CNF_FILE` is the name of the DIMACS-formatted CNF file input into the script
The `DAG_FILE` is the name of the DAG file to be generated
and `number_of_nodes` is the number of nodes in the resulting dag file.

The computation will take some time, and the output dag file can be passed together with the respective CNF file into the Dagster solver.

### Requirements

The Dagify script requires a python environment with the following libraries:

* networkx
* click
* tqdm
* PIL (optional for image output)

### Options

There are multiple additional command-line options:

`--icf FLOAT`
the factor by which the algorithm will prioritize minimizing incoming branches to a node

`--ocf FLOAT`
the factor by which the algorithm will prioritize minimizing outgoing branches to a node

`--continuous_reduction`
perform transitive reduction in the process of minimizing the DAG. This tends to produce DAGs which are less branching, but is slightly more expensive computationally

`--not_pass_all_data`
nodes only pass variables onwards if future nodes need them (not recommended)

`--dot_output TEXT`
a filename to output an image displaying the connection graph dag structure

`--image_output TEXT`
a filename to output an image showing the min-fill association between the variables for visualization of the problem structure

`--dag_reduction_sample_size INTEGER`
specifies how much to sample for the best places to minimize when the DAG is being compacted: unspecified means a complete search, specify to create a less perfect search at the prospect of faster DAG
generation

`--min_fill_sample_size INTEGER`
specifies how much to sample for the best variables to eliminate in the min-fill algorithm: unspecified means a complete search, specify to create a less perfect search at the prospect of faster DAG
generation


## Glossary

CNF - Conjunctive Normal Form formula in propositional logic

CDCL - Conflict-Driven Clause Learning, an enhancement of the Davis–Putnam–Logemann–Loveland, itself the Davis–Putnam algorithm with unit propagation.

gNovelty+ - A greedy dynamic local search, that uses the 'novelty+' heuristic to escape from local minima. Important related systems are probSAT and Sparrow. 

PDDL - Planning Domain Definition Language



