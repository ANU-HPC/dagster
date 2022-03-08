#!/bin/bash
while read line
do
  IFS=' ' read -r -a sol <<< "$line"
  len=${#sol[@]}
  dim=$(echo "sqrt($len)" | bc)
  echo -n "┌"
  for x in $(seq 1 $dim)
  do
	  echo -n "─"
  done
  echo "┐"
  for x in $(seq 1 $dim)
  do
	  echo -n "│"
	  for y in $(seq 1 $dim)
	  do
		  idx=$(((x-1)*dim + y-1))
		  val=${sol[$idx]}
		  if [[ $val -gt "0" ]]
		  then
			  echo -n "█"
		  else
			  echo -n " "
		  fi
	  done
	  echo "│"
  done
  echo -n "└"
  for x in $(seq 1 $dim)
  do
	  echo -n "─"
  done
  echo "┘"
done<$1

