#!/bin/bash
# This script accepts a value N 
# and prints the mapping from a solution
#   (in terms of a space-separated list of T/F variables produced as output from tinisat)
#   in the file costas_$N.solution
# to a Costas array constrained as specified in the file costas_$N.mapping
# All constraints matching positive literals are printed.
# For example, given N=3 and a file costas_3.solution containing the set of positive literals:
# 3 4 8 13 14
# this script will print:
# c0 h2
# c1 h0
# c2 h1
# c0 c1 d-2
# c1 c2 d1

if [ "$#" -ne 1 ]
then
  echo "Usage: $0 N" >&2
  exit 1
fi
N=$1
for i in `cat costas_$N.solution`
do
  sed -n "s/MAPPING: ${i} -> //p" costas_$N.mapping
done
