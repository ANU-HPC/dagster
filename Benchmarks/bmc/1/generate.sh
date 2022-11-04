: '
Script for turning the EMD problem with CBMC at different levels of fidelity.
generating the dags for AI variable propagation.
Also initiating and recording the timing of dagster running these problems

BMC for attack on protocl elaborated in 

A lightweight security protocol for ultra-low power ASIC implementation for wireless Implantable Medical Devices
 by Saied Hosseini-Khayat
  5th International Symposium on Medical Information and Communication Technology (2011)
: '


echo "generating 128 bit CNF"
cbmc emd8.c --dimacs > 8bitcnf_r.cnf
echo "generating 64 bit CNF"
cbmc emd16.c --dimacs > 16bitcnf_r.cnf
echo "generating 32 bit CNF"
cbmc emd32.c --dimacs > 32bitcnf_r.cnf
echo "generating 16 bit CNF"
cbmc emd64.c --dimacs > 64bitcnf_r.cnf

echo "splitting CNFs"
python trim.py 8bitcnf_r.cnf 8bitcnf.cnf
python trim.py 16bitcnf_r.cnf 16bitcnf.cnf
python trim.py 32bitcnf_r.cnf 32bitcnf.cnf
python trim.py 64bitcnf_r.cnf 64bitcnf.cnf

echo "generating AI CNF and DAGs"

python cmbc_joiner.py 8bitcnf.cnf 64bitcnf.cnf R8_64.cnf R8_64.dag
python cmbc_joiner.py 16bitcnf.cnf 64bitcnf.cnf R16_64.cnf R16_64.dag
python cmbc_joiner.py 32bitcnf.cnf 64bitcnf.cnf R32_64.cnf R32_64.dag

python ../../../utilities/dumb_dag_generator.py ./64bitcnf.cnf ./64bitcnf.dag


export OMPI_MCA_btl=self,tcp
export GLOG_v=0 # verbose glog logging at level 5 (0 is least verbose)
export GLOG_logtostderr=true # otherwise all logging goes to /tmp/appname.hostname.username.log.INFO.date.pid

echo "8_64"
time mpirun -n 3 ../../../dagster/dagster -b 0 -e 0 -i 1 ./R8_64.dag ./R8_64.cnf
echo "16_64"
time mpirun -n 3 ../../../dagster/dagster -b 0 -e 0 -i 1 ./R16_64.dag ./R16_64.cnf
echo "32_64"
time mpirun -n 3 ../../../dagster/dagster -b 0 -e 0 -i 1 ./R32_64.dag ./R32_64.cnf
echo "64"
time mpirun -n 3 ../../../dagster/dagster -b 0 -e 0 -i 1 ./64bitcnf.dag ./64bitcnf.cnf




