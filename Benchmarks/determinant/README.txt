Maximum Determinant benchmarks:
-------------------------------

What:

#SAT enumeration of square adjacency matrices, one or more of which will have a maximum determinant for the input dimension. 

Usage:

python3 determinant.py SIZE BITS-PER-INT OUTPUT.cnf OUTPUT.map OUTPUT.dag

SIZE: Size of matrix (square)

BITS-PER-INT: Number of bits to represents entries in matrix inverse.

OUTPUT.cnf: Output clauses.

OUTPUT.map: Output mapping file, describing the semantics of propositions.

OUTPUT.dag: File describing the problem and decomposition according to OUTPUT.cnf

Example:

python3 determinant.py 3 10 x.cnf x.map x.dag

