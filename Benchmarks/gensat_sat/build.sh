#!/bin/bash

BINARY=ggen

if [ ! "$#" -ne 1 ] ; then
    BINARY=$1
fi

(>&2 echo "Compiling $BINARY, a program that generates random SAT formulae in DIMACS CNF format.")

gcc -o $BINARY -Wall -g ggensata2.c -lm


