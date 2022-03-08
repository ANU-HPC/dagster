STARTINGDIR=`pwd`
cd ../../Benchmarks/ramsay
make clean
make
cd $STARTINGDIR
cd ../../dagster/gnovelty/simple_standalone
make clean
make
cd $STARTINGDIR
cd ../../dagster/standalone_tinisat/
rm tinisat
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
export TINISAT_LOCATION=../../dagster/standalone_tinisat/tinisat
if [ ! -f "$TINISAT_LOCATION" ]; then
    echo "Failed to build tinisat at ${TINISAT_LOCATION}"
    exit 1
fi

export LINGELING_LOCATION=~/lingeling/lingeling
if [ ! -f "$LINGELING_LOCATION" ]; then
    echo "Could not find lingeling at ${LINGELING_LOCATION}"
    exit 1
fi

export MINISAT_LOCATION=~/minisat/build/release/bin/minisat
if [ ! -f "$MINISAT_LOCATION" ]; then
    echo "Could not find minisat at ${MINISAT_LOCATION}"
    exit 1
fi

pip install click
pip install tqdm
python runner.py
deactivate
