SUDOKU cnf generator
--------------------


Several scripts that generates SUDOKU cnfs from an index of minimal sudoku puzzles.
(where minimal sudokus have 17 'clues' - numbers filled in - since no 16 clue uniquely soluble sudokus exist)

These scripts were inspired by AALTO lecture notes:
https://users.aalto.fi/~tjunttil/2020-DP-AUT/notes-sat/solving.html


Commandline invocation:
------------------------

$python gen.py <FILE_NAME> <SUDOKU_INDEX>

where <FILE_NAME> is the index of sudoku puzzles - in this case sudoku17.txt
and <SUDOKU_INDEX> is the line number in the sudoku index file for the puzzle to generate [0-49150]

will output a cnf file "sudoku_cnf_<SUDOKU_INDEX>.txt"




$python dag_gen.py <FILE_NAME> <SUDOKU_INDEX>

Is the same except it will also output a DAG file for dagster "sudoku_dag_<SUDOKU_INDEX>.txt"




$python view.py

will attempt to input default output file emitted by dagster - "dag_out.txt" and print the numbers of the filled-in sudoku puzzle to stdout.
