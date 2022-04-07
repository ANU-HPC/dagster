#!/bin/bash

g++ -ggdb -o successor_approach successor_approach.cc


#./successor_approach "bbww" > test.cnf 2> test.map
./successor_approach "0000000000000000" > test.cnf 2> test.map

# EXPERIMENTAL DATA POINT
# mpirun -n 4 ./mpi_launcher -f ./Benchmarks/david_has_a_problem/test.cnf -d 500 -s 1000 -r cdclfirst -a ghosts
# c Tinisat 0.23
# Tinisat -- c solving -f
# Tinisat -- c 4740 variables, 17706 clauses
# Tinisat -- c 375 decisions, 182 conflicts, 0 restarts
# Time Elapsed: 0.10, Decisions per second: 3899.22
# Suggestions received 364 taken 85
# Sends Performed: 1, Gets Performed: 376
# Tinisat -- c solved in 0.10 seconds

../../Originals/tinisat0.22/tinisat test.cnf | grep 0$ | grep -v Elap | tr ' ' '\n' | grep -v ^- | grep -v ^0 > test.sol

while read p ; do
    #echo $p
    cat test.map | grep "MAPPING: $p \-" 
done < test.sol
