Ramsay Benchmark
________________


Program to generate a CNF&MAP&DAG file for a give ramsay problem.
the specific ramsay colouring problem comes from various papers, particularly from Kowalski:

"401 and Beyond: Improved Bounds and Algorithms for the Ramsey Algebra Search" by Jeremy F. Alm (http://arxiv.org/abs/1609.01817v2)
"A reduced upper bound for an edge-coloring problem from relation algebra" by Jeremy F. Alm and David A. Andrews (Algebra Univers. (2019) 80:19, DOI: https://doi.org/10.1007/s00012-019-0592-6)
"Representability of Ramsey Relation Algebras" by Tomasz Kowalski (Algebra Univers. 74 (2015) 265–275DOI 10.1007/s00012-015-0353-0)

The central requirement being that a ramsay problem in this context is with a series of N graph vertices and M colours, to colour each arc in the graph with one of the M colours such that:
A) no monochromatic triangles exist
B) every vertex is a part of every possible non-monochromatic triangle

for instance if there are two colours (Red and Green), then ever vertex must be one part of at least one Red-Red-Green triange and at least one Green-Green-Red triangle, thought there is no specification, but never part of any Red-Red-Red triangle or a Green-Green-Green triangle.


Note: 
The script employs symmetry breaking constraints documented in:
"Breaking Symmetries in Graph Representation" by Michael Codish, Alice Miller, Patrick Prosser and Peter J. Stuckey
(as does some other CNF generators in this benchmark collection)


USAGE:
------

to compile the generator script just use the makefile:
$make

which will compile the ./generate_ramsey_NM executable which you invoke as:

$./generate_ramsey_NM -N [NUMBER OF VERTICES] -M [NUMBER OF COLOURS] > [CNF_OUTPUT_FILE] 2> [MAP_OUTPUT_FILE]

this call will also generate a file "dag_[VERTICES]_[COLOURS].dag" which will hold a decomposition for dagster to use

The dag will hold 3 nodes, one that attempts to colour all edges connected to the first vertex of the problem subject to all symmetry breaking constraints, and the second will subsequently colour
[Z] many edges from the second vertex (simply to split the problem up more) before passing solving to the third and final node, of colouring all edges in the problem subject to all constraints.

please note that [Z] can also be specified on commandline like so:

$./generate_ramsey_NM -N [NUMBER OF VERTICES] -M [NUMBER OF COLOURS] -Z [0->N-2] > [CNF_OUTPUT_FILE] 2> [MAP_OUTPUT_FILE]

where Z effectively controls the degree of further decomposition


View Ramsay Utility
-------------------

a small python script which will take the output file of literal evaluations from dagster, and together with the map file, will spit out a SVG file visualising the colouring of the edges for <= 8 colours in the graph.

