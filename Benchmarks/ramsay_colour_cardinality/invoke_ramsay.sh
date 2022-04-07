#!/bin/bash

make clean
make

./generate_ramsey_NM -N $1 -M $2  > ramsey_N${1}_M${2}.cnf 2>  ramsey_N${1}_M${2}.map

