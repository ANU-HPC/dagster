#!/bin/bash
echo "CLEANING UP"
for d in ./*/ ;
  do (echo "$d" && cd "$d" && rm -f *.sols && rm -f *.true_sols && rm -f *.check);
done
