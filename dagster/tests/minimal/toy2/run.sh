#!/bin/bash
set -e
export OMPI_MCA_btl=self,tcp
export GLOG_v=0 # verbose glog logging at level 5 (0 is least verbose)
export GLOG_logtostderr=true # otherwise all logging goes to /tmp/appname.hostname.username.log.INFO.date.pid
echo "RUNNING TEST"
echo "        TEST part 1"
mpirun -n 2 ../../../dagster -m 0 -g 1 ./dag.txt ./cnf.txt -o output1.sols
echo "        TEST part 2"
mpirun -n 5 --oversubscribe ../../../dagster -m 1 -k 1 -g 1 ./dag.txt ./cnf.txt -o output2.sols
echo "        TEST part 3"
mpirun -n 5 --oversubscribe ../../../dagster -m 2 -k 1 -g 1 ./dag.txt ./cnf.txt -o output3.sols
echo "        TEST part 4"
python ../../check.py cnf.txt output1.sols
python ../../check.py cnf.txt output2.sols
python ../../check.py cnf.txt output3.sols

echo "RUNNING TEST"
echo "        TEST part 1"
mpirun -n 5 --oversubscribe ../../../dagster -m 0 -g 1 -h TEMP_D ./dag.txt ./cnf.txt -o output1.sols
echo "        TEST part 2"
mpirun -n 5 --oversubscribe ../../../dagster -m 1 -k 1 -g 1 -h TEMP_D ./dag.txt ./cnf.txt -o output2.sols
echo "        TEST part 3"
mpirun -n 5 --oversubscribe ../../../dagster -m 2 -k 1 -g 1 -h TEMP_D ./dag.txt ./cnf.txt -o output3.sols
echo "        TEST part 4"
python ../../check.py cnf.txt output1.sols
python ../../check.py cnf.txt output2.sols
python ../../check.py cnf.txt output3.sols
rm -rf TEMP_D
