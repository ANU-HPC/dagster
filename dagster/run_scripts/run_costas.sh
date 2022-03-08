#!/bin/bash

if [ "$#" -ne 1 ]
then
  echo "Usage: $0 N" >&2
  exit 1
fi
N=$1

# work around a known issue in OpenMPI https://github.com/open-mpi/ompi/issues/6518
export OMPI_MCA_btl=self,tcp

if [ ! -f  ../../Benchmarks/costas/costas_${N}.dag ] ||  [ ! -f  ../../Benchmarks/costas/costas_${N}.cnf ] ; then
    (>&2 echo "Requested costas problem file does not seem to exist, we shall generate it now...")
    MYDIR=`pwd`
    cd ../../Benchmarks/costas/
    if [ ! -f generate_costas_N ] ; then
	make
    fi

    ./generate_costas_N -N ${N} > costas_${N}.cnf 2> costas_${N}.mapping
    
    cd ${MYDIR}

    
    if [ ! -f  ../../Benchmarks/costas/costas_${N}.dag ]  &&  [ ! -f  ../../Benchmarks/costas/costas_${N}.cnf ] ; then
	(>&2 echo "The input problem file for the requested costas problem could not be generated. Exiting.  ")
	exit 1
    fi
    
    (>&2 echo "The input file should be generated now, continuing. ")
fi

export OMPI_MCA_btl=self,tcp
export GLOG_v=1 # verbose glog logging at level 5 (0 is least verbose)
export GLOG_logtostderr=true # otherwise all logging goes to /tmp/appname.hostname.username.log.INFO.date.pid
#mpirun -n 16 ./dagster -m 1 -k 1 ../Benchmarks/costas/costas_${N}.dag ../Benchmarks/costas/costas_${N}.cnf 
echo "mpirun -n 16 ../dagster ../../Benchmarks/costas/costas_${N}.dag ../../Benchmarks/costas/costas_${N}.cnf"
mpirun -n 16 ../dagster ../../Benchmarks/costas/costas_${N}.dag ../../Benchmarks/costas/costas_${N}.cnf 
echo "Number of solutions to Costas order $N:"
cat dag_out.txt | wc -l
