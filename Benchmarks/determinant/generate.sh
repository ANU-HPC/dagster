: '
Script
: '

echo "generating 6"
python det19.py 6 10 cnf6.txt map6.txt dag6.txt
echo "generating 7"
python det19.py 7 10 cnf7.txt map7.txt dag7.txt
echo "generating 8"
python det19.py 8 10 cnf8.txt map8.txt dag8.txt
echo "generating 9"
python det19.py 9 10 cnf9.txt map9.txt dag9.txt
echo "generating 10"
python det19.py 10 10 cnf10.txt map10.txt dag10.txt

export OMPI_MCA_btl=self,tcp
export GLOG_v=0 # verbose glog logging at level 5 (0 is least verbose)
export GLOG_logtostderr=true # otherwise all logging goes to /tmp/appname.hostname.username.log.INFO.date.pid

echo "6"
time timeout 300s mpirun -n 6 ../../dagster/dagster -b 0 -e 1 ./dag6.txt ./cnf6.txt
echo "6 dumb"
time timeout 300s mpirun -n 2 ../../dagster/dagster -b 0 -e 1 ./dag6.txtdumb ./cnf6.txt


echo "7"
time timeout 300s mpirun -n 6 ../../dagster/dagster -b 0 -e 1 ./dag7.txt ./cnf7.txt
echo "7 dumb"
time timeout 300s mpirun -n 2 ../../dagster/dagster -b 0 -e 1 ./dag7.txtdumb ./cnf7.txt


echo "8"
time timeout 300s mpirun -n 6 ../../dagster/dagster -b 0 -e 1 ./dag8.txt ./cnf8.txt
echo "8 dumb"
time timeout 300s mpirun -n 2 ../../dagster/dagster -b 0 -e 1 ./dag8.txtdumb ./cnf8.txt


echo "9"
time timeout 300s mpirun -n 6 ../../dagster/dagster -b 0 -e 1 ./dag9.txt ./cnf9.txt
echo "9 dumb"
time timeout 300s mpirun -n 2 ../../dagster/dagster -b 0 -e 1 ./dag9.txtdumb ./cnf9.txt


echo "10"
time timeout 300s mpirun -n 6 ../../dagster/dagster -b 0 -e 1 ./dag10.txt ./cnf10.txt
echo "10 dumb"
time timeout 300s mpirun -n 2 ../../dagster/dagster -b 0 -e 1 ./dag10.txtdumb ./cnf10.txt







