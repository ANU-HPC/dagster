
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
pip install statistics
python3 timer.py
python3 timer_processing.py
deactivate
rm dag_out.txt
rm dag.txt
rm cnf.txt
rm output.sols
