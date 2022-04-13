#!/bin/bash
for r in {1..25}
do
	echo "Run $r"
	for n in 2 4 8 16
	do
		echo "Testing $n Processes"
		mkdir run-$r-mpi-$n
		for i in {1..100}
		do
			echo "uf250-0$i"
			timeout 60 mpirun -n $n ./mpi_launcher Benchmarks/uf250-1065/uf250-0$i.cnf >> run-$r-mpi-$n/uf250-0$i-output
		done
	done
done

#echo "UNSAT"
#echo "Testing Tinisat"
#mkdir unsat-run-1-mpi-1
#for i in {1..25}
#do
	#echo "uuf250-0$i"
	#timeout 300 mpirun -n 1 ./mpi_launcher Benchmarks/uuf250-1065/uuf250-0$i.cnf >> unsat-run-1-mpi-1/uuf250-0$i-output
#done

#for r in {1..25}
#do
	#echo "Run $r"
	#for n in 2 4 8 16
	#do
		#echo "Testing $n Processes"
		#mkdir unsat-run-$r-mpi-$n
		#for i in {1..100}
		#do
			#echo "uuf250-0$i"
			#timeout 300 mpirun -n $n ./mpi_launcher Benchmarks/uuf250-1065/uuf250-0$i.cnf >> unsat-run-$r-mpi-$n/uuf250-0$i-output
		#done
	#done
#done
