STARTINGDIR=`pwd`
cd ../../Benchmarks/ramsay
make clean
make
cd $STARTINGDIR
cd ../../dagster
make clean
make
cd $STARTINGDIR
export OMPI_MCA_btl=self,tcp
export GLOG_v=0
export GLOG_logtostderr=true
export MPI_PROCESSORS_AVAILABLE=2
python runner.py
