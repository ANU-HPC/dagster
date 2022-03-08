
STARTINGDIR=`pwd`
export TINISAT_LOCATION=../../dagster/standalone_tinisat/tinisat
echo "making dagster"
cd ../../dagster
make clean
make
echo "making tinisat"
cd ./standalone_tinisat/
rm tinisat
make
cd ../../Benchmarks/sgen
make
cd $STARTINGDIR

DIR="env"
if [ -d "$DIR" ]; then
  ### Take action if $DIR exists ###
  echo "Environment ${DIR} already exists"
else
  ###  Control will jump here if $DIR does NOT exists ###
  echo "Environment ${DIR} not found. creating..."
  python3 -m venv env
fi
source env/bin/activate
export OMPI_MCA_btl=self,tcp
export GLOG_v=3
export GLOG_logtostderr=true
pip install click
pip install tqdm
pip install fire
pip install networkx
pip install python-sat
pip install numpy
python run2.py
python run.py
deactivate
rm bog*
rm dag*
rm cnf*
echo "DONE"
