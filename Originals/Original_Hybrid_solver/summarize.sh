#!/bin/bash

echo "Time"
for i in {1..100}
do
  for r in {1..25}
  do
    cat run-${r}-mpi-16/uf250-0${i}-output | grep solved | awk '{print $6}' | tr '\n' '\0'
  done
  echo
done

echo "Decisions per second"
for i in {1..100}
do
  for r in {1..25}
  do
    cat run-${r}-mpi-16/uf250-0${i}-output | grep Decisions | awk '{print $7}' | tr '\n' '\0'
  done
  echo
done

echo "Total decisions"
for i in {1..100}
do
  for r in {1..25}
  do
    cat run-${r}-mpi-16/uf250-0${i}-output | grep decisions | awk '{print $4}' | tr '\n' '\0'
  done
  echo
done
