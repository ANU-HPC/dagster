: '
Script for turning the EMD problem with CBMC at different levels of fidelity.
generating the dags for AI variable propagation.
Also initiating and recording the timing of dagster running these problems
: '


echo "generating 128 bit CNF"
#cbmc emd2.c -D R128 --dimacs > R128.cnf
echo "generating 64 bit CNF"
cbmc emd2.c -D R64 --dimacs > R64.cnf
echo "generating 32 bit CNF"
cbmc emd2.c -D R32 --dimacs > R32.cnf
echo "generating 16 bit CNF"
cbmc emd2.c -D R16 --dimacs > R16.cnf

echo "splitting CNFs"
#python trim.py R128.cnf R128_r.cnf
python trim.py R64.cnf R64_r.cnf
python trim.py R32.cnf R32_r.cnf
python trim.py R16.cnf R16_r.cnf

echo "generating AI CNF and DAGs"
python cmbc_joiner.py R16_r.cnf R32_r.cnf R16_32.cnf R16_32.dag
python cmbc_joiner.py R16_r.cnf R64_r.cnf R16_64.cnf R16_64.dag
python cmbc_joiner.py R32_r.cnf R64_r.cnf R32_64.cnf R32_64.dag
#python cmbc_joiner.py R16_r.cnf R128_r.cnf R16_128.cnf R16_128.dag
#python cmbc_joiner.py R32_r.cnf R128_r.cnf R32_128.cnf R32_128.dag
#python cmbc_joiner.py R64_r.cnf R128_r.cnf R64_128.cnf R64_128.dag
python ../../../utilities/dumb_dag_generator.py ./R16_r.cnf ./R16_r.dag
python ../../../utilities/dumb_dag_generator.py ./R32_r.cnf ./R32_r.dag
python ../../../utilities/dumb_dag_generator.py ./R64_r.cnf ./R64_r.dag
#python ../../../utilities/dumb_dag_generator.py ./R128_r.cnf ./R128_r.dag

export OMPI_MCA_btl=self,tcp
export GLOG_v=0 # verbose glog logging at level 5 (0 is least verbose)
export GLOG_logtostderr=true # otherwise all logging goes to /tmp/appname.hostname.username.log.INFO.date.pid

echo "16"
time mpirun -n 3 ../../../dagster/dagster -b 0 -e 0 ./R16_r.dag ./R16_r.cnf
echo "16_32"
time mpirun -n 3 ../../../dagster/dagster -b 0 -e 0 ./R16_32.dag ./R16_32.cnf
echo "16_64"
time mpirun -n 3 ../../../dagster/dagster -b 0 -e 0 ./R16_64.dag ./R16_64.cnf
#echo "16_128"
#time mpirun -n 3 ../../../dagster/dagster -b 0 -e 0 ./R16_128.dag ./R16_128.cnf

echo "32"
time mpirun -n 3 ../../../dagster/dagster -b 0 -e 0 ./R32_r.dag ./R32_r.cnf
echo "32_62"
time mpirun -n 3 ../../../dagster/dagster -b 0 -e 0 ./R32_64.dag ./R32_64.cnf
#echo "32_128"
#time mpirun -n 3 ../../../dagster/dagster -b 0 -e 0 ./R32_128.dag ./R32_128.cnf

echo "64"
time mpirun -n 3 ../../../dagster/dagster -b 0 -e 0 ./R64_r.dag ./R64_r.cnf
#echo "64_128"
#time mpirun -n 3 ../../../dagster/dagster -b 0 -e 0 ./R64_128.dag ./R64_128.cnf

#echo "128"
#time mpirun -n 3 ../../../dagster/dagster -b 0 -e 0 ./R128_r.dag ./R128_r.cnf






