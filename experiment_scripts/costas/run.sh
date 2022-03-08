STARTINGDIR=`pwd`
export OMPI_MCA_btl=self,tcp
export GLOG_v=3
export GLOG_logtostderr=true
export DAGSTER_PROCESSORS=64
cd ../../Benchmarks/costas
make clean
make
cd ../../dagster
make clean
make
cd $STARTINGDIR
python3 runner.py


