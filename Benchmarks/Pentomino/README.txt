Pentominos Benchmark
____________________



Pentominos benchmark was derived by considering that tiling problems can/may be naturally broken into regions and thus may be ameniable for computation using dagster
particularly the inspriation for pentomino tiling problems was inspired from a puzzle solving video on a youtube channel 'Cracking The Cryptic'
specifically found at:  youtube.com/watch?v=S2aN-s3hG6Y

Specifically a pentomino is a 5 connected square block (eg. similar to tetris game, of placing tetronimos), and a grid is generated that needs to be filled by pentominos, the grid has specifically marked boundaries between cells that are 'walls' and cannot have a pentomino placed across them, the challenge then is to place pentominos in the grid, such that:
a) no pentomino crosses a wall
b) no two pentominos of the same shape (counting reflections and rotations) touch each other

these two constraints consist of a tiling problem.

The benchmark utility program - Benchmarks/Pentomino/pentominos.py - contains a number of execution modes that generate, decompose, solve and display pentomino problems.

simply executing the pentomino utility program will display some of these commands:

$ python pentominos.py 
> ----------------------------
> : PENTOMINO PUZZLE UTILITY :
> ----------------------------
> Inspired by 'Airlocks' by Gliperal
> As solved by Mark Goodliffe - Cracking The Cryptic
> see: youtube.com/watch?v=S2aN-s3hG6Y
> 
> Usage: pentominos.py [OPTIONS] COMMAND [ARGS]...
> 
> Options:
>   --help  Show this message and exit.
> 
> Commands:
>   check-multiply-soluble
>   create
>   dag-make
>   display
>   display-tikz
>   generate
>   solve
>   view-solution


these commands may have subcommands, for instance






