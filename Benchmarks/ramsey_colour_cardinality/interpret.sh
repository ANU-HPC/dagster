#!/bin/bash

SOLUTIONS=$1
MAP=$2

COUNT=0
while read p ; do
    echo "SOLUTION: $COUNT"
    for f in `echo "$p" | tr ' ' '\n' | grep  -v ^\-` ; do
	#echo "$f $MAP"
	cat $MAP | grep MAPPING | grep  "\ $f\ "  
    done
    
    COUNT=$(($COUNT + 1))
done < $SOLUTIONS



