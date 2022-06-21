#!/bin/bash
set -e
export OMPI_MCA_btl=self,tcp
export GLOG_v=0 # verbose glog logging at level 5 (0 is least verbose)
export GLOG_logtostderr=true # otherwise all logging goes to /tmp/appname.hostname.username.log.INFO.date.pid
echo "RUNNING TEST"
echo "        TEST part 1"
mpirun -n 2 --oversubscribe ../../../dagster -m 0 -i 1 -e 3 ./8-64.dag ./8-64.cnf -o output1.sols
echo "        TEST part 2"
mpirun -n 2 --oversubscribe ../../../dagster -m 0 -i 1 -e 3 ./8-64_alter.dag ./8-64.cnf -o output2.sols
echo "        TEST part 3"
mpirun -n 2 --oversubscribe ../../../dagster -m 0 -i 1 -e 3 ./8-64_alter2.dag ./8-64.cnf -o output3.sols
echo "        TEST part 4"
python ../../check.py oog.cnf output1.sols some
python ../../check.py oog.cnf output2.sols some
python ../../check.py oog.cnf output3.sols some
