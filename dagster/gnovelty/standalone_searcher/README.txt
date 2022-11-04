gNOVELTY+ Standalone Searcher tool:
-----------------------------------

This tool uses the gNovelty+ algorithm to solve an input CNF file.
It tries to solve for as many different solutions to the CNF file as it can in a given number of seconds.
Using a single thread search.

To compile:
$ make

To Run
$ ./gnovelty <filename> <seconds> <search_mode>

where <CNF_filename> is a mandatory valid CNF file
and <seconds> is the number of seconds the search will run for
<search_mode> is 0,1,2:
 - mode 0: will report all variable values to a solution, and search-for-and-report all soluitons (consisting of all variables) not already reported (note: this can cause quite a blow-out)
 - mode 1: will report all variables nessisary to satisfy the CNF, and not more, and also to be entirely logically distinct from previously reported solutions
 - mode 2: will report all variables nessisary to satisfy the CNF, and not more, but will NOT report on variables to make reported solutions logically distinct from previous solutions (ie. there will be compatable overlapping solutions from previous solutions)

selection of modes 0,1,2 depend on how much detail the solutions should report, and if trimming of solution variables should leave them logically disjoint, or trim further to allow overlap between solution spaces.

