#!/bin/bash
set -e
export OMPI_MCA_btl=self,tcp
export GLOG_v=0 # verbose glog logging at level 5 (0 is least verbose)
export GLOG_logtostderr=true # otherwise all logging goes to /tmp/appname.hostname.username.log.INFO.date.pid
echo "RUNNING TEST"
echo "        TEST part 1"
mpirun -n 6 --oversubscribe ../../../dagster -m 0 -g 1 ./dag.txt ./cnf.txt -o output1.sols
echo "        TEST part 2"
mpirun -n 5 --oversubscribe ../../../dagster -m 1 -k 1 -g 1 ./dag.txt ./cnf.txt -o output2.sols
echo "        TEST part 3"
mpirun -n 5 --oversubscribe ../../../dagster -m 2 -k 1 -g 1 ./dag.txt ./cnf.txt -o output3.sols
echo "        TEST part 4"
mpirun -n 5 --oversubscribe ../../../dagster -m 3 -g 1 ./dag.txt ./cnf.txt -o output4.sols
echo "        TEST part 5"
mpirun -n 5 --oversubscribe ../../../dagster -m 4 -k 1 -g 1 -q 0 ./dag.txt ./cnf.txt -o output5.sols
echo "        TEST part 6"
mpirun -n 5 --oversubscribe ../../../dagster -m 4 -k 1 -g 1 -q 1 ./dag.txt ./cnf.txt -o output6.sols
echo "        TEST part 7"
mpirun -n 5 --oversubscribe ../../../dagster -m 4 -k 1 -g 1 -q 2 ./dag.txt ./cnf.txt -o output7.sols
echo "        TEST part 8"
python ../../check.py cnf.txt output1.sols
python ../../check.py cnf.txt output2.sols
python ../../check.py cnf.txt output3.sols
python ../../check.py cnf.txt output4.sols
python ../../check.py cnf.txt output5.sols
python ../../check.py cnf.txt output6.sols
python ../../check.py cnf.txt output7.sols
