
STARTINGDIR=`pwd`
echo "making dagster"
cd ../../dagster
make clean
make
echo "making tinisat"
cd $STARTINGDIR
cd ../../dagster/standalone_tinisat/
rm tinisat
make
echo "making gensat_sat"
cd ../../Benchmarks/gensat_sat
./build.sh
cd $STARTINGDIR

export OMPI_MCA_btl=self,tcp
export GLOG_v=1
export GLOG_logtostderr=true
rm v*
python run.py
rm v*
rm dag_out.txt
echo "DONE"
