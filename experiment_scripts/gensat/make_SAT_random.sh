#!/bin/bash
STARTINGDIR=`pwd`
cd ../../Benchmarks/gensat
./build.sh
cd $STARTINGDIR


export PATH=/home/cgretton/local/bin/:$PATH
export CPATH=/home/cgretton/local/include/
export LD_LIBRARY_PATH=/home/cgretton/local/lib/
export LIBRARY_PATH=/home/cgretton/local/lib/
export PATH=$HOME/openssl/bin:$PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/openssl/lib


export OMPI_MCA_btl=self,tcp
export GLOG_v=2
export GLOG_logtostderr=true


rm v*


for g in 50; do 
    N=$((300 + $g))
    C=`echo "import math; print(math.floor($N * 4.25))" | python | awk -F. '{print $1}'`


    ../../Benchmarks/gensat/ggen ${N} ${C} 3 1 100
    for f in `ls v${N}c${C}*.dag` ; do
	mv `echo "$f" | sed s/"\.dag"/""/g` in.cnf
	head -2 in.cnf > tmp
	tail -$((`cat in.cnf | grep ^p | awk '{print $4}'` - 16))  in.cnf  | head -1 >> tmp
	tail -$((`cat in.cnf | grep ^p | awk '{print $4}'` - 16))  in.cnf  | head -1 >> tmp
	tail -$((`cat in.cnf | grep ^p | awk '{print $4}'` - 16))  in.cnf  | head -1 >> tmp
	tail -$((`cat in.cnf | grep ^p | awk '{print $4}'` - 16))  in.cnf  | head -1 >> tmp
	tail -$((`cat in.cnf | grep ^p | awk '{print $4}'` - 16))  in.cnf  | head -1 >> tmp
	tail -$((`cat in.cnf | grep ^p | awk '{print $4}'` - 16))  in.cnf  | head -1 >> tmp
	tail -$((`cat in.cnf | grep ^p | awk '{print $4}'` - 16))  in.cnf  | head -1 >> tmp
	tail -$((`cat in.cnf | grep ^p | awk '{print $4}'` - 16))  in.cnf  | head -1 >> tmp
	tail -$((`cat in.cnf | grep ^p | awk '{print $4}'` - 16))  in.cnf  | head -1 >> tmp
	tail -$((`cat in.cnf | grep ^p | awk '{print $4}'` - 16))  in.cnf  | head -1 >> tmp
	tail -$((`cat in.cnf | grep ^p | awk '{print $4}'` - 16))  in.cnf  | head -1 >> tmp
	tail -$((`cat in.cnf | grep ^p | awk '{print $4}'` - 16))  in.cnf  | head -1 >> tmp
	tail -$((`cat in.cnf | grep ^p | awk '{print $4}'` - 16))  in.cnf  | head -1 >> tmp
	tail -$((`cat in.cnf | grep ^p | awk '{print $4}'` - 16))  in.cnf  | head -1 >> tmp
	tail -$((`cat in.cnf | grep ^p | awk '{print $4}'` - 16))  in.cnf  | head -1 >> tmp
	tail -$((`cat in.cnf | grep ^p | awk '{print $4}'` - 16))  in.cnf  | head -1 >> tmp
	tail -$((`cat in.cnf | grep ^p | awk '{print $4}'` - 16))  in.cnf >> tmp
	if timeout 10s ../../Originals/gnovelty+-T/gnovelty+  tmp  2>&1  | grep -q " SATISFIABLE" ; then
            (>&2 echo "SAT instance being skipped")
            sleep 1
            continue
	fi

	mv "$f" in.dag
	cp in.cnf ../../dagster/
	cp in.dag ../../dagster/
	cd ../../dagster/
	if [ ! -e dagster ]  ; then
	    (>&2 echo "Cannot find dagster executable, building")
	    make clean ; make -j 16
	fi
	rm dag_out.txt
	time mpirun -n 92 timeout 3600s ./dagster -m 0 -k 1 -d 100 -s 100 -e 0 -g 0 -c minisat -r cdclfirst -a ghosts in.dag in.cnf 2>&1 | gzip > $f.run.output.gz
	if [ ! -e dag_out.txt ] ; then
	    (>&2 echo "ERROR, NO OUTPUT PRODUCED BY DAGSTER on input $f")
	    cd $STARTINGDIR
	    (>&2 echo "A ROUND WAS ABORTED")
	    sleep 1
	    continue
	fi
	MODELS=`wc -l dag_out.txt | awk '{print $1}'`
	echo "COUNT OF MODELS: $MODELS"
	if [ $MODELS -ge 1 ] ; then 
	    (>&2 echo "Dagster found SAT, running gnovelty+ to confirm")
	    #time ../Originals/gnovelty+-T/gnovelty+ in.cnf
	else
	    (>&2 echo "Dagster found UNSAT, running TiniSAT to confirm")
	    #time ../Originals/tinisat0.22/tinisat in.cnf
	fi

	cd $STARTINGDIR

	(>&2 echo "A ROUND IS COMPLETED")
	sleep 1
    done
done
