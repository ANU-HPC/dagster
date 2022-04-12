#!/bin/bash
set -e
export OMPI_MCA_btl=self,tcp
export GLOG_v=0 # verbose glog logging at level 5 (0 is least verbose)
export GLOG_logtostderr=true # otherwise all logging goes to /tmp/appname.hostname.username.log.INFO.date.pid
echo "RUNNING TEST"
echo "        TEST part 1"
time mpirun -n 3 --oversubscribe ../../../dagster -m 0 -v 1 -g 1 ./dag.txt ./cnf.txt -o output1.sols
echo "        TEST part 2"
if [ -f checkpoint_5.check ]; then
	time mpirun -n 3 --oversubscribe ../../../dagster -m 0 -g 1 -u checkpoint_5.check ./dag.txt ./cnf.txt -o output2.sols
elif [ -f checkpoint_4.check ]; then
	time mpirun -n 3 --oversubscribe ../../../dagster -m 0 -g 1 -u checkpoint_4.check ./dag.txt ./cnf.txt -o output2.sols
elif [ -f checkpoint_3.check ]; then
	time mpirun -n 3 --oversubscribe ../../../dagster -m 0 -g 1 -u checkpoint_3.check ./dag.txt ./cnf.txt -o output2.sols
elif [ -f checkpoint_2.check ]; then
	time mpirun -n 3 --oversubscribe ../../../dagster -m 0 -g 1 -u checkpoint_2.check ./dag.txt ./cnf.txt -o output2.sols
elif [ -f checkpoint_1.check ]; then
	time mpirun -n 3 --oversubscribe ../../../dagster -m 0 -g 1 -u checkpoint_1.check ./dag.txt ./cnf.txt -o output2.sols
elif [ -f checkpoint_0.check ]; then
	time mpirun -n 3 --oversubscribe ../../../dagster -m 0 -g 1 -u checkpoint_0.check ./dag.txt ./cnf.txt -o output2.sols
else
   echo "NO CHECKPOINT OUTPUTTED ??."
fi
echo "        TEST part 3"
python ../../check.py cnf.txt output1.sols
python ../../check.py cnf.txt output2.sols
rm *.check
rm *.sols
