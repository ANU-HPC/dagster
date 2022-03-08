#!/bin/bash
export OMPI_MCA_btl=self,tcp
export GLOG_v=0 # verbose glog logging at level 5 (0 is least verbose)
export GLOG_logtostderr=true # otherwise all logging goes to /tmp/appname.hostname.username.log.INFO.date.pid

for x in {5..8} ; do
    rm dag_out.txt 
    time mpirun -n 16  ../dagster -m 1 -k 5 -d 100 -s 30 -r cdclfirst -a ghosts ../../Benchmarks/costas/debugging/costas_$x.dag ../../Benchmarks/costas/debugging/costas_$x.cnf
    NUM_SOLS=`wc -l ../dag_out.txt`
    if [ $NUM_SOLS -lt 1 ] ; then
	echo "ERROR: DID NOT FIND ANY SOLUTIONS AT SIZE: $x"
    else
	echo "REPORTING: FOUND: $NUM_SOLS SOLUTIONS AT SIZE: $x"
    fi
done
